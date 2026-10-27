/* SPDX-License-Identifier: MIT
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments (the voice start and the render loop,
 *   from Felucca-Engines' tests/fe_host.c)
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's engine host: three of Felucca-Engines' MIT engines (ANALOG, PHASE, WHEEL) and KLASSIK's FM6 (eng_fm6.c,
 * on msfa), DRUMS (eng_drums.c, on Plaits' drum models) and ACID (eng_acid.c, on Open303) on one part of six voices.
 * Built as one translation unit with the engines, as felucca_engine.h asks. A first version: its envelope is a plain
 * ADSR of its own (not Felucca's voice.c), and there is no LFO, glide, unison or effect yet; FM6, DRUMS and ACID have
 * envelopes of their own (ownenv), DRUMS plays each instrument on one voice (same_note). A MONO preset (ACID's) plays
 * on the first voice and keeps the keys held: a key while another is held is legato, and the latest let go while
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
#define NPART 1
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
static volatile int sel_e = KL_ANALOG, sel_p = 0;   /* asked for */
static volatile int8_t sel_v[8];
static volatile uint32_t sel_n;                     /* asks so far: the render takes the values when it moves */
static volatile int hush;
#define QN 64u
static volatile uint32_t q_w, q_r;                  /* main loop writes q_w, the render q_r */
static uint8_t q_on[QN], q_note[QN], q_vel[QN];

void kl_select(int e, int p, const int8_t v[8])
{
    uint32_t k;
    for (k = 0; k < 8u; k++)
        sel_v[k] = v[k];
    sel_e = eng_ok(e) ? e : KL_ANALOG;
    sel_p = p;
    sel_n = sel_n + 1u;                             /* last: the render may come between, and takes them next */
}

void kl_key(int on, int note, int vel)
{
    uint32_t w = q_w;
    if (w - q_r >= QN)                              /* full: dropped (64 waiting is more than the keys can do) */
        return;
    q_on[w % QN] = (uint8_t)(on != 0);
    q_note[w % QN] = (uint8_t)clamp(note, 0, 127);
    q_vel[w % QN] = (uint8_t)clamp(vel, 1, 127);
    q_w = w + 1u;
}

void kl_silence(void) { hush = 1; }

/* -------------------------------------------------------------- the voices --- */
static int cur_e = -1, cur_p = -1;                  /* playing */
static uint32_t cur_n;                              /* the ask whose values play */
static uint8_t env_set[4];                          /* the preset's ATK DEC SUS REL, 0..127 */
static int32_t env_fenv;                            /* its ENV -> FILTER, -64..63 */
static uint8_t mono_mode;
static uint32_t vage;
static volatile uint32_t hits;                      /* notes started, and the last one's (the app's DRUMS pads) */
static volatile uint8_t hit_note;
static int32_t last_amp[NVOICE];
#define NHELD 8
static uint8_t held[NHELD], nheld;                  /* MONO: the keys held, the latest last */

static void held_drop(int note)
{
    uint32_t i, j;
    for (i = j = 0; i < nheld; i++)
        if (held[i] != note)
            held[j++] = held[i];
    nheld = (uint8_t)j;
}

static void held_push(int note)
{
    uint32_t i;
    held_drop(note);
    if (nheld == NHELD) {                           /* full: the oldest goes */
        for (i = 1; i < NHELD; i++)
            held[i - 1u] = held[i];
        nheld--;
    }
    held[nheld++] = (uint8_t)note;
}

static void load(int e, int p)
{
    const engine_t *en = ENG[e];
    const preset_t *pr;
    track_t *t = &trk[0];
    if (p < 0 || p >= en->npresets)                /* the engine and the preset asked for at different times */
        p = 0;
    pr = &en->presets[p];
    if (e != cur_e) {                               /* another engine: from silence, WHEEL's own state cleared */
        memset(t, 0, sizeof *t);
        memset(drw_t, 0, sizeof drw_t);
        memset(drw_v, 0, sizeof drw_v);
        memset(last_amp, 0, sizeof last_amp);
        nheld = 0;
    }
    t->p[P_ATK] = pr->env[0];
    t->p[P_REL] = pr->env[3];
    t->p[P_FTYPE] = 0;
    t->p[P_PRESET] = (int16_t)p;
    memcpy(env_set, pr->env, sizeof env_set);
    env_fenv = pr->fenv;
    mono_mode = pr->mono;
    cur_e = e;
    cur_p = p;
}

/* the sound asked for: the preset at a change, then the values, each kept to its range (for a block they may be
 * another engine's, asked for at a different time: none must index past an engine's table) */
static void ensure_loaded(void)
{
    uint32_t n = sel_n, k;
    int e = sel_e, p = sel_p;
    if (e == cur_e && p == cur_p && n == cur_n)
        return;
    if (e != cur_e || p != cur_p)
        load(e, p);
    for (k = 0; k < 8u; k++)
        trk[0].p[P_E0 + k] = (int16_t)clamp(sel_v[k], ENG[cur_e]->edit[k].min, ENG[cur_e]->edit[k].max);
    cur_n = n;
}

static int same_note(int a, int b)                  /* DRUMS: the same instrument (eng_drums.c), one voice each */
{
    return cur_e == KL_DRUMS ? DR_OF_PC[a % 12] == DR_OF_PC[b % 12] : a == b;
}

