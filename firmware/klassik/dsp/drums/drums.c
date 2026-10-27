/* SPDX-License-Identifier: MIT
 * Copyright 2014, 2016 Emilie Gillet (Mutable Instruments Plaits: plaits/dsp/drums/analog_bass_drum.h,
 *   synthetic_bass_drum.h, analog_snare_drum.h, synthetic_snare_drum.h, hi_hat.h; stmlib: dsp/filter.h, dsp/dsp.h,
 *   dsp/units.h, utils/random.h)
 * Changed 2026 by fabiomorena: ported from C++ to C, in one file; for the FM-1:
 *   - classes as structs and functions; Plaits' sustain mode (free-running oscillators) left out: every hit decays
 *   - 44.1 kHz: the times Plaits gives in seconds follow the rate as they do there; the decays Plaits gives a
 *     sample (the 808 snare's noise, the hi-hat's envelope) are scaled by 48000 / 44100 to keep their times
 *   - no tables and no libm: SemitonesToRatio, Sine and Sqrt are approximations of their own (pd_exp2, pd_sine,
 *     pd_sqrt); the tan approximations' constants written out as floats
 *   - integer conversions kept signed (the random word, the square oscillators' increments)
 *   - the hi-hat: SquareNoise, SwingVCA, the resonant colour filter and the one-stage envelope (Plaits' first HiHat);
 *     added: a step count (bits) for the metallic noise, as a sampled machine's hats
 *   - SyntheticBassDrumClick's filter set at each render (it never changes)
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
#include "drums.h"

#define R48 (48000.0f / PD_FS)                  /* a decay Plaits gives a sample, at this rate */

/* ---------------------------------------------------------------- helpers --- */
static float pd_floor(float x)
{
    float i = (float)(int32_t)x;
    return x < i ? i - 1.0f : i;
}

static float pd_exp2(float x)                   /* 2^x, |x| < 100: 2^frac by its series (to x^6, 1.5e-4 off) */
{
    union {
        float f;
        int32_t i;
    } u;
    float fl = pd_floor(x), f = x - fl;
    u.f = 1.0f + f * (0.693147181f + f * (0.240226507f + f * (0.0555041087f + f * (0.00961812911f +
                                                                                      f * 0.00133335581f))));
    u.i += (int32_t)fl * (1 << 23);             /* the exponent (a negative one too: no shift of a sign) */
    return u.f;
}

float pd_semitones(float st) { return pd_exp2(st * (1.0f / 12.0f)); }

float pd_sine(float phase)                      /* sin(2 pi phase), a parabola refined: under 0.1 % of full off */
{
    float t = phase - pd_floor(phase) - 0.5f, s = 8.0f * t - 16.0f * t * pd_abs(t);
    s = 0.225f * (s * pd_abs(s) - s) + s;
    return -s;
}

static float pd_sqrt(float x)                   /* x >= 0; a guess from the bits, three Newton steps */
{
    union {
        float f;
        int32_t i;
    } u;
    int k;
    if (x <= 0.0f)
        return 0.0f;
    u.f = x;
    u.i = (u.i >> 1) + 0x1FC00000;
    for (k = 0; k < 3; k++)
        u.f = 0.5f * (u.f + x / u.f);
    return u.f;
}

static uint32_t rng_state;
float pd_random(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return (float)(int32_t)(rng_state >> 8) * (1.0f / 16777216.0f);
}

static float pd_clamp(float x, float lo, float hi) { return x < lo ? lo : x > hi ? hi : x; }

static float diode(float x)
{
    if (x >= 0.0f)
        return x;
    x *= 2.0f;
    return 0.7f * x / (1.0f + pd_abs(x));
}

/* the trigger pulse the 808's bass drum and snare share (Q39 / Q40, Q45 / Q46) */
static float trigger_pulse(int32_t *left, float *held, float height, float decay)
{
    float pulse;
    if (*left) {
        --*left;
        pulse = *left ? height : height - 1.0f;
        *held = pulse;
    } else {
        *held *= decay;
        pulse = *held;
    }
    return pulse;
}

