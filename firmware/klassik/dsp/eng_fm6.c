/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* FM6: six operators as a DX7 has them, msfa's voice (dsp/fm6/msfa.c, Apache-2.0) as one more of the engines
 * (felucca_engine.h; included by engines.c after Felucca-Engines' own). Its envelopes are the voice's amplitude
 * (ownenv): the host's ADSR and velocity stay out, and the voice ends when its carriers are silent (done).
 *
 * A preset is a whole DX7 voice (fm6_voice_t) and the eight values that describe it: ALGO, FEEDBK, MOD, DTUN on
 * EDIT 1, ATK, DEC, SUS, REL on EDIT 2. ALGO and FEEDBK are the voice's own; DTUN spreads the operators' detune
 * (0: none); MOD is the loudest modulator's output level; ATK, DEC and REL are the carriers' first, second and last
 * rate, the other way round (0: the fastest), each 0..99 for 80 rate steps; SUS the carriers' sustain level. Turned,
 * a value moves every operator of the voice by its own difference from the preset's (MOD the modulators only), so
 * the voice keeps its shape; the voice as it plays is made again at a change (fm6_sync). ALGO and FEEDBK play at once,
 * the others from the next note. The presets are KLASSIK's own sounds, not the DX7's. */
#include "fm6/msfa.h"

#if FM_N != CTL
#error "eng_fm6.c: msfa's block (FM_N) is the engines' (CTL)"
#endif

enum { FM_ALGO, FM_FEEDBK, FM_MOD, FM_DTUN, FM_ATK, FM_DEC, FM_SUS, FM_REL };
#define FM6_SHIFT 11                                /* msfa's voice (a carrier at full level 2^25) to the engines' */

typedef struct {                                    /* an operator, as a DX7's screen shows it */
    uint8_t r[4], l[4];                             /* its envelope's rates and levels, 0..99 */
    uint8_t ol;                                     /* output level, 0..99 */
    uint8_t coarse, fine;                           /* the ratio: coarse 0 (0.5) .. 31, times 1 + fine / 100 */
    uint8_t kvs, rs;                                /* velocity sensitivity, rate scaling: 0..7 */
} fm6_op_t;

typedef struct {
    fm6_op_t op[6];                                 /* OP1 .. OP6 */
    uint8_t alg, fb;                                /* 1..32, 0..7 */
    uint8_t pr[4], pl[4];                           /* the pitch envelope (level 50: the note's pitch) */
    uint8_t lfo[6];                                 /* speed, delay, pitch depth, amp depth, key sync, wave (4 sine) */
    uint8_t pms;                                    /* pitch modulation sensitivity, 0..7 */
} fm6_voice_t;

#define OP(r1, r2, r3, r4, l1, l2, l3, l4, ol, co, fi, kvs, rs) \
    {{r1, r2, r3, r4}, {l1, l2, l3, l4}, ol, co, fi, kvs, rs}
#define OP_OFF OP(99, 99, 99, 99, 0, 0, 0, 0, 0, 1, 0, 0, 0)
#define PEG_FLAT .pr = {99, 99, 99, 99}, .pl = {50, 50, 50, 50}
#define LFO_OFF .lfo = {35, 0, 0, 0, 1, 0}, .pms = 3

static const fm6_voice_t FM6_VOICE[] = {
    {   /* E.PIANO: three pairs, a bell-like tine (14:1) over two 1:1 bodies; harder is brighter */
        {OP(96, 29, 20, 64, 99, 87, 0, 0, 99, 1, 0, 2, 3), OP(98, 56, 30, 70, 99, 62, 0, 0, 64, 14, 0, 7, 3),
         OP(95, 22, 18, 60, 99, 90, 0, 0, 99, 1, 0, 2, 3), OP(95, 30, 25, 60, 99, 82, 0, 0, 76, 1, 0, 6, 3),
         OP(95, 22, 18, 60, 99, 90, 0, 0, 82, 1, 0, 2, 3), OP(95, 35, 25, 60, 99, 75, 0, 0, 68, 1, 0, 5, 3)},
        5, 3, PEG_FLAT, LFO_OFF,
    },
    {   /* GLASS BELL: three pairs at bell-like ratios (1, 3, 0.5 under 3.51, 7.07, 1.41), a long ring */
        {OP(99, 30, 20, 35, 99, 70, 0, 0, 99, 1, 0, 2, 1), OP(99, 38, 25, 35, 99, 60, 0, 0, 72, 3, 17, 5, 1),
         OP(99, 34, 22, 38, 99, 60, 0, 0, 80, 3, 0, 2, 1), OP(99, 45, 30, 38, 99, 45, 0, 0, 58, 7, 1, 6, 1),
         OP(99, 28, 18, 32, 99, 75, 0, 0, 86, 0, 0, 2, 1), OP(99, 40, 28, 32, 99, 50, 0, 0, 60, 1, 41, 4, 1)},
        5, 4, PEG_FLAT, LFO_OFF,
    },
    {   /* FM BASS: a 0.5:0.5 pair that holds, a stack of three over a 1:1 carrier for the pluck */
        {OP(99, 52, 40, 78, 99, 88, 76, 0, 99, 0, 0, 2, 2), OP(99, 70, 45, 78, 99, 72, 55, 0, 80, 0, 0, 6, 2),
         OP(99, 60, 45, 78, 99, 80, 0, 0, 74, 1, 0, 3, 2), OP(99, 75, 50, 78, 99, 60, 0, 0, 72, 1, 0, 6, 2),
         OP(99, 80, 55, 78, 99, 40, 0, 0, 58, 3, 0, 6, 2), OP(99, 85, 60, 78, 99, 30, 0, 0, 55, 1, 0, 6, 2)},
        1, 5, PEG_FLAT, LFO_OFF,
    },
    {   /* BRASS: four carriers, two modulators that open after the tone, a small scoop and a late vibrato */
        {OP(62, 35, 25, 62, 99, 93, 90, 0, 97, 1, 0, 1, 1), OP(52, 40, 30, 62, 99, 88, 84, 0, 78, 1, 0, 5, 1),
         OP(62, 35, 25, 62, 99, 93, 90, 0, 92, 1, 0, 1, 1), OP(62, 35, 25, 62, 99, 93, 90, 0, 90, 1, 0, 1, 1),
         OP(62, 35, 25, 62, 99, 93, 90, 0, 78, 2, 0, 1, 1), OP(50, 40, 30, 62, 99, 88, 84, 0, 74, 1, 0, 5, 1)},
        22, 6, .pr = {84, 95, 95, 60}, .pl = {47, 50, 50, 50}, .lfo = {30, 50, 4, 0, 0, 4}, .pms = 3,
    },
    {   /* MARIMBA: two pairs that fall straight to silence, the fourth partial of a bar strong */
        {OP(99, 52, 35, 55, 99, 0, 0, 0, 99, 1, 0, 3, 3), OP(99, 72, 50, 60, 99, 0, 0, 0, 70, 4, 0, 6, 3),
         OP(99, 66, 45, 60, 99, 0, 0, 0, 72, 4, 0, 3, 3), OP(99, 80, 60, 65, 99, 0, 0, 0, 52, 10, 0, 7, 3),
         OP_OFF, OP_OFF},
        5, 0, PEG_FLAT, LFO_OFF,
    },
    {   /* FM ORGAN: six sines as drawbars (16', 8', 4', 2 2/3', 2', 1 1/3'), no velocity */
        {OP(99, 99, 99, 82, 99, 99, 99, 0, 97, 1, 0, 0, 0), OP(99, 99, 99, 82, 99, 99, 99, 0, 88, 2, 0, 0, 0),
         OP(99, 99, 99, 82, 99, 99, 99, 0, 90, 0, 0, 0, 0), OP(99, 99, 99, 82, 99, 99, 99, 0, 80, 3, 0, 0, 0),
         OP(99, 99, 99, 82, 99, 99, 99, 0, 78, 4, 0, 0, 0), OP(99, 99, 99, 82, 99, 99, 99, 0, 72, 6, 0, 0, 0)},
        32, 2, PEG_FLAT, LFO_OFF,
    },
    {   /* WARM PAD: three slow pairs, one an octave up, spread wide, a slow vibrato */
        {OP(36, 30, 22, 40, 99, 92, 88, 0, 95, 1, 0, 1, 0), OP(30, 25, 20, 40, 99, 85, 78, 0, 62, 1, 0, 2, 0),
         OP(36, 30, 22, 40, 99, 92, 88, 0, 85, 2, 0, 1, 0), OP(28, 25, 20, 40, 99, 80, 72, 0, 55, 2, 0, 2, 0),
         OP(36, 30, 22, 40, 99, 92, 88, 0, 92, 1, 0, 1, 0), OP(30, 25, 20, 40, 99, 85, 78, 0, 60, 3, 0, 2, 0)},
        5, 3, PEG_FLAT, .lfo = {28, 60, 6, 0, 0, 4}, .pms = 3,
    },
    {   /* CLAV: two bright pairs (3:1, 5:2) that fall away while held; harder is brighter */
        {OP(99, 62, 40, 72, 99, 70, 0, 0, 99, 1, 0, 3, 2), OP(99, 72, 50, 72, 99, 55, 0, 0, 78, 3, 0, 7, 2),
         OP(99, 66, 45, 72, 99, 65, 0, 0, 82, 2, 0, 3, 2), OP(99, 80, 55, 72, 99, 40, 0, 0, 72, 5, 0, 7, 2),
         OP_OFF, OP_OFF},
        5, 0, PEG_FLAT, LFO_OFF,
    },
};

/* the eight values that describe each voice (the rule at the top: ATK, DEC, REL from the slowest sounding carrier,
 * (99 - its rate) * 99 / 80; SUS its highest sustain level; MOD the loudest modulator; DTUN chosen) */
static const preset_t FM6_PRESETS[] = {
    {.name = "E.PIANO", .e = {5, 3, 76, 2, 5, 95, 0, 48}, .env = {0, 0, 127, 0}},
    {.name = "GLASS BELL", .e = {5, 4, 72, 3, 0, 88, 0, 83}, .env = {0, 0, 127, 0}},
    {.name = "FM BASS", .e = {1, 5, 80, 0, 0, 58, 76, 26}, .env = {0, 0, 127, 0}},
    {.name = "BRASS", .e = {22, 6, 78, 3, 46, 79, 90, 46}, .env = {0, 0, 127, 0}},
    {.name = "MARIMBA", .e = {5, 0, 70, 0, 0, 58, 0, 54}, .env = {0, 0, 127, 0}},
    {.name = "FM ORGAN", .e = {32, 2, 0, 1, 0, 0, 99, 21}, .env = {0, 0, 127, 0}},
    {.name = "WARM PAD", .e = {5, 3, 62, 6, 78, 85, 88, 73}, .env = {0, 0, 127, 0}},
    {.name = "CLAV", .e = {5, 0, 78, 1, 0, 46, 0, 33}, .env = {0, 0, 127, 0}},
};

static int32_t fm6_rate(int32_t v) { return v * 80 / 99; }   /* a value 0..99 as rate steps */
static int32_t fm6_clamp99(int32_t v) { return v < 0 ? 0 : v > 99 ? 99 : v; }

/* the voice of preset p as the values m play it, in msfa's layout (OP6 first) */
static void fm6_make(int p, const int16_t m[8], uint8_t out[FM_PATCH])
{
    static const int8_t DTUN_SIGN[6] = {1, -1, 1, -1, 1, -1};
    const fm6_voice_t *vc = &FM6_VOICE[p];
    const int8_t *pm = FM6_PRESETS[p].e;
    int32_t alg = clamp(m[FM_ALGO], 1, 32) - 1, k, i;
    int32_t d_atk = fm6_rate(m[FM_ATK]) - fm6_rate(pm[FM_ATK]), d_dec = fm6_rate(m[FM_DEC]) - fm6_rate(pm[FM_DEC]);
    int32_t d_rel = fm6_rate(m[FM_REL]) - fm6_rate(pm[FM_REL]);
    int32_t d_sus = m[FM_SUS] - pm[FM_SUS], d_mod = m[FM_MOD] - pm[FM_MOD];
    for (k = 0; k < 6; k++) {                       /* OP k + 1 */
        const fm6_op_t *o = &vc->op[k];
        uint8_t *d = out + (5 - k) * 21;
        d[0] = (uint8_t)fm6_clamp99(o->r[0] - d_atk);
        d[1] = (uint8_t)fm6_clamp99(o->r[1] - d_dec);
        d[2] = (uint8_t)fm6_clamp99(o->r[2] - d_dec);
        d[3] = (uint8_t)fm6_clamp99(o->r[3] - d_rel);
        d[4] = o->l[0];
        d[5] = o->l[1];
        d[6] = (uint8_t)fm6_clamp99(o->l[2] + d_sus);
        d[7] = o->l[3];
        d[8] = 39;                                  /* level scaling: none (break point C3, depths 0) */
        d[9] = d[10] = d[11] = d[12] = 0;
        d[13] = o->rs;
        d[14] = 0;                                  /* amplitude modulation sensitivity */
        d[15] = o->kvs;
        d[16] = (uint8_t)(fm_carrier((int)alg, 5 - (int)k) ? o->ol : fm6_clamp99(o->ol + d_mod));
        d[17] = 0;                                  /* ratio, not fixed */
        d[18] = o->coarse;
        d[19] = o->fine;
        d[20] = (uint8_t)(7 + DTUN_SIGN[k] * clamp(m[FM_DTUN], 0, 7));
    }
    for (i = 0; i < 4; i++) {
        out[126 + i] = vc->pr[i];
        out[130 + i] = vc->pl[i];
    }
    out[134] = (uint8_t)alg;
    out[135] = (uint8_t)clamp(m[FM_FEEDBK], 0, 7);
    out[136] = 1;                                   /* oscillator key sync */
    for (i = 0; i < 6; i++)
        out[137 + i] = vc->lfo[i];
    out[143] = vc->pms;
    out[144] = 24;                                  /* transpose: none */
    for (i = 145; i < 155; i++)
        out[i] = ' ';
    out[155] = 0x3f;
}

static struct {
    uint8_t patch[FM_PATCH];                        /* the voice as it plays */
    uint8_t ready;                                  /* patch made (every part starts without) */
    int16_t made[9];                                /* made from: the preset and the eight values */
    fm_lfo_t lfo;
    int32_t lfo_val, lfo_delay;                     /* this block's */
} fm6_part[NPART];
static fm_note_t fm6_note[NPART][NVOICE];

static void fm6_sync(struct track *t)               /* the voice made again when the sound changed */
{
    int part = (int)(t - trk), k, p = clamp(t->p[P_PRESET], 0, (int32_t)NELEM(FM6_VOICE) - 1);
    int16_t m[8];
    for (k = 0; k < 8; k++)
        m[k] = t->p[P_E0 + k];
    if (fm6_part[part].ready && fm6_part[part].made[0] == p && !memcmp(&fm6_part[part].made[1], m, sizeof m))
        return;
    fm6_make(p, m, fm6_part[part].patch);
    fm_lfo_reset(&fm6_part[part].lfo, fm6_part[part].patch + 137);
    fm6_part[part].made[0] = (int16_t)p;
    memcpy(&fm6_part[part].made[1], m, sizeof m);
    fm6_part[part].ready = 1;
}

static fm_note_t *fm6_voice(struct track *t, voice_t *v) { return &fm6_note[t - trk][v - t->v]; }

static void fm6_note_on(struct track *t, voice_t *v)
{
    fm6_sync(t);
    if (retrig)                                     /* the voice still sounds: on from where it is, no click */
        fm_note_retrigger(fm6_voice(t, v), fm6_part[t - trk].patch, v->note, v->vel);
    else
        fm_note_init(fm6_voice(t, v), fm6_part[t - trk].patch, v->note, v->vel);
    fm_lfo_keydown(&fm6_part[t - trk].lfo);
}

static void fm6_block(struct track *t)              /* the voice up to date, the LFO a block on */
{
    int part = (int)(t - trk);
    fm6_sync(t);
    fm6_part[part].lfo_val = fm_lfo_sample(&fm6_part[part].lfo);
    fm6_part[part].lfo_delay = fm_lfo_delay(&fm6_part[part].lfo);
}

static void fm6_render(struct track *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    fm_note_t *fn = fm6_voice(t, v);
    int part = (int)(t - trk);
    int32_t buf[FM_N];
    uint32_t i;
    (void)m;                                        /* its own envelopes, pitch and LFO */
    if (!v->gate && fn->down)
        fm_note_keyup(fn);
    fm_note_algorithm(fn, t->p[P_E0 + FM_ALGO] - 1, t->p[P_E0 + FM_FEEDBK]);
    memset(buf, 0, sizeof buf);
    fm_note_compute(fn, buf, fm6_part[part].lfo_val, fm6_part[part].lfo_delay);
    for (i = 0; i < n && i < (uint32_t)FM_N; i++)
        out[i] += buf[i] >> FM6_SHIFT;
}

static int fm6_done(struct track *t, voice_t *v) { return fm_note_done(fm6_voice(t, v)); }

static const engine_t ENG_FM6 = {
    .name = "FM6",
    .page_title = {"ALGO", "ENV"},
    .edit = {
        {"ALGO", F_INT, 1, 32, 5, 0, 0},
        {"FEEDBK", F_INT, 0, 7, 0, 0, 0},
        {"MOD", F_INT, 0, 99, 70, 0, 0},
        {"DTUN", F_INT, 0, 7, 0, 0, 0},
        {"ATK", F_INT, 0, 99, 0, 0, 0},
        {"DEC", F_INT, 0, 99, 50, 0, 0},
        {"SUS", F_INT, 0, 99, 0, 0, 0},
        {"REL", F_INT, 0, 99, 40, 0, 0},
    },
    .presets = FM6_PRESETS,
    .npresets = NELEM(FM6_PRESETS),
    .knob = {P_E0, P_E1, P_E2, P_E3},
    .note_on = fm6_note_on,
    .render = fm6_render,
    .block = fm6_block,
    .ownenv = 1,
    .done = fm6_done,
};
