/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* DRUMS: four kits in the style of Roland's TR-606, TR-707, TR-808 and TR-909, twelve instruments each, all
 * synthetic (no samples from the machines; the 707 and the 909's cymbals were samples, so theirs are likenesses).
 * The bass drums, snares and hats are Plaits' models (dsp/drums/drums.c, MIT); the toms, clap, rim shot and cowbell
 * are this file's. Included by engines.c after the other engines.
 *
 * One instrument a semitone, the same in every octave: C BD, C# RS, D SD, D# CP, E CB, F LT, F# CH, G MT, G# the
 * kit's own (606 and 909 RD ride, 707 TB tambourine, 808 CL claves), A HT, A# OH, B CY (BD, RS, SD, CP, CH and OH
 * where General MIDI has them). An instrument plays on one voice, as on the machines: hit again, it rings on from
 * where it is (engines.c same_note); CH ends a sounding OH; velocity is the accent; letting a key go does nothing.
 * Its own envelopes (ownenv): a voice ends after a few quiet blocks.
 *
 * The eight values (EDIT 1 DRUM: BD TUNE, BD DEC, SD TUNE, SNAPPY; EDIT 2 KIT: TOM TUNE, HAT DEC, HAT TONE, DRIVE)
 * set what they name; a kit's own are its defaults. TUNE is in semitones; HAT DEC and HAT TONE set CH and OH (the
 * cymbal keeps the kit's; HAT TONE spans the bright part of the hats' filter, 1.9 to 9.6 kHz); DRIVE saturates
 * every voice. */
#include "drums/drums.h"

#define PCT (1.0f / 99.0f)                         /* a value 0..99 as 0..1 */

#if FS != PD_FS
#error "eng_drums.c: the drum models run at FS (dsp/drums/drums.h PD_FS)"
#endif

enum { DR_BD, DR_RS, DR_SD, DR_CP, DR_CB, DR_LT, DR_CH, DR_MT, DR_X, DR_HT, DR_OH, DR_CY, DR_N };   /* DR_X: G# */
static const uint8_t DR_OF_PC[12] = {DR_BD, DR_RS, DR_SD, DR_CP, DR_CB, DR_LT, DR_CH, DR_MT, DR_X, DR_HT, DR_OH,
                                     DR_CY};
static const char *const DR_NAME[DR_N] = {"BD", "RS", "SD", "CP", "CB", "LT", "CH", "MT", "", "HT", "OH", "CY"};
enum { DM_BD808, DM_BD909, DM_SD808, DM_SD909, DM_HAT, DM_CLAP, DM_RIM, DM_COW, DM_TOM, DM_N };
enum { DE_BD_TUNE, DE_BD_DEC, DE_SD_TUNE, DE_SNAPPY, DE_TOM_TUNE, DE_HAT_DEC, DE_HAT_TONE, DE_DRIVE };

typedef struct {                                    /* an instrument of a kit */
    uint8_t model;                                  /* DM_* */
    uint16_t hz;                                    /* its pitch (hats: the lowest of the six oscillators) */
    uint8_t tone, decay, x;                         /* 0..99; x: the bass drums' FM, the snares' snappiness, the
                                                     * hats' noisiness, the toms' fall in pitch (the macros set some
                                                     * of these) */
    uint8_t level;                                  /* 0..99 */
    uint8_t bits;                                   /* hats: a sampled machine's steps, 0 = none */
} drum_inst_t;

typedef struct {
    const char *x;                                  /* the name of its instrument on G# */
    drum_inst_t inst[DR_N];
} drum_kit_t;

