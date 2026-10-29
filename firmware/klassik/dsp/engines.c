/* SPDX-License-Identifier: MIT
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments (the voice start and the render loop,
 *   from Felucca-Engines' tests/fe_host.c)
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's engine host: three of Felucca-Engines' MIT engines (ANALOG, PHASE, WHEEL) and KLASSIK's FM6 (eng_fm6.c,
 * on msfa), DRUMS (eng_drums.c, on Plaits' drum models) and ACID (eng_acid.c, on Open303) on four parts of six voices
 * each, every part its own sound; together the parts sound at most KL_BUDGET (each voice costs its engine's COST).
 * Built as one translation unit with the engines, as felucca_engine.h asks. A first version: its envelope is a plain
 * ADSR of its own (not Felucca's voice.c), an LFO a part (KLASSIK's own: lfo_tick), and no glide, unison or effect; FM6, DRUMS and ACID have
 * envelopes of their own (ownenv), DRUMS plays each instrument on one voice (same_note). A MONO preset (ACID's, some of ANALOG's)
 * plays on the part's first voice and keeps the keys held: a key while another is held is legato, and the latest let go while
 * another is held goes back to that one (legato 2). The eight parameters come from the app with each sound asked
 * for (an edit plays from the next block: the engines read them every block); the preset gives the rest. */
#include <stdint.h>
#ifdef PLAT_HOST
#include <string.h>
#else                                               /* the FM-1 has no libc: felucca-bsp's chip/libc.c has these */
void *memset(void *d, int c, unsigned n);
void *memcpy(void *d, const void *s, unsigned n);
int memcmp(const void *a, const void *b, unsigned n);
#endif
#include "klassik_dsp.h"

#define NVOICE 6
#define NPART KL_NPART
#include "fe_tables.h"
#include "felucca_engine.h"

enum { P_ATK, P_REL, P_FTYPE, P_E0, P_E1, P_E2, P_E3, P_E4, P_E5, P_E6, P_E7, P_PRESET, P_COUNT };   /* P_PRESET: FM6's */
typedef struct track {
    int16_t p[P_COUNT];
    voice_t v[NVOICE];
} track_t;
static track_t trk[NPART];
static int retrig;                                  /* the voice a note_on starts still sounds (FM6 goes on from it) */
static int legato;                                  /* MONO: 1 the note_on comes while a key is held (a slide), 2 it
                                                     * goes back to a key still held (the latest let go) */

#include "dsp.c"

/* ---------------------------------------------------------- each part's LFO --- */
/* a wave a block: the main loop sets it (kl_lfo), the app's render may set where it is (kl_lfo_phase); the host puts
 * it on ANALOG's, PHASE's and WHEEL's block (vmod_t: pitch, cutoff, shape, amplitude), FM6 and ACID take it
 * themselves (lfo_pitch, lfo_lin, lfo_gain) */
typedef struct {
    volatile uint32_t inc, fade;                    /* a block's step, Q32 of a turn; blocks the depth fades in */
    volatile uint8_t depth, wave, dest, retrig;     /* 0..100 (0 off), KL_SIN.., KL_LFO_*, 1 = each note starts it */
    uint8_t ph_set, hit;                            /* kl_lfo_phase came this block; a note started (S&H: anew) */
    uint32_t ph, ph_to, fade_n, rnd;
    int32_t out, amt;                               /* this block's wave (Q15), its depth (percent, Q8, faded in) */
} lfo_t;
static lfo_t lfo[NPART];

static void lfo_tick(int part)                      /* a block on */
{
    lfo_t *l = &lfo[part];
    uint32_t was = l->ph, u;
    l->ph = l->ph_set ? l->ph_to : l->ph + l->inc;
    l->ph_set = 0;
    switch (l->wave) {
    case KL_TRI:                                    /* from 0 up, as the sine */
        u = (l->ph + 0x40000000u) >> 16;
        l->out = u < 32768u ? (int32_t)u * 2 - 32768 : 98303 - (int32_t)u * 2;
        break;
    case KL_SAW:                                    /* up from the bottom: the turn starts low */
        l->out = (int32_t)(l->ph >> 16) - 32768;
        break;
    case KL_SQR:
        l->out = l->ph < 0x80000000u ? 32767 : -32767;
        break;
    case KL_SH:                                     /* a new random value each turn (and with RETRIG each note) */
        if (l->ph < was || l->hit) {
            l->rnd = l->rnd * 1103515245u + 12345u;
            l->out = (int32_t)(l->rnd >> 16) - 32768;
        }
        break;
    default:
        l->out = SINE[l->ph >> 22];
        break;
    }
    l->hit = 0;
    if (l->fade_n < l->fade)
        l->fade_n++;
    l->amt = l->fade ? (int32_t)l->depth * 256 * (int32_t)l->fade_n / (int32_t)l->fade : (int32_t)l->depth * 256;
}

