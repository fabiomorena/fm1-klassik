/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* ACID: a monophonic bass in the style of Roland's TB-303, on Robin Schmidt's Open303 (dsp/acid/acid.c, MIT), its
 * TB_303 filter. Included by engines.c after the other engines.
 *
 * One voice (the presets are MONO): a key while another is held slides to its note without a new envelope (legato,
 * engines.c's held keys); the latest let go while another is held, the pitch slides back to that one. A note is
 * accented when it starts with velocity 100 or more: from the panel's keys the app gives 127 while ENV is held, else
 * 64; MIDI's own velocity. Its own envelopes (ownenv): a voice ends a few quiet blocks after its key.
 *
 * The eight values (EDIT 1 FILTER: TUNE, CUTOFF, RESO, ENV MOD; EDIT 2 VOICE: DECAY, ACCENT, WAVE, SLIDE) are the
 * 303's seven knobs and the slide time, in the ranges of Open303's plug-in: TUNE in semitones (it slides there, as a
 * slide does); CUTOFF 314 .. 2394 Hz and DECAY 200 .. 2000 ms, each an equal factor a step; RESO, ENV MOD and ACCENT
 * 0 .. 100 %; WAVE saw (0) to square (99); SLIDE the slide's time constant in ms (the 303's 60 ms at 60). DECAY and
 * ACCENT play from the next note, as in Open303, the others at once. */
#include "acid/acid.h"

#if FS != ACID_FS
#error "eng_acid.c: Open303 runs at FS (dsp/acid/acid.h ACID_FS)"
#endif

enum { AC_TUNE, AC_CUTOFF, AC_RESO, AC_ENV_MOD, AC_DECAY, AC_ACCENT, AC_WAVE, AC_SLIDE };
#define AC_PCT (100.0f / 99.0f)                     /* a value 0..99 as percent */
#define AC_LEVEL (2.0f * VOICE_FS)                  /* Open303's output (volume 0 dB) to the engines' */

static const preset_t ACID_PRESETS[] = {
    {.name = "ACID LINE", .e = {0, 35, 75, 60, 40, 70, 0, 60}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "SQUELCH", .e = {0, 25, 90, 85, 30, 90, 0, 60}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "DEEP SAW", .e = {0, 15, 30, 25, 60, 40, 0, 60}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "SQUARE", .e = {0, 40, 60, 50, 45, 60, 99, 60}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "RUBBER", .e = {0, 20, 70, 40, 80, 50, 60, 80}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "SCREAM", .e = {0, 60, 97, 70, 25, 99, 20, 40}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "PLUCK", .e = {0, 30, 50, 90, 5, 60, 0, 30}, .env = {0, 0, 127, 0}, .mono = 1},
    {.name = "SLOW SLIDE", .e = {0, 45, 80, 55, 55, 60, 30, 99}, .env = {0, 0, 127, 0}, .mono = 1},
};

typedef struct {
    acid_t a;
    int8_t val[8];                                  /* the values the voice was set with */
    uint8_t ready;                                  /* acid_init done */
    uint8_t note;                                   /* the key it plays (TUNE moves it) */
    uint16_t quiet;                                 /* blocks in a row it was silent */
    uint8_t lfo_cut, lfo_pitch;                     /* the part's LFO moved the cutoff, the pitch (back when it stops) */
} acid_voice_t;
static acid_voice_t acid_v[NPART];

static acid_voice_t *acid_voice(struct track *t) { return &acid_v[t - trk]; }

static void acid_value(acid_voice_t *d, int k, int v)   /* a value into Open303's units */
{
    acid_t *a = &d->a;
    switch (k) {
    case AC_TUNE:
        acid_glide(a, (float)(d->note + v));
        break;
    case AC_CUTOFF:
        acid_set_cutoff(a, 314.0f * acid_exp2(2.93065f * (float)v * (1.0f / 99.0f)));     /* log2(2394 / 314) */
        break;
    case AC_RESO:
        acid_set_resonance(a, (float)v * AC_PCT);
        break;
    case AC_ENV_MOD:
        acid_set_env_mod(a, (float)v * AC_PCT);
        break;
    case AC_DECAY:
        acid_set_decay(a, 200.0f * acid_exp2(3.32193f * (float)v * (1.0f / 99.0f)));      /* log2(10) */
        break;
    case AC_ACCENT:
        acid_set_accent(a, (float)v * AC_PCT);
        break;
    case AC_WAVE:
        acid_set_waveform(a, (float)v * (1.0f / 99.0f));
        break;
    default:
        acid_set_slide_time(a, 5.0f * (float)v);    /* Open303's slew is a fifth of the slide time */
        break;
    }
}

