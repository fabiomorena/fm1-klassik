/* SPDX-License-Identifier: MIT
 * Copyright 2014, 2016 Emilie Gillet (Mutable Instruments Plaits: plaits/dsp/drums/; stmlib: dsp/filter.h,
 *   dsp/dsp.h, dsp/units.h, utils/random.h)
 * Changed 2026 by fabiomorena: ported from C++ to C, see drums.c.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE. */
/* Plaits' drum models (https://github.com/pichenettes/eurorack, plaits/dsp/drums/) for KLASSIK's DRUMS
 * (dsp/eng_drums.c): the 808 bass drum and snare, the "909-ish" bass drum and snare, the 808 hi-hat. Each renders
 * n samples of one voice in float (the AC79 has a single-precision FPU; no double, no libm). f0 and the filters'
 * frequencies are in cycles a sample; tone, decay and the rest 0..1, as in Plaits. A trigger starts a hit on the
 * state as it is (a retrigger rings on); a zeroed state is a silent one. */
#pragma once
#include <stdint.h>

#ifndef PD_FS
#define PD_FS 44100                     /* the rate the models run at (Plaits': 48000) */
#endif

typedef struct { float g, r, h, s1, s2; } pd_svf_t;      /* stmlib's Svf */
typedef struct { float g, gi, s; } pd_onepole_t;         /* stmlib's OnePole */

typedef struct {                        /* AnalogBassDrum: the 808's */
    int32_t pulse_left, fm_pulse_left;
    float pulse, pulse_height, pulse_lp, fm_pulse_lp, retrig_pulse, lp_out, tone_lp;
    pd_svf_t resonator;
} pd_bd808_t;

typedef struct {                        /* SyntheticBassDrum: "inadvertently 909-ish" */
    float f0, phase, phase_noise, fm, fm_lp, body_env, body_env_lp, transient_env, transient_env_lp, tone_lp;
    float click_lp, click_hp, noise_lp, noise_hp;
    pd_svf_t click_filter;
    int32_t body_env_pulse_width, fm_pulse_width;
} pd_bd909_t;

typedef struct {                        /* AnalogSnareDrum: the 808's */
    int32_t pulse_left;
    float pulse, pulse_height, pulse_lp, noise_envelope;
    pd_svf_t resonator[5], noise_filter;
} pd_sd808_t;

typedef struct {                        /* SyntheticSnareDrum: 909 numbers */
    float phase[2], drum_amplitude, snare_amplitude, fm;
    int32_t hold_counter;
    pd_onepole_t drum_lp, snare_hp;
    pd_svf_t snare_lp;
} pd_sd909_t;

typedef struct {                        /* HiHat<SquareNoise, SwingVCA, resonance, one-stage envelope>: the 808's */
    float envelope, noise_clock, noise_sample;
    uint32_t phase[6];
    pd_svf_t color, hpf;
} pd_hat_t;

void pd_bd808(pd_bd808_t *s, int trigger, float accent, float f0, float tone, float decay, float attack_fm,
              float self_fm, float *out, int n);
void pd_bd909(pd_bd909_t *s, int trigger, float accent, float f0, float tone, float decay, float dirtiness,
              float fm_amount, float fm_decay, float *out, int n);
void pd_sd808(pd_sd808_t *s, int trigger, float accent, float f0, float tone, float decay, float snappy, float *out,
              int n);
void pd_sd909(pd_sd909_t *s, int trigger, float accent, float f0, float fm_amount, float decay, float snappy,
              float *out, int n);
/* added: bits 1..8 steps the metallic noise as a sampled machine's would be (0: as the 808, unstepped) */
void pd_hat(pd_hat_t *s, int trigger, float accent, float f0, float tone, float decay, float noisiness, int bits,
            float *out, int n);

float pd_semitones(float st);           /* stmlib's SemitonesToRatio: 2^(st / 12) */
float pd_random(void);                  /* stmlib's Random::GetFloat: 0 .. 1 */

/* stmlib's tan approximations (Svf::set_f_q, OnePole::set_f), their constants as floats */
static inline float pd_tan_dirty(float f) { return f * (3.14159265f + 11.5839450f * f * f); }
static inline float pd_tan_fast(float f)
{
    float f2 = f * f;
    return f * (3.14159265f + f2 * (10.1080462f + 55.7873885f * f2));
}
static inline float pd_tan_accurate(float f)
{
    float f2 = f * f;
    return f * (3.14159265f + f2 * (10.3353657f + f2 * (40.8207000f + f2 * (161.205313f + f2 * (86.4620378f +
        f2 * 2799.88348f)))));
}
static inline void pd_svf_set(pd_svf_t *s, float g, float resonance)
{
    s->g = g;
    s->r = 1.0f / resonance;
    s->h = 1.0f / (1.0f + s->r * s->g + s->g * s->g);
}
/* one sample: its band-pass, low-pass and high-pass at once */
static inline float pd_svf(pd_svf_t *s, float in, float *lp, float *hp)
{
    float h = (in - s->r * s->s1 - s->g * s->s1 - s->s2) * s->h, bp = s->g * h + s->s1, l;
    s->s1 = s->g * h + bp;
    l = s->g * bp + s->s2;
    s->s2 = s->g * bp + l;
    if (lp)
        *lp = l;
    if (hp)
        *hp = h;
    return bp;
}
static inline float pd_abs(float x) { return x < 0.0f ? -x : x; }
static inline float pd_soft_limit(float x) { return x * (27.0f + x * x) / (27.0f + 9.0f * x * x); }
static inline float pd_soft_clip(float x) { return x < -3.0f ? -1.0f : x > 3.0f ? 1.0f : pd_soft_limit(x); }