/* ---------------------------------------------------------- 808 bass drum --- */
void pd_bd808(pd_bd808_t *s, int trigger, float accent, float f0, float tone, float decay, float attack_fm,
              float self_fm, float *out, int n)
{
    const float pulse_decay = 1.0f - 1.0f / (0.2e-3f * PD_FS), pulse_filter = 1.0f / (0.1e-3f * PD_FS);
    const float retrig_decay = 1.0f - 1.0f / (0.05f * PD_FS);
    const float scale = 0.001f / f0, q = 1500.0f * pd_semitones(decay * 80.0f);
    const float tone_f = pd_clamp(4.0f * f0 * pd_semitones(tone * 108.0f), 0.0f, 1.0f);
    const float exciter_leak = 0.08f * (tone + 0.25f);
    if (trigger) {
        s->pulse_left = (int32_t)(1.0e-3f * PD_FS);
        s->fm_pulse_left = (int32_t)(6.0e-3f * PD_FS);
        s->pulse_height = 3.0f + 7.0f * accent;
        s->lp_out = 0.0f;
    }
    while (n--) {
        float pulse = trigger_pulse(&s->pulse_left, &s->pulse, s->pulse_height, pulse_decay), fm_pulse = 0.0f;
        float punch, f, resonator_out;
        s->pulse_lp += pulse_filter * (pulse - s->pulse_lp);                    /* C40 / R163 / R162 / D83 */
        pulse = diode((pulse - s->pulse_lp) + pulse * 0.044f);
        if (s->fm_pulse_left) {                                                  /* Q41 / Q42 */
            --s->fm_pulse_left;
            fm_pulse = 1.0f;
            s->retrig_pulse = s->fm_pulse_left ? 0.0f : -0.8f;                   /* C39 / C52 */
        } else {
            s->retrig_pulse *= retrig_decay;                                     /* C39 / R161 */
        }
        s->fm_pulse_lp += pulse_filter * (fm_pulse - s->fm_pulse_lp);
        punch = 0.7f + diode(10.0f * s->lp_out - 1.0f);                         /* Q43 and R170 leakage */
        f = f0 * (1.0f + s->fm_pulse_lp * 1.7f * attack_fm + punch * 0.08f * self_fm);   /* Q43 / R165 */
        f = pd_clamp(f, 0.0f, 0.4f);
        pd_svf_set(&s->resonator, pd_tan_dirty(f), 1.0f + q * f);
        resonator_out = pd_svf(&s->resonator, (pulse - s->retrig_pulse * 0.2f) * scale, &s->lp_out, 0);
        s->tone_lp += tone_f * (pulse * exciter_leak + resonator_out - s->tone_lp);
        *out++ = s->tone_lp;
    }
}

/* ------------------------------------------------------ 909-ish bass drum --- */
static float distorted_sine(float phase, float phase_noise, float dirtiness)
{
    float frac, triangle, sine;
    phase += phase_noise * dirtiness;
    frac = phase - (float)(int32_t)phase;
    triangle = (frac < 0.5f ? frac : 1.0f - frac) * 4.0f - 1.0f;
    sine = 2.0f * triangle / (1.0f + pd_abs(triangle));
    return sine + (1.0f - dirtiness) * (pd_sine(frac + 0.75f) - sine);
}

static float transistor_vca(float s, float gain)
{
    s = (s - 0.6f) * gain;
    return 3.0f * s / (2.0f + pd_abs(s)) + gain * 0.3f;
}