#define DI(m, hz, tone, dec, x, lev, bits) {m, hz, tone, dec, x, lev, bits}
static const drum_kit_t DRUM_KIT[] = {
    {"RD", {    /* 606: a small analog machine, high and short; no clap, rim shot, cowbell or ride: added */
        DI(DM_BD808, 66, 40, 0, 10, 99, 0), DI(DM_RIM, 520, 50, 15, 0, 60, 0), DI(DM_SD808, 240, 30, 30, 0, 80, 0),
        DI(DM_CLAP, 1300, 45, 30, 0, 70, 0), DI(DM_COW, 600, 55, 30, 0, 55, 0), DI(DM_TOM, 120, 30, 35, 20, 80, 0),
        DI(DM_HAT, 245, 0, 0, 5, 75, 0), DI(DM_TOM, 150, 30, 35, 20, 80, 0), DI(DM_HAT, 245, 90, 88, 15, 80, 0),
        DI(DM_TOM, 185, 30, 35, 20, 80, 0), DI(DM_HAT, 245, 0, 0, 5, 82, 0), DI(DM_HAT, 230, 80, 80, 35, 72, 0),
    }},
    {"TB", {    /* 707: samples (8 bits, the cymbals 6) in the original: 909-like voices, stepped hats */
        DI(DM_BD909, 58, 30, 0, 40, 99, 0), DI(DM_RIM, 480, 55, 12, 0, 60, 0), DI(DM_SD909, 210, 30, 40, 0, 85, 0),
        DI(DM_CLAP, 1150, 45, 40, 0, 75, 0), DI(DM_COW, 580, 50, 35, 0, 55, 0), DI(DM_TOM, 110, 40, 45, 45, 80, 0),
        DI(DM_HAT, 470, 0, 0, 55, 88, 8), DI(DM_TOM, 150, 40, 45, 45, 80, 0), DI(DM_HAT, 600, 97, 55, 75, 99, 8),
        DI(DM_TOM, 200, 40, 45, 45, 80, 0), DI(DM_HAT, 470, 0, 0, 55, 85, 8), DI(DM_HAT, 440, 85, 85, 65, 70, 6),
    }},
    {"CL", {    /* 808: analog, the long booming bass drum, toms that fall in pitch, claves */
        DI(DM_BD808, 52, 30, 0, 20, 99, 0), DI(DM_RIM, 455, 50, 20, 0, 60, 0), DI(DM_SD808, 180, 40, 35, 0, 85, 0),
        DI(DM_CLAP, 1050, 40, 45, 0, 75, 0), DI(DM_COW, 540, 50, 40, 0, 55, 0), DI(DM_TOM, 95, 35, 50, 35, 85, 0),
        DI(DM_HAT, 414, 0, 0, 15, 70, 0), DI(DM_TOM, 135, 35, 50, 35, 85, 0), DI(DM_RIM, 2500, 0, 25, 0, 60, 0),
        DI(DM_TOM, 190, 35, 50, 35, 85, 0), DI(DM_HAT, 414, 0, 0, 15, 78, 0), DI(DM_HAT, 380, 78, 85, 45, 70, 0),
    }},
    {"RD", {    /* 909: analog drums with a pitch sweep; its hats and cymbals were 6-bit samples */
        DI(DM_BD909, 52, 35, 0, 55, 99, 0), DI(DM_RIM, 500, 60, 15, 0, 60, 0), DI(DM_SD909, 190, 40, 40, 0, 85, 0),
        DI(DM_CLAP, 1200, 45, 45, 0, 75, 0), DI(DM_COW, 560, 50, 35, 0, 55, 0), DI(DM_TOM, 100, 45, 45, 60, 80, 0),
        DI(DM_HAT, 500, 0, 0, 50, 90, 6), DI(DM_TOM, 140, 45, 45, 60, 80, 0), DI(DM_HAT, 520, 92, 92, 25, 70, 6),
        DI(DM_TOM, 190, 45, 45, 60, 80, 0), DI(DM_HAT, 500, 0, 0, 50, 85, 6), DI(DM_HAT, 460, 85, 85, 60, 75, 6),
    }},
};
#undef DI

static const preset_t DRUM_PRESETS[] = {
    {.name = "606", .e = {0, 40, 0, 60, 0, 20, 70, 0}, .env = {0, 0, 127, 0}},
    {.name = "707", .e = {0, 35, 0, 55, 0, 30, 60, 0}, .env = {0, 0, 127, 0}},
    {.name = "808", .e = {0, 70, 0, 45, 0, 30, 50, 0}, .env = {0, 0, 127, 0}},
    {.name = "909", .e = {0, 50, 0, 65, 0, 35, 65, 10}, .env = {0, 0, 127, 0}},
};

static const char *drum_name(int kit, int inst)
{
    return inst == DR_X ? DRUM_KIT[clamp(kit, 0, (int32_t)NELEM(DRUM_KIT) - 1)].x : DR_NAME[inst];
}

/* ----------------------------------------- the toms, clap, rim shot, cowbell --- */
typedef struct {                                    /* noise through a band-pass: three quick bursts, then a tail */
    pd_svf_t bp;
    float burst, tail;
    int32_t t;
} clap_t;