static void lfo_note(int part)                      /* a note of the part starts: RETRIG, FADE */
{
    lfo_t *l = &lfo[part];
    if (l->retrig) {
        l->ph = 0u - l->inc;                        /* the next tick: 0 */
        l->hit = 1;
    }
    l->fade_n = 0;
}

static int32_t lfo_depth(int part, int dest) { return lfo[part].dest == dest ? lfo[part].amt : 0; }

static int32_t lfo_pitch(int part)                  /* PITCH: 1/256 semitone, +-12 semitones at DEPTH 100 */
{
    int32_t a = lfo_depth(part, KL_LFO_PITCH);
    return a ? lfo[part].out * (int32_t)((uint32_t)a * (uint32_t)a / 213333u) >> 15 : 0;   /* 3072 * (a / 25600)^2 */
}

static int32_t lfo_lin(int part, int dest, int32_t full)   /* FILTER, SHAPE: +-full at DEPTH 100 */
{
    int32_t a = lfo_depth(part, dest);
    return a ? lfo[part].out * (full * a / 25600) >> 15 : 0;
}

static int32_t lfo_gain(int part)                   /* AMP: Q15, 1 at the wave's top, 1 - DEPTH at its trough */
{
    int32_t a = lfo_depth(part, KL_LFO_AMP), u = (lfo[part].out + 32768) >> 1;
    return a ? 32768 - (32768 - u) * a / 25600 : 32768;
}

#include "eng_analog.c"
#include "eng_phase.c"
#include "eng_wheel.c"
#include "eng_fm6.c"
#include "eng_drums.c"
#include "eng_acid.c"

static const engine_t *const ENG[KL_NENG] = {&ENG_ANALOG, &ENG_PHASE, &ENG_WHEEL, &ENG_FM6, &ENG_DRUMS, &ENG_ACID};

/* ------------------------------------------------------------ the app's questions --- */
static int eng_ok(int e) { return e >= 0 && e < KL_NENG; }
const char *kl_engine_name(int e) { return eng_ok(e) ? ENG[e]->name : ""; }
int kl_presets(int e) { return eng_ok(e) ? ENG[e]->npresets : 0; }
const char *kl_preset_name(int e, int p)
{
    return eng_ok(e) && p >= 0 && p < ENG[e]->npresets ? ENG[e]->presets[p].name : "";
}

void kl_preset_values(int e, int p, int8_t v[8])
{
    uint32_t k;
    for (k = 0; k < 8u; k++)
        v[k] = eng_ok(e) && p >= 0 && p < ENG[e]->npresets ? ENG[e]->presets[p].e[k] : 0;
}

const char *kl_page_title(int e, int page) { return eng_ok(e) ? ENG[e]->page_title[page & 1] : ""; }

const char *kl_drum_name(int kit, int pc) { return drum_name(kit, DR_OF_PC[pc % 12]); }
int kl_drum_pages(int pc)
{
    int i = DR_OF_PC[pc % 12];
    return i == DR_BD || i == DR_SD ? 1 : i == DR_LT || i == DR_MT || i == DR_HT || i == DR_CH || i == DR_OH ? 2 : 0;
}

static track_t ask_t;                               /* a sound's values, for an engine's desc hook (it reads p[]) */

void kl_param(int e, int k, const int8_t v[8], kl_param_t *d)
{
    const param_desc_t *pd;
    uint32_t i;
    if (!eng_ok(e) || k < 0 || k > 7) {
        memset(d, 0, sizeof *d);
        d->label = "";
        return;
    }
    pd = &ENG[e]->edit[k];
    if (ENG[e]->desc) {                             /* another name in some modes: the same range */
        const param_desc_t *alt;
        for (i = 0; i < 8u; i++)
            ask_t.p[P_E0 + i] = v[i];
        if ((alt = ENG[e]->desc(&ask_t, (uint32_t)k)) != 0)
            pd = alt;
    }
    d->label = pd->label;
    d->unit = pd->unit;
    d->names = pd->names;
    d->min = pd->min;
    d->max = pd->max;
    d->fmt = pd->fmt == F_ENUM && pd->names ? KL_ENUM : pd->fmt == F_PCT || pd->fmt == F_CUTOFF ? KL_PCT : KL_INT;
}