static void acid_sync(struct track *t)              /* the values as they are now, those that changed */
{
    acid_voice_t *d = acid_voice(t);
    int k;
    if (!d->ready) {
        acid_init(&d->a);
        acid_set_mode(&d->a, ACID_TB303);
        d->note = 48;
        for (k = 0; k < 8; k++) {
            d->val[k] = (int8_t)t->p[P_E0 + k];
            if (k != AC_TUNE)
                acid_value(d, k, d->val[k]);
        }
        d->ready = 1;
        return;
    }
    for (k = 0; k < 8; k++)
        if (d->val[k] != t->p[P_E0 + k]) {
            d->val[k] = (int8_t)t->p[P_E0 + k];
            acid_value(d, k, d->val[k]);
        }
}

static void acid_note_on(struct track *t, voice_t *v)
{
    acid_voice_t *d = acid_voice(t);
    float pitch;
    int accent = v->vel >= 100;
    acid_sync(t);
    d->note = v->note;
    d->quiet = 0;
    pitch = (float)(v->note + d->val[AC_TUNE]);
    if (legato == 2)                                /* back to a key still held */
        acid_glide(&d->a, pitch);
    else if (legato)
        acid_slide(&d->a, pitch, accent);
    else {
        if (!retrig)                                /* from silence: a clean start, as Open303's first note */
            acid_reset(&d->a);
        acid_trigger(&d->a, pitch, accent);
    }
}

/* the part's LFO: FILTER +-2 octaves of CUTOFF, PITCH as the host's (1/256 semitone, as a factor); each block */
static void acid_lfo(struct track *t, acid_voice_t *d)
{
    int part = (int)(t - trk);
    int32_t c = lfo_lin(part, KL_LFO_FILTER, 1 << 15), p = lfo_pitch(part);
    if (c || d->lfo_cut) {
        acid_set_cutoff(&d->a, 314.0f * acid_exp2(2.93065f * (float)d->val[AC_CUTOFF] * (1.0f / 99.0f) +
                                                   (float)c * (2.0f / 32768.0f)));
        d->lfo_cut = c != 0;
    }
    if (p || d->lfo_pitch) {
        acid_set_pitch_mod(&d->a, acid_exp2((float)p * (1.0f / 3072.0f)));
        d->lfo_pitch = p != 0;
    }
}

static void acid_render_v(struct track *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    acid_voice_t *d = acid_voice(t);
    float buf[CTL], peak = 0.0f;
    uint32_t i;
    (void)m;
    if (n > CTL)
        n = CTL;
    acid_sync(t);
    acid_lfo(t, d);
    if (!v->gate && d->a.note_on)                   /* its key let go */
        acid_release(&d->a);
    acid_render(&d->a, buf, (int)n);
    for (i = 0; i < n; i++) {
        float y = buf[i] * AC_LEVEL;
        peak = y > peak ? y : -y > peak ? -y : peak;
        out[i] += (int32_t)y;
    }
    d->quiet = peak < 128.0f ? (uint16_t)(d->quiet + 1u) : 0u;  /* 128: ~-84 dBFS at full MASTER */
}

static int acid_done(struct track *t, voice_t *v) { return !v->gate && acid_voice(t)->quiet >= 8; }

static const engine_t ENG_ACID = {
    .name = "ACID",
    .page_title = {"FILTER", "VOICE"},
    .edit = {
        {"TUNE", F_INT, -12, 12, 0, 0, 0},
        {"CUTOFF", F_INT, 0, 99, 35, 0, 0},
        {"RESO", F_INT, 0, 99, 75, 0, 0},
        {"ENV MOD", F_INT, 0, 99, 60, 0, 0},
        {"DECAY", F_INT, 0, 99, 40, 0, 0},
        {"ACCENT", F_INT, 0, 99, 70, 0, 0},
        {"WAVE", F_INT, 0, 99, 0, 0, 0},
        {"SLIDE", F_INT, 0, 99, 60, 0, 0},
    },
    .presets = ACID_PRESETS,
    .npresets = NELEM(ACID_PRESETS),
    .knob = {P_E0, P_E1, P_E2, P_E3},
    .note_on = acid_note_on,
    .render = acid_render_v,
    .ownenv = 1,
    .done = acid_done,
};