void pd_bd909(pd_bd909_t *s, int trigger, float accent, float f0, float tone, float decay, float dirtiness,
              float fm_amount, float fm_decay, float *out, int n)
{
    float f0_step, fm_decay_k, body_decay, tone_f, d;
    const float transient_decay = 1.0f - 1.0f / (0.005f * PD_FS);
    decay *= decay;
    fm_decay *= fm_decay;
    f0_step = (f0 - s->f0) / (float)n;          /* ParameterInterpolator: a step toward f0 at each use */
    d = 1.0f - 8.0f * f0;
    dirtiness *= d > 0.0f ? d : 0.0f;
    fm_decay_k = 1.0f - 1.0f / (0.008f * (1.0f + fm_decay * 4.0f) * PD_FS);
    body_decay = 1.0f - 1.0f / (0.02f * PD_FS) * pd_semitones(-decay * 60.0f);
    tone_f = pd_clamp(4.0f * f0 * pd_semitones(tone * 108.0f), 0.0f, 1.0f);
    pd_svf_set(&s->click_filter, pd_tan_fast(5000.0f / PD_FS), 2.0f);
    if (trigger) {
        s->fm = 1.0f;
        s->body_env = s->transient_env = 0.3f + 0.7f * accent;
        s->body_env_pulse_width = (int32_t)(PD_FS * 0.001f);
        s->fm_pulse_width = (int32_t)(PD_FS * 0.0013f);
    }
    while (n--) {
        float mix = 0.0f, body, transient, click_in, err, lp;
        s->phase_noise += 0.002f * (pd_random() - 0.5f - s->phase_noise);
        if (s->fm_pulse_width) {
            --s->fm_pulse_width;
            s->phase = 0.25f;
        } else {
            float inc;
            s->fm *= fm_decay_k;
            s->f0 += f0_step;                   /* not during the pulse: Plaits' f0 lags it, as there */
            inc = s->f0 * (1.0f + fm_amount * 3.5f * s->fm_lp);
            s->phase += inc < 0.5f ? inc : 0.5f;
            if (s->phase >= 1.0f)
                s->phase -= 1.0f;
        }
        if (s->body_env_pulse_width) {
            --s->body_env_pulse_width;
        } else {
            s->body_env *= body_decay;
            s->transient_env *= transient_decay;
        }
        s->body_env_lp += 0.1f * (s->body_env - s->body_env_lp);
        s->transient_env_lp += 0.1f * (s->transient_env - s->transient_env_lp);
        s->fm_lp += 0.1f * (s->fm - s->fm_lp);
        body = distorted_sine(s->phase, s->phase_noise, dirtiness);
        click_in = s->body_env_pulse_width ? 0.0f : 1.0f;                     /* SyntheticBassDrumClick */
        err = click_in - s->click_lp;
        s->click_lp += (err > 0.0f ? 0.5f : 0.1f) * err;
        s->click_hp += 0.04f * (s->click_lp - s->click_hp);
        pd_svf(&s->click_filter, s->click_lp - s->click_hp, &lp, 0);
        s->noise_lp += 0.05f * (pd_random() - s->noise_lp);                    /* SyntheticBassDrumAttackNoise */
        s->noise_hp += 0.005f * (s->noise_lp - s->noise_hp);
        transient = lp + (s->noise_lp - s->noise_hp);
        mix -= transistor_vca(body, s->body_env_lp);
        mix -= transient * s->transient_env_lp * tone;
        s->tone_lp += tone_f * (mix - s->tone_lp);
        *out++ = s->tone_lp;
    }
}

/* -------------------------------------------------------------- 808 snare --- */
void pd_sd808(pd_sd808_t *s, int trigger, float accent, float f0, float tone, float decay, float snappy, float *out,
              int n)
{
    static const float MODES[5] = {1.00f, 2.00f, 3.18f, 4.16f, 5.62f};
    const float decay_xt = decay * (1.0f + decay * (decay - 1.0f));
    const float pulse_decay = 1.0f - 1.0f / (0.1e-3f * PD_FS);
    const float q = 2000.0f * pd_semitones(decay_xt * 84.0f);
    const float noise_decay = 1.0f - 0.0017f * R48 * pd_semitones(-decay * (50.0f + snappy * 10.0f));
    const float exciter_leak = snappy * (2.0f - snappy) * 0.1f;
    float f[5], gain[5], f_noise;
    int i;
    snappy = pd_clamp(snappy * 1.1f - 0.05f, 0.0f, 1.0f);
    if (trigger) {
        s->pulse_left = (int32_t)(1.0e-3f * PD_FS);
        s->pulse_height = 3.0f + 7.0f * accent;
        s->noise_envelope = 2.0f;
    }
    for (i = 0; i < 5; i++) {
        f[i] = f0 * MODES[i] < 0.499f ? f0 * MODES[i] : 0.499f;
        pd_svf_set(&s->resonator[i], pd_tan_fast(f[i]), 1.0f + f[i] * (i == 0 ? q : q * 0.25f));
    }
    if (tone < 0.666667f) {                     /* 808-style (2 modes) */
        tone *= 1.5f;
        gain[0] = 1.5f + (1.0f - tone) * (1.0f - tone) * 4.5f;
        gain[1] = 2.0f * tone + 0.15f;
        gain[2] = gain[3] = gain[4] = 0.0f;
    } else {                                    /* "what the 808 could have been if there were extra modes" */
        tone = (tone - 0.666667f) * 3.0f;
        gain[0] = 1.5f - tone * 0.5f;
        gain[1] = 2.15f - tone * 0.7f;
        for (i = 2; i < 5; i++) {
            gain[i] = tone;
            tone *= tone;
        }
    }
    f_noise = pd_clamp(f0 * 16.0f, 0.0f, 0.499f);
    pd_svf_set(&s->noise_filter, pd_tan_fast(f_noise), 1.0f + f_noise * 1.5f);
    while (n--) {
        float pulse = trigger_pulse(&s->pulse_left, &s->pulse, s->pulse_height, pulse_decay), shell = 0.0f, noise;
        s->pulse_lp += 0.75f * (pulse - s->pulse_lp);    /* R189 / C57 / R190 + C58 / C59 / R197 / R196 / IC14 */
        for (i = 0; i < 5; i++) {
            float excitation = i == 0 ? (pulse - s->pulse_lp) + 0.006f * pulse : 0.026f * pulse;
            shell += gain[i] * (pd_svf(&s->resonator[i], excitation, 0, 0) + excitation * exciter_leak);
        }
        shell = pd_soft_clip(shell);
        noise = 2.0f * pd_random() - 1.0f;                      /* C56 / R194 / Q48 / C54 / R188 / D54 */
        if (noise < 0.0f)
            noise = 0.0f;
        s->noise_envelope *= noise_decay;
        noise *= s->noise_envelope * snappy * 2.0f;
        noise = pd_svf(&s->noise_filter, noise, 0, 0);          /* C66 / R201 / C67 / R202 / R203 / Q49 */
        *out++ = noise + shell * (1.0f - snappy);               /* IC13 */
    }
}