void kl_shape(int e, const int8_t v[8], uint8_t out[8])
{
    uint32_t k;
    for (k = 0; k < 8u; k++) {
        int32_t lo, hi;
        out[k] = 128;
        if (!eng_ok(e))
            continue;
        lo = ENG[e]->edit[k].min;
        hi = ENG[e]->edit[k].max;
        if (hi > lo)
            out[k] = (uint8_t)clamp((v[k] - lo) * 255 / (hi - lo), 0, 255);
    }
}

/* ------------------------------------------------- the main loop asks, the render plays --- */
typedef struct {                                    /* a part: its sound, as asked for and as it plays */
    volatile int sel_e, sel_p;                      /* asked for */
    volatile int8_t sel_v[8];
    volatile uint32_t sel_n;                        /* asks so far: the render takes the values when it moves */
    uint8_t loaded;                                 /* cur_e and cur_p play */
    int cur_e, cur_p;                               /* playing */
    uint32_t cur_n;                                 /* the ask whose values play */
    uint8_t env_set[4];                             /* the preset's ATK DEC SUS REL, 0..127 */
    int32_t env_fenv;                               /* its ENV -> FILTER, -64..63 */
    uint8_t mono_mode;
    uint8_t held[8], nheld;                         /* MONO: the keys held, the latest last */
    int32_t last_amp[NVOICE];
    volatile uint32_t hits;                         /* notes started, and the last one's (the app's DRUMS pads) */
    volatile uint8_t hit_note;
} part_t;
static part_t prt[NPART];
static volatile int hush;
#define QN 64u
static volatile uint32_t q_w, q_r;                  /* main loop writes q_w, the render q_r */
static uint8_t q_part[QN], q_on[QN], q_note[QN], q_vel[QN];
static uint32_t vage;                               /* voice starts so far, all parts: the oldest is the lowest */

/* what a voice of each engine costs, in tenths of an FM6 voice (measured on the host: RESEARCH.md); the parts
 * together may sound KL_BUDGET of them: past it the oldest voice anywhere goes */
static const uint8_t COST[KL_NENG] = {8, 5, 11, 10, 14, 74};

static int part_ok(int part) { return part >= 0 && part < NPART; }

void kl_select(int part, int e, int p, const int8_t v[8])
{
    part_t *q;
    uint32_t k;
    if (!part_ok(part))
        return;
    q = &prt[part];
    for (k = 0; k < 8u; k++)
        q->sel_v[k] = v[k];
    q->sel_e = eng_ok(e) ? e : KL_ANALOG;
    q->sel_p = p;
    q->sel_n = q->sel_n + 1u;                       /* last: the render may come between, and takes them next */
}

void kl_key(int part, int on, int note, int vel)
{
    uint32_t w = q_w;
    if (!part_ok(part) || w - q_r >= QN)            /* full: dropped (64 waiting is more than the keys can do) */
        return;
    q_part[w % QN] = (uint8_t)part;
    q_on[w % QN] = (uint8_t)(on != 0);
    q_note[w % QN] = (uint8_t)clamp(note, 0, 127);
    q_vel[w % QN] = (uint8_t)clamp(vel, 1, 127);
    q_w = w + 1u;
}

void kl_silence(void) { hush = 1; }

/* -------------------------------------------------------------- the voices --- */
static void held_drop(part_t *q, int note)
{
    uint32_t i, j;
    for (i = j = 0; i < q->nheld; i++)
        if (q->held[i] != note)
            q->held[j++] = q->held[i];
    q->nheld = (uint8_t)j;
}

static void held_push(part_t *q, int note)
{
    uint32_t i;
    held_drop(q, note);
    if (q->nheld == sizeof q->held) {               /* full: the oldest goes */
        for (i = 1; i < sizeof q->held; i++)
            q->held[i - 1u] = q->held[i];
        q->nheld--;
    }
    q->held[q->nheld++] = (uint8_t)note;
}