typedef struct {                                    /* two resonators struck at once, short, hard */
    pd_svf_t r1, r2, hp;
    int32_t strike;
} rim_t;

typedef struct {                                    /* two square waves a fifth or so apart, band-passed */
    uint32_t p1, p2;
    pd_svf_t bp;
    float env1, env2;
} cow_t;

typedef struct {                                    /* a sine that falls in pitch after the stick, and the stick */
    pd_svf_t bp;
    float phase, sweep, amp, stick;
} tom_t;

#define CLAP_GAP ((int32_t)(0.010f * FS))           /* 10 ms between the bursts */
static void clap(clap_t *s, int trigger, float accent, float f, float tone, float decay, float *out, int n)
{
    const float burst_decay = 1.0f - 1.0f / (0.0025f * FS);
    const float tail_decay = 1.0f - 1.0f / ((0.03f + 0.4f * decay * decay) * FS);
    const float gain = 0.3f + 0.7f * accent;
    pd_svf_set(&s->bp, pd_tan_fast(f), 1.0f + 3.0f * tone);
    if (trigger) {
        s->t = 0;
        s->burst = s->tail = 1.0f;
    }
    while (n--) {
        s->t++;
        if (s->t == CLAP_GAP || s->t == 2 * CLAP_GAP)
            s->burst = 1.0f;
        s->burst *= burst_decay;
        s->tail *= tail_decay;
        *out++ = pd_svf(&s->bp, 2.0f * pd_random() - 1.0f, 0, 0) * (s->burst + 0.3f * s->tail) * gain;
    }
}

static void rim(rim_t *s, int trigger, float accent, float f, float tone, float decay, float *out, int n)
{
    const float hz = f * FS, tau = 0.004f + 0.03f * decay;
    pd_svf_set(&s->r1, pd_tan_fast(f), 1.0f + 3.14159f * hz * tau);
    pd_svf_set(&s->r2, pd_tan_fast(f * 3.66f < 0.45f ? f * 3.66f : 0.45f), 1.0f + 3.14159f * hz * 3.66f * tau);
    pd_svf_set(&s->hp, pd_tan_fast(300.0f / FS), 0.7f);
    if (trigger)
        s->strike = 1;
    while (n--) {
        float x = s->strike ? 30.0f * (0.3f + 0.7f * accent) : 0.0f, y, hp;
        s->strike = 0;
        y = pd_svf(&s->r1, x, 0, 0) + (0.3f + 0.7f * tone) * pd_svf(&s->r2, x, 0, 0);
        pd_svf(&s->hp, pd_soft_clip(2.0f * y), 0, &hp);
        *out++ = hp;
    }
}

static void cowbell(cow_t *s, int trigger, float accent, float f, float tone, float decay, float *out, int n)
{
    const uint32_t i1 = (uint32_t)(int32_t)(f * 2147483648.0f) << 1, i2 = (uint32_t)(int32_t)(f * 1.48f *
                                                                                               2147483648.0f) << 1;
    const float d1 = 1.0f - 1.0f / (0.012f * FS), d2 = 1.0f - 1.0f / ((0.03f + 0.25f * decay) * FS);
    float c = f * 1.48f * (1.5f + 2.5f * tone);
    pd_svf_set(&s->bp, pd_tan_fast(c < 0.45f ? c : 0.45f), 2.0f);
    if (trigger)
        s->env1 = s->env2 = 0.3f + 0.7f * accent;
    while (n--) {
        float sq;
        s->p1 += i1;
        s->p2 += i2;
        sq = (s->p1 >> 31 ? 1.0f : -1.0f) + (s->p2 >> 31 ? 1.0f : -1.0f);
        s->env1 *= d1;
        s->env2 *= d2;
        *out++ = pd_svf(&s->bp, sq, 0, 0) * (0.75f * s->env1 + 0.25f * s->env2);
    }
}

/* the 808's and 909's toms start high and fall to their pitch: sweep 0..1 starts up to 80 % above it and falls in
 * ~40 ms; the stick is band-passed noise a few ms long, tone its level */