/* ---------------------------------------------------------- 909-ish snare --- */
static float onepole_lp(pd_onepole_t *p, float in)
{
    float lp = (p->g * in + p->s) * p->gi;
    p->s = p->g * (in - lp) + lp;
    return lp;
}

static void onepole_set(pd_onepole_t *p, float f)
{
    p->g = pd_tan_fast(f);
    p->gi = 1.0f / (1.0f + p->g);
}

static float snare_sine(float phase)
{
    float triangle = (phase < 0.5f ? phase : 1.0f - phase) * 4.0f - 1.3f;
    return 2.0f * triangle / (1.0f + pd_abs(triangle));
}

void pd_sd909(pd_sd909_t *s, int trigger, float accent, float f0, float fm_amount, float decay, float snappy,
              float *out, int n)
{
    const float decay_xt = decay * (1.0f + decay * (decay - 1.0f));
    const float fm_decay = 1.0f - 1.0f / (0.007f * PD_FS);
    float drum_decay, snare_decay, drum_level, snare_level, reset_amount;
    fm_amount *= fm_amount;
    drum_decay = 1.0f - 1.0f / (0.015f * PD_FS) * pd_semitones(-decay_xt * 72.0f - fm_amount * 12.0f + snappy * 7.0f);
    snare_decay = 1.0f - 1.0f / (0.01f * PD_FS) * pd_semitones(-decay * 60.0f - snappy * 7.0f);
    snappy = pd_clamp(snappy * 1.1f - 0.05f, 0.0f, 1.0f);
    drum_level = pd_sqrt(1.0f - snappy);
    snare_level = pd_sqrt(snappy);
    onepole_set(&s->snare_hp, 10.0f * f0 < 0.5f ? 10.0f * f0 : 0.5f);
    pd_svf_set(&s->snare_lp, pd_tan_fast(35.0f * f0 < 0.5f ? 35.0f * f0 : 0.5f), 0.5f + 2.0f * snappy);
    onepole_set(&s->drum_lp, 3.0f * f0);
    if (trigger) {
        s->snare_amplitude = s->drum_amplitude = 0.3f + 0.7f * accent;
        s->fm = 1.0f;
        s->phase[0] = s->phase[1] = 0.0f;
        s->hold_counter = (int32_t)((0.04f + decay * 0.03f) * PD_FS);
    }
    /* "the 909 circuit has a funny kind of oscillator coupling": the oscillators' resets intermodulate */
    reset_amount = pd_clamp((0.125f - f0) * 8.0f, 0.0f, 1.0f);
    reset_amount *= reset_amount * fm_amount;
    while (n--) {
        float reset_noise = 0.0f, f, drum, noise, snare, lp;
        /* the drum's envelope has a very long tail; the snare's holds 40 to 70 ms */
        s->drum_amplitude *= (s->drum_amplitude > 0.03f || !(n & 1)) ? drum_decay : 1.0f;
        if (s->hold_counter)
            --s->hold_counter;
        else
            s->snare_amplitude *= snare_decay;
        s->fm *= fm_decay;
        reset_noise += s->phase[0] > 0.5f ? -1.0f : 1.0f;
        reset_noise += s->phase[1] > 0.5f ? -1.0f : 1.0f;
        reset_noise *= reset_amount * 0.025f;
        f = f0 * (1.0f + fm_amount * (4.0f * s->fm));
        s->phase[0] += f;
        s->phase[1] += f * 1.47f;
        if (reset_amount > 0.1f) {
            if (s->phase[0] >= 1.0f + reset_noise)
                s->phase[0] = 1.0f - s->phase[0];
            if (s->phase[1] >= 1.0f + reset_noise)
                s->phase[1] = 1.0f - s->phase[1];
        } else {
            if (s->phase[0] >= 1.0f)
                s->phase[0] -= 1.0f;
            if (s->phase[1] >= 1.0f)
                s->phase[1] -= 1.0f;
        }
        drum = -0.1f + snare_sine(s->phase[0]) * 0.60f + snare_sine(s->phase[1]) * 0.25f;
        drum = onepole_lp(&s->drum_lp, drum * s->drum_amplitude * drum_level);
        noise = pd_random();
        pd_svf(&s->snare_lp, noise, &lp, 0);
        snare = lp - onepole_lp(&s->snare_hp, lp);
        snare = (snare + 0.1f) * (s->snare_amplitude + s->fm) * snare_level;
        *out++ = snare + drum;                  /* "it's a snare, it's a drum, it's a snare drum" */
    }
}