static void load(int part, int e, int p)
{
    part_t *q = &prt[part];
    const engine_t *en = ENG[e];
    const preset_t *pr;
    track_t *t = &trk[part];
    if (p < 0 || p >= en->npresets)                /* the engine and the preset asked for at different times */
        p = 0;
    pr = &en->presets[p];
    if (!q->loaded || e != q->cur_e) {              /* another engine: from silence, WHEEL's own state cleared */
        memset(t, 0, sizeof *t);
        memset(&drw_t[part], 0, sizeof drw_t[part]);
        memset(drw_v[part], 0, sizeof drw_v[part]);
        memset(q->last_amp, 0, sizeof q->last_amp);
        q->nheld = 0;
    }
    t->p[P_ATK] = pr->env[0];
    t->p[P_REL] = pr->env[3];
    t->p[P_FTYPE] = 0;
    t->p[P_PRESET] = (int16_t)p;
    memcpy(q->env_set, pr->env, sizeof q->env_set);
    q->env_fenv = pr->fenv;
    q->mono_mode = pr->mono;
    q->cur_e = e;
    q->cur_p = p;
    q->loaded = 1;
}

/* the sound asked for: the preset at a change, then the values, each kept to its range (for a block they may be
 * another engine's, asked for at a different time: none must index past an engine's table) */
static void ensure_loaded(int part)
{
    part_t *q = &prt[part];
    uint32_t n = q->sel_n, k;
    int e = q->sel_e, p = q->sel_p;
    if (q->loaded && e == q->cur_e && p == q->cur_p && n == q->cur_n)
        return;
    if (!q->loaded || e != q->cur_e || p != q->cur_p)
        load(part, e, p);
    for (k = 0; k < 8u; k++)
        trk[part].p[P_E0 + k] = (int16_t)clamp(q->sel_v[k], ENG[q->cur_e]->edit[k].min, ENG[q->cur_e]->edit[k].max);
    q->cur_n = n;
}

static int same_note(int part, int a, int b)        /* DRUMS: the same instrument (eng_drums.c), one voice each */
{
    return prt[part].cur_e == KL_DRUMS ? DR_OF_PC[a % 12] == DR_OF_PC[b % 12] : a == b;
}

/* the engine's note_on; a voice that still sounds keeps its phases and keep slots (from fe_host.c) */
static void voice_note_on(int part, voice_t *v, int sounding)
{
    const engine_t *en = ENG[prt[part].cur_e];
    uint32_t ph0 = v->ph[0], ph1 = v->ph[1], ph2 = v->ph[2], k;
    int32_t s[8];
    memcpy(s, v->s, sizeof s);
    retrig = sounding;
    en->note_on(&trk[part], v);
    if (sounding) {
        v->ph[0] = ph0;
        v->ph[1] = ph1;
        v->ph[2] = ph2;
        for (k = 0; k < 8u; k++)
            if (k != 2u && k != 3u && (en->keep >> k & 1u))
                v->s[k] = s[k];
    }
}

/* room for a voice of engine e (v, if it sounds, is to be reused): the oldest voices anywhere go, those already
 * let go first, until the parts' cost with it is within the budget */
static void voice_room(int e, const voice_t *v)
{
    for (;;) {
        uint32_t used = COST[e], part, k;
        voice_t *go = 0;
        for (part = 0; part < NPART; part++)
            for (k = 0; k < NVOICE; k++) {
                voice_t *o = &trk[part].v[k];
                if (!o->active || o == v)
                    continue;
                used += COST[prt[part].cur_e];
                if (!go || o->gate < go->gate || (o->gate == go->gate && o->age < go->age))
                    go = o;
            }
        if (used <= KL_BUDGET || !go)
            return;
        go->active = 0;
        go->stage = 0;
        go->env = 0;
    }
}