static void voice_start(int note, int vel)
{
    const engine_t *en = ENG[cur_e];
    track_t *t = &trk[0];
    voice_t *v = 0;
    uint32_t k, oldest = 0xFFFFFFFFu;
    legato = 0;
    if (mono_mode) {
        v = &t->v[0];
        legato = nheld > 0 && v->active && v->gate;
        held_push(note);
    }
    for (k = 0; !v && k < NVOICE; k++)              /* the same note again: that voice */
        if (t->v[k].active && same_note(t->v[k].note, note))
            v = &t->v[k];
    for (k = 0; !v && k < NVOICE; k++)              /* a free one */
        if (!t->v[k].active)
            v = &t->v[k];
    if (!v)                                         /* else the oldest */
        for (k = 0; k < NVOICE; k++)
            if (t->v[k].age < oldest) {
                oldest = t->v[k].age;
                v = &t->v[k];
            }
    {   /* from fe_host.c: a retrigger keeps the phases and the keep slots */
        int sounding = v->active && v->stage != 0;
        uint32_t ph0 = v->ph[0], ph1 = v->ph[1], ph2 = v->ph[2];
        int32_t s[8];
        memcpy(s, v->s, sizeof s);
        v->note = (uint8_t)note;
        v->vel = (uint8_t)vel;
        v->gate = 1;
        v->active = 1;
        v->stage = 1;
        v->age = ++vage;
        hit_note = (uint8_t)note;
        hits = hits + 1u;
        v->pitch16 = v->pitch_cur = (int32_t)note * 16;
        v->fine = 0;
        if (!sounding)
            v->env = v->env_out = 0;
        retrig = sounding;
        en->note_on(t, v);
        if (sounding) {
            v->ph[0] = ph0;
            v->ph[1] = ph1;
            v->ph[2] = ph2;
            for (k = 0; k < 8u; k++)
                if (k != 2u && k != 3u && (en->keep >> k & 1u))
                    v->s[k] = s[k];
        }
    }
}

static void voice_stop(int note)
{
    track_t *t = &trk[0];
    uint32_t k;
    if (mono_mode) {
        voice_t *v = &t->v[0];
        held_drop(note);
        if (!v->active || !v->gate || v->note != note)
            return;
        if (nheld) {                                /* back to the latest key still held */
            v->note = held[nheld - 1u];
            v->pitch16 = v->pitch_cur = (int32_t)v->note * 16;
            legato = 2;
            retrig = 1;
            ENG[cur_e]->note_on(t, v);
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

void kl_key_now(int on, int note, int vel)
{
    ensure_loaded();
    if (on)
        voice_start(clamp(note, 0, 127), clamp(vel, 1, 127));
    else
        voice_stop(note);
}

/* a plain ADSR, once per engine block (32 samples, 0.73 ms): attack up to ~1.5 s, decay toward the sustain
 * level, release by a fraction a block (ATK, DEC, SUS, REL of the preset, 0..127). Returns Q15. */
static int32_t env_tick(voice_t *v)
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

void kl_render(int32_t *mono, uint32_t n)
{
    track_t *t = &trk[0];
    int32_t out[CTL];
    uint32_t b, i, k;
    ensure_loaded();
    if (hush) {
        hush = 0;
        for (k = 0; k < NVOICE; k++)
            t->v[k].active = 0, t->v[k].stage = 0, t->v[k].env = 0;
        nheld = 0;
        q_r = q_w;
    }
    while (q_r != q_w) {
        uint32_t j = q_r % QN;
        kl_key_now(q_on[j], q_note[j], q_vel[j]);
        q_r = q_r + 1u;
    }
    for (b = 0; b + CTL <= n; b += CTL) {
        const engine_t *en = ENG[cur_e];
        if (en->block)
            en->block(t);
        memset(out, 0, sizeof out);
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
                a = env = env_tick(v);
                if (en->amp)
                    a = en->amp(t, v, a);
                if (!v->active)
                    continue;
                a = a * (40 + v->vel) / 167;           /* velocity: 0.24 .. 1 */
            }
            base = pitch_inc((uint32_t)clamp(v->pitch16, 0, 2047));
            memset(&m, 0, sizeof m);
            m.pitch16 = v->pitch16;
            m.inc = base;
            m.amp0 = last_amp[k];
            m.amp1 = a;
            m.envq15 = env;
            m.cutoff = env_fenv * (env >> 7);           /* ENV -> FILTER: up to +-64 cutoff steps */
            m.shape = 64 << 8;                          /* the middle: no shape modulation yet */
            last_amp[k] = a;
            en->render(t, v, out, CTL, &m);
        }
        for (i = 0; i < CTL; i++)
            mono[b + i] += out[i];
    }
}

uint32_t kl_hits(int *note)
{
    *note = hit_note;
    return hits;
}

int kl_voices(void)
{
    uint32_t k;
    int n = 0;
    for (k = 0; k < NVOICE; k++)
        n += trk[0].v[k].active ? 1 : 0;
    return n;
}

int kl_value_now(int k) { return k >= 0 && k < 8 ? trk[0].p[P_E0 + k] : 0; }
int kl_note_now(void) { return trk[0].v[0].active ? trk[0].v[0].note : -1; }