static void tom(tom_t *s, int trigger, float accent, float f, float tone, float decay, float sweep, float *out, int n)
{
    const float sweep_decay = 1.0f - 1.0f / (0.04f * FS), stick_decay = 1.0f - 1.0f / (0.004f * FS);
    const float amp_decay = 1.0f - 1.0f / ((0.03f + 0.2f * decay) * FS);
    pd_svf_set(&s->bp, pd_tan_fast(f * 12.0f < 0.45f ? f * 12.0f : 0.45f), 1.5f);
    if (trigger) {
        s->sweep = 1.0f;
        s->amp = s->stick = 0.3f + 0.7f * accent;
    }
    while (n--) {
        s->phase += f * (1.0f + 0.8f * sweep * s->sweep);
        if (s->phase >= 1.0f)
            s->phase -= 1.0f;
        s->sweep *= sweep_decay;
        s->amp *= amp_decay;
        s->stick *= stick_decay;
        *out++ = pd_soft_clip(1.5f * pd_sine(s->phase)) * s->amp +
                 pd_svf(&s->bp, 2.0f * pd_random() - 1.0f, 0, 0) * s->stick * tone;
    }
}

/* ----------------------------------------------------------------- voices --- */
typedef struct {
    uint8_t inst, model, trig, choke;
    uint16_t quiet;                                 /* blocks in a row it was silent */
    float accent, fade, dc;
    union {
        pd_bd808_t bd808;
        pd_bd909_t bd909;
        pd_sd808_t sd808;
        pd_sd909_t sd909;
        pd_hat_t hat;
        clap_t clap;
        rim_t rim;
        cow_t cow;
        tom_t tom;
    } m;
} drum_voice_t;
static drum_voice_t drum_v[NPART][NVOICE];

/* the level each model's 0..99 is a share of, measured in the simulator (velocity 100, MASTER 50 %): the bass drums
 * peak near -24 dBFS, the snares -26, the hats -29 (docs/RESEARCH.md) */
static const float DM_NORM[DM_N] = {6.3f, 3.55f, 2.5f, 2.8f, 3.35f, 5.0f, 1.7f, 1.55f, 3.0f};

static drum_voice_t *drum_voice(struct track *t, voice_t *v) { return &drum_v[t - trk][v - t->v]; }
static const drum_inst_t *drum_inst(const struct track *t, int inst)
{
    return &DRUM_KIT[clamp(t->p[P_PRESET], 0, (int32_t)NELEM(DRUM_KIT) - 1)].inst[inst];
}

static void drums_note_on(struct track *t, voice_t *v)
{
    drum_voice_t *d = drum_voice(t, v);
    int inst = DR_OF_PC[v->note % 12], model = drum_inst(t, inst)->model, k;
    if (!retrig || d->model != model)               /* a fresh voice, or another model: from silence */
        memset(d, 0, sizeof *d);
    d->inst = (uint8_t)inst;
    d->model = (uint8_t)model;
    d->trig = 1;
    d->choke = 0;
    d->quiet = 0;
    d->fade = 1.0f;
    d->accent = (float)v->vel * (1.0f / 127.0f);
    if (inst == DR_CH)                              /* the closed hat ends the open one */
        for (k = 0; k < NVOICE; k++) {
            drum_voice_t *o = &drum_v[t - trk][k];
            if (&t->v[k] != v && t->v[k].active && o->inst == DR_OH)
                o->choke = 1;
        }
}

static float drum_tune(int32_t st) { return pd_semitones((float)st); }