static void voice_start(int part, int note, int vel)
{
    part_t *q = &prt[part];
    track_t *t = &trk[part];
    voice_t *v = 0;
    uint32_t k, oldest = 0xFFFFFFFFu;
    int sounding;
    legato = 0;
    if (q->mono_mode) {
        v = &t->v[0];
        legato = q->nheld > 0 && v->active && v->gate;
        held_push(q, note);
    }
    for (k = 0; !v && k < NVOICE; k++)              /* the same note again: that voice */
        if (t->v[k].active && same_note(part, t->v[k].note, note))
            v = &t->v[k];
    for (k = 0; !v && k < NVOICE; k++)              /* a free one */
        if (!t->v[k].active)
            v = &t->v[k];
    if (!v)                                         /* else the part's oldest */
        for (k = 0; k < NVOICE; k++)
            if (t->v[k].age < oldest) {
                oldest = t->v[k].age;
                v = &t->v[k];
            }
    voice_room(q->cur_e, v);
    sounding = v->active && v->stage != 0;
    v->note = (uint8_t)note;
    v->vel = (uint8_t)vel;
    v->gate = 1;
    v->active = 1;
    v->stage = 1;
    v->age = ++vage;
    q->hit_note = (uint8_t)note;
    q->hits = q->hits + 1u;
    lfo_note(part);
    v->pitch16 = v->pitch_cur = (int32_t)note * 16;
    v->fine = 0;
    if (!sounding)
        v->env = v->env_out = 0;
    voice_note_on(part, v, sounding);
}

static void voice_stop(int part, int note)
{
    part_t *q = &prt[part];
    track_t *t = &trk[part];
    uint32_t k;
    if (q->mono_mode) {
        voice_t *v = &t->v[0];
        held_drop(q, note);
        if (!v->active || !v->gate || v->note != note)
            return;
        if (q->nheld) {                             /* back to the latest key still held */
            v->note = q->held[q->nheld - 1u];
            v->pitch16 = v->pitch_cur = (int32_t)v->note * 16;
            legato = 2;
            voice_note_on(part, v, 1);
            return;
        }
        v->gate = 0;
        v->stage = 3;
        return;
    }
    for (k = 0; k < NVOICE; k++)
        if (t->v[k].active && t->v[k].gate && t->v[k].note == note) {
            t->v[k].gate = 0;
            t->v[k].stage = 3;
        }
}

void kl_key_now(int part, int on, int note, int vel)
{
    if (!part_ok(part))
        return;
    ensure_loaded(part);
    if (on)
        voice_start(part, clamp(note, 0, 127), clamp(vel, 1, 127));
    else
        voice_stop(part, note);
}

/* a plain ADSR, once per engine block (32 samples, 0.73 ms): attack up to ~1.5 s, decay toward the sustain
 * level, release by a fraction a block (ATK, DEC, SUS, REL of the preset, 0..127). Returns Q15. */
static int32_t env_tick(const uint8_t env_set[4], voice_t *v)
{
    int32_t sus = (int32_t)env_set[2] << 17;
    switch (v->stage) {
    case 1: {
        int32_t a = env_set[0];
        v->env += (1 << 24) / (1 + (a * a >> 3));
        if (v->env >= 1 << 24) {
            v->env = 1 << 24;
            v->stage = 2;
        }
        break;
    }
    case 2:
        v->env -= (v->env - sus) >> (1 + env_set[1] / 16);
        break;
    case 3:
        v->env -= v->env >> (2 + env_set[3] / 16);
        if (v->env < (1 << 24) / 2000) {
            v->env = 0;
            v->stage = 0;
            v->active = 0;
        }
        break;
    default:
        break;
    }
    return v->env >> 9;
}

static void part_block(int part, int32_t *out)      /* one block of a part's voices, added to out */
{
    part_t *q = &prt[part];
    track_t *t = &trk[part];
    const engine_t *en = ENG[q->cur_e];
    uint32_t k;
    int32_t po = lfo_pitch(part), cut = lfo_lin(part, KL_LFO_FILTER, 64 << 8), shp = lfo_lin(part, KL_LFO_SHAPE, 64 << 8);
    int32_t g = lfo_gain(part);
    if (en->block)
        en->block(t);
    for (k = 0; k < NVOICE; k++) {
        voice_t *v = &t->v[k];
        vmod_t m;
        int32_t env, a;
        uint32_t base;
        if (!v->active)
            continue;
        if (en->ownenv) {                           /* its own envelopes and velocity: full, until done */
            if (en->done(t, v)) {
                v->active = 0;
                v->stage = 0;
                continue;
            }
            a = env = 1 << 15;
        } else {
            a = env = env_tick(q->env_set, v);
            if (en->amp)
                a = en->amp(t, v, a);
            if (!v->active)
                continue;
            a = a * (40 + v->vel) / 167;           /* velocity: 0.24 .. 1 */
            a = mulq15(a, g);                       /* the LFO's AMP */
        }
        memset(&m, 0, sizeof m);
        m.pitch16 = v->pitch16 + (po >> 4);         /* the LFO's PITCH: 1/16 semitone in pitch16, the rest in fine */
        m.fine = (po & 15) * 237 >> 8;              /* 1/256 semitone: 2^(1/3072) - 1 = 225.6e-6, 0.924 of 1/4096 */
        base = pitch_inc((uint32_t)clamp(m.pitch16, 0, 2047));
        m.inc = base + (uint32_t)((int32_t)(base >> 12) * m.fine);
        m.amp0 = q->last_amp[k];
        m.amp1 = a;
        m.envq15 = env;
        m.cutoff = q->env_fenv * (env >> 7) + cut;  /* ENV -> FILTER: up to +-64 cutoff steps; the LFO's FILTER */
        m.shape = (64 << 8) + shp;                  /* the middle, and the LFO's SHAPE */
        q->last_amp[k] = a;
        en->render(t, v, out, CTL, &m);
    }
}