/* ----------------------------------------------------------- 808 hi-hat --- */
void pd_hat(pd_hat_t *s, int trigger, float accent, float f0, float tone, float decay, float noisiness, int bits,
            float *out, int n)
{
    static const float RATIOS[6] = {1.0f, 1.304f, 1.466f, 1.787f, 1.932f, 2.536f};   /* nominal f0: 414 Hz */
    const float envelope_decay = 1.0f - 0.003f * R48 * pd_semitones(-decay * 84.0f);
    const float steps = bits > 0 ? (float)(1 << (bits - 1)) : 0.0f;
    float cutoff, noise_f;
    uint32_t inc[6];
    int i, k;
    if (trigger)
        s->envelope = (1.5f + 0.5f * (1.0f - decay)) * (0.3f + 0.7f * accent);
    for (k = 0; k < 6; k++) {                   /* SquareNoise: six square oscillators, at 2 f0 */
        float f = 2.0f * f0 * RATIOS[k];
        inc[k] = (uint32_t)(int32_t)((f < 0.499f ? f : 0.499f) * 2147483648.0f) << 1;
    }
    for (i = 0; i < n; i++) {
        uint32_t sum = 0;
        for (k = 0; k < 6; k++) {
            s->phase[k] += inc[k];
            sum += s->phase[k] >> 31;
        }
        out[i] = 0.33f * (float)(int32_t)sum - 1.0f;
    }
    cutoff = pd_clamp(150.0f / PD_FS * pd_semitones(tone * 72.0f), 0.0f, 16000.0f / PD_FS);   /* the BPF */
    pd_svf_set(&s->color, pd_tan_accurate(cutoff), 3.0f + 3.0f * tone);
    for (i = 0; i < n; i++)
        out[i] = pd_svf(&s->color, out[i], 0, 0);
    /* "not at all part of the 808 circuit": clocked noise mixed into the oscillators' */
    noisiness *= noisiness;
    noise_f = pd_clamp(f0 * (16.0f + 16.0f * (1.0f - noisiness)), 0.0f, 0.5f);
    for (i = 0; i < n; i++) {
        s->noise_clock += noise_f;
        if (s->noise_clock >= 1.0f) {
            s->noise_clock -= 1.0f;
            s->noise_sample = pd_random() - 0.5f;
        }
        out[i] += noisiness * (s->noise_sample - out[i]);
        if (steps > 0.0f) {                     /* added: stepped, as a sample would be */
            float x = pd_clamp(out[i], -1.0f, 1.0f) * steps;
            out[i] = (float)(int32_t)(x + (x >= 0.0f ? 0.5f : -0.5f)) / steps;
        }
    }
    for (i = 0; i < n; i++) {                   /* SwingVCA */
        float x = out[i];
        s->envelope *= envelope_decay;
        x *= x > 0.0f ? 4.0f : 0.1f;
        x = x / (1.0f + pd_abs(x));
        out[i] = (x + 0.1f) * s->envelope;
    }
    pd_svf_set(&s->hpf, pd_tan_accurate(cutoff), 0.5f);
    for (i = 0; i < n; i++) {
        float hp;
        pd_svf(&s->hpf, out[i], 0, &hp);
        out[i] = hp;
    }
}