static void drums_render(struct track *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    drum_voice_t *d = drum_voice(t, v);
    const drum_inst_t *in = drum_inst(t, d->inst);
    float buf[CTL], f = (float)in->hz / FS, tone = in->tone * PCT, decay = in->decay * PCT;
    float x = in->x * PCT, drive = t->p[P_E0 + DE_DRIVE] * PCT, peak = 0.0f, gain;
    uint32_t i;
    int trig = d->trig;
    (void)m;
    if (n > CTL)
        n = CTL;
    d->trig = 0;
    switch (d->inst) {                              /* what the eight values set */
    case DR_BD:
        f *= drum_tune(t->p[P_E0 + DE_BD_TUNE]);
        decay = t->p[P_E0 + DE_BD_DEC] * PCT;
        break;
    case DR_SD:
        f *= drum_tune(t->p[P_E0 + DE_SD_TUNE]);
        x = t->p[P_E0 + DE_SNAPPY] * PCT;
        break;
    case DR_LT:
    case DR_MT:
    case DR_HT:
        f *= drum_tune(t->p[P_E0 + DE_TOM_TUNE]);
        break;
    case DR_CH:
        decay = 0.10f + 0.30f * (t->p[P_E0 + DE_HAT_DEC] * PCT);
        tone = 0.6f + 0.4f * (t->p[P_E0 + DE_HAT_TONE] * PCT);   /* the filter 150 Hz * 2^(6 tone): 1.9..9.6 kHz */
        break;
    case DR_OH:
        decay = 0.45f + 0.45f * (t->p[P_E0 + DE_HAT_DEC] * PCT);
        tone = 0.6f + 0.4f * (t->p[P_E0 + DE_HAT_TONE] * PCT);
        break;
    default:
        break;
    }
    switch (d->model) {
    case DM_BD808:
        pd_bd808(&d->m.bd808, trig, d->accent, f, tone, decay, x * 4.0f < 1.0f ? x * 4.0f : 1.0f,
                 x * 4.0f - 1.0f < 0.0f ? 0.0f : x * 4.0f - 1.0f > 1.0f ? 1.0f : x * 4.0f - 1.0f, buf, (int)n);
        break;
    case DM_BD909:
        pd_bd909(&d->m.bd909, trig, d->accent, f, tone, decay, 0.4f - 0.25f * decay * decay,
                 x * 2.0f < 1.0f ? x * 2.0f : 1.0f, x * 2.0f - 1.0f > 0.0f ? x * 2.0f - 1.0f : 0.0f, buf, (int)n);
        break;
    case DM_SD808:
        pd_sd808(&d->m.sd808, trig, d->accent, f, tone, decay, x, buf, (int)n);
        break;
    case DM_SD909:
        pd_sd909(&d->m.sd909, trig, d->accent, f, tone, decay, x, buf, (int)n);
        break;
    case DM_HAT:
        pd_hat(&d->m.hat, trig, d->accent, f * 0.5f, tone, decay, x, in->bits, buf, (int)n);
        break;
    case DM_CLAP:
        clap(&d->m.clap, trig, d->accent, f, tone, decay, buf, (int)n);
        break;
    case DM_RIM:
        rim(&d->m.rim, trig, d->accent, f, tone, decay, buf, (int)n);
        break;
    case DM_TOM:
        tom(&d->m.tom, trig, d->accent, f, tone, decay, x, buf, (int)n);
        break;
    default:
        cowbell(&d->m.cow, trig, d->accent, f, tone, decay, buf, (int)n);
        break;
    }
    gain = DM_NORM[d->model] * in->level * PCT * VOICE_FS;
    for (i = 0; i < n; i++) {
        float y = buf[i];
        d->dc += 0.00285f * (y - d->dc);            /* a DC blocker, ~20 Hz: Plaits' models leave some */
        y -= d->dc;
        if (drive > 0.0f)
            y += drive * (pd_soft_clip(y * (1.0f + 6.0f * drive)) - y);
        if (d->choke)
            d->fade *= 0.995f;                      /* ended by the closed hat: gone in ~5 ms */
        y *= d->fade * gain;
        peak = pd_abs(y) > peak ? pd_abs(y) : peak;
        out[i] += (int32_t)y;
    }
    d->quiet = peak < 128.0f ? (uint16_t)(d->quiet + 1u) : 0u;  /* 128: ~-84 dBFS at full MASTER */
}

static int drums_done(struct track *t, voice_t *v) { return drum_voice(t, v)->quiet >= 8; }

static const engine_t ENG_DRUMS = {
    .name = "DRUMS",
    .page_title = {"DRUM", "KIT"},
    .edit = {
        {"BD TUNE", F_INT, -12, 12, 0, 0, 0},
        {"BD DEC", F_INT, 0, 99, 50, 0, 0},
        {"SD TUNE", F_INT, -12, 12, 0, 0, 0},
        {"SNAPPY", F_INT, 0, 99, 50, 0, 0},
        {"TOM TUNE", F_INT, -12, 12, 0, 0, 0},
        {"HAT DEC", F_INT, 0, 99, 30, 0, 0},
        {"HAT TONE", F_INT, 0, 99, 50, 0, 0},
        {"DRIVE", F_INT, 0, 99, 0, 0, 0},
    },
    .presets = DRUM_PRESETS,
    .npresets = NELEM(DRUM_PRESETS),
    .knob = {P_E0, P_E1, P_E2, P_E3},
    .note_on = drums_note_on,
    .render = drums_render,
    .ownenv = 1,
    .done = drums_done,
};