void kl_render(int32_t *const out[KL_NPART], uint32_t n)
{
    uint32_t b, i, k;
    int part;
    for (part = 0; part < NPART; part++)
        ensure_loaded(part);
    if (hush) {
        hush = 0;
        for (part = 0; part < NPART; part++) {
            for (k = 0; k < NVOICE; k++)
                trk[part].v[k].active = 0, trk[part].v[k].stage = 0, trk[part].v[k].env = 0;
            prt[part].nheld = 0;
        }
        q_r = q_w;
    }
    while (q_r != q_w) {
        uint32_t j = q_r % QN;
        kl_key_now(q_part[j], q_on[j], q_note[j], q_vel[j]);
        q_r = q_r + 1u;
    }
    for (b = 0; b + CTL <= n; b += CTL)
        for (part = 0; part < NPART; part++) {
            int32_t blk[CTL];
            lfo_tick(part);
            memset(blk, 0, sizeof blk);
            part_block(part, blk);
            for (i = 0; i < CTL; i++)
                out[part][b + i] += blk[i];
        }
}

uint32_t kl_hits(int part, int *note)
{
    if (!part_ok(part)) {
        *note = 0;
        return 0;
    }
    *note = prt[part].hit_note;
    return prt[part].hits;
}

int kl_voices(int part)
{
    uint32_t k;
    int n = 0, p;
    for (p = 0; p < NPART; p++)
        for (k = 0; k < NVOICE; k++)
            n += (part < 0 || part == p) && trk[p].v[k].active ? 1 : 0;
    return n;
}

int kl_cost_now(void)
{
    uint32_t k;
    int n = 0, p;
    for (p = 0; p < NPART; p++)
        for (k = 0; k < NVOICE; k++)
            n += trk[p].v[k].active ? COST[prt[p].cur_e] : 0;
    return n;
}

int kl_value_now(int part, int k) { return part_ok(part) && k >= 0 && k < 8 ? trk[part].p[P_E0 + k] : 0; }
int kl_note_now(int part) { return part_ok(part) && trk[part].v[0].active ? trk[part].v[0].note : -1; }

uint32_t kl_lfo_dests(int e)
{
    static const uint8_t DESTS[KL_NENG] = {15, 1 | 2 | 8, 1 | 8, 1 | 8, 0, 1 | 2};   /* ANALOG .. ACID: KL_LFO_* bits */
    return eng_ok(e) ? DESTS[e] : 0u;
}

void kl_lfo(int part, uint32_t inc, int depth, int wave, int dest, int retrig, uint32_t fade)
{
    lfo_t *l;
    if (!part_ok(part))
        return;
    l = &lfo[part];
    l->inc = inc;
    l->depth = (uint8_t)clamp(depth, 0, 100);
    l->wave = (uint8_t)clamp(wave, 0, KL_NWAVE - 1);
    l->dest = (uint8_t)clamp(dest, 0, KL_LFO_NDEST - 1);
    l->retrig = (uint8_t)(retrig != 0);
    l->fade = fade;
}

void kl_lfo_phase(int part, uint32_t ph)
{
    if (!part_ok(part))
        return;
    lfo[part].ph_to = ph;
    lfo[part].ph_set = 1;
}

uint32_t kl_lfo_phase_now(int part) { return part_ok(part) ? lfo[part].ph : 0u; }
int32_t kl_lfo_now(int part) { return part_ok(part) ? lfo[part].out : 0; }
int kl_lfo_depth_now(int part) { return part_ok(part) ? (int)(lfo[part].amt >> 8) : 0; }
