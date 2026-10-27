/* SPDX-License-Identifier: MIT
 * Copyright (c) 2009 Robin Schmidt (www.rs-met.com) (Open303: rosic_Open303.cpp/.h, rosic_TeeBeeFilter.cpp/.h,
 *   rosic_BlendOscillator.cpp/.h, rosic_MipMappedWaveTable.h, rosic_AnalogEnvelope.cpp/.h,
 *   rosic_DecayEnvelope.cpp/.h, rosic_LeakyIntegrator.cpp/.h, rosic_OnePoleFilter.h, rosic_BiquadFilter.h,
 *   rosic_EllipticQuarterBandFilter.h, GlobalFunctions.h)
 * Changed 2026 by fabiomorena: ported from C++ to C, in one file; for the FM-1:
 *   - classes as one struct and functions; float instead of double, no libm: exp, pow and log as approximations of
 *     their own (acid_exp2, acid_log2); the main envelope's and the slew limiter's coefficients kept as 1 - the
 *     coefficient (in float a coefficient a hair under 1 would round their times: up to 0.5 %)
 *   - what Open303 computes at start-up in double (the wave tables' FFTs, the fixed filters' and envelopes'
 *     coefficients) made at build time by tools/gen_acid_tables.py; the tables int16, 10 of the 12 (a note under
 *     20 kHz reaches table 9), from table 5 on each half as long as the one before; the 12th-order elliptic
 *     anti-alias filter as six second-order sections (in direct form II, as Open303 has it, it would not hold in
 *     float)
 *   - the oscillator's phase an integer (Q21 of the table index) instead of a double; the pitch slew limiter
 *     (LeakyIntegrator) keeps its distance from the goal instead of its output (the same sums: in float the
 *     output would stop short of the note, a slower slide further: 3e-5 of it at 60 ms, measured)
 *   - the note list (Open303::noteOn) left to the caller: acid_trigger, acid_slide, acid_glide and acid_release are
 *     triggerNote, slideToNote, releaseNote with a key still held, and with none; acid_glide sets the pitch with
 *     the tuning (releaseNote leaves it out)
 *   - the AnalogEnvelope as Open303 drives it (attack 0, hold 0, decay 1230 ms, sustain 0): one sample of attack,
 *     then decay while the note is held, release after; the MidiNoteEvent velocity, the sequencer, the pitch wheel,
 *     the volume, the filter's other modes and drive left out (TB_303 and LP_18 kept); the normalizers n1 and n2,
 *     which Open303 sets to 1, left out
 *   - a floor of 1e-20 under the envelopes (no denormals in float)
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
#include "acid.h"
#include "acid_tables.h"

#if ACID_GEN_FS != ACID_FS
#error "acid.c: acid_tables.h is made for another rate (gen_acid_tables.py --fs)"
#endif

#define FSO ((float)(ACID_FS * ACID_OS))       /* the oversampled rate */
#define TINY 1e-20f                             /* Open303's TINY (FLT_MIN in double): against denormals */
#define PHASE_ONE (1u << 21)                    /* a table index of 1 in the phase */

/* ---------------------------------------------------------------- helpers --- */
static float acid_floor(float x)
{
    float i = (float)(int32_t)x;
    return x < i ? i - 1.0f : i;
}

float acid_exp2(float x)                        /* 2^x, |x| < 126: 2^f for |f| <= 1/2 by its series (1.2e-7 off) */
{
    union {
        float f;
        int32_t i;
    } u;
    float n = acid_floor(x + 0.5f), f = x - n;
    u.f = 1.0f + f * (0.693147181f + f * (0.240226507f + f * (0.0555041087f + f * (0.00961812911f +
                                                                                      f * (0.00133335581f +
                                                                                           f * 0.000154035304f)))));
    u.i += (int32_t)n * (1 << 23);
    return u.f;
}

static float acid_log2(float x)                 /* log2 x, x > 0: the exponent, ln of the mantissa by atanh */
{
    union {
        float f;
        int32_t i;
    } u;
    float e, m, s, s2;
    u.f = x;
    e = (float)(((u.i >> 23) & 255) - 127);
    u.i = (u.i & 0x007FFFFF) | 0x3F800000;      /* the mantissa, 1 .. 2 */
    m = u.f;
    if (m > 1.41421356f) {
        m *= 0.5f;
        e += 1.0f;
    }
    s = (m - 1.0f) / (m + 1.0f);
    s2 = s * s;
    return e + 2.88539008f * s * (1.0f + s2 * (0.333333333f + s2 * (0.2f + s2 * 0.142857143f)));
}

static float acid_exp(float x) { return acid_exp2(x * 1.44269504f); }

static float one_minus_exp_neg(float x)         /* 1 - e^-x, x >= 0, precise also where e^-x is a hair under 1 */
{
    return x < 0.01f ? x * (1.0f - x * (0.5f - x * (1.0f / 6.0f))) : 1.0f - acid_exp(-x);
}

static float pitch_to_freq(float pitch, float tuning) { return tuning * acid_exp2((pitch - 69.0f) * (1.0f / 12.0f)); }

/* ------------------------------------------------------------- parameters --- */
static void env_mod_scaler_offset(acid_t *a)    /* calculateEnvModScalerAndOffset, the measured mapping */
{
    const float c0 = 313.8152786f, c1 = 2394.411987f;   /* the lowest and highest nominal cutoff */
    const float oF = 0.048292931f, oC = 0.294391201f;
    const float sLoF = 3.773996325f, sLoC = 0.736965594f, sHiF = 4.194548788f, sHiC = 0.864344901f;
    float e = a->env_mod * 0.01f;
    float c = acid_log2(a->cutoff / c0) / acid_log2(c1 / c0);
    float lo = sLoF * e + sLoC, hi = sHiF * e + sHiC;
    a->env_scaler = (1.0f - c) * lo + c * hi;
    a->env_offset = oF * c + oC;
}

void acid_set_cutoff(acid_t *a, float hz)
{
    a->cutoff = hz;
    env_mod_scaler_offset(a);
}

void acid_set_resonance(acid_t *a, float pct)   /* TeeBeeFilter::setResonance */
{
    float r = 0.01f * pct;
    a->reso_skewed = (1.0f - acid_exp(-3.0f * r)) / (1.0f - 0.0497870684f);   /* 1 - e^-3 */
}

void acid_set_env_mod(acid_t *a, float pct)
{
    a->env_mod = pct;
    env_mod_scaler_offset(a);
}

void acid_set_decay(acid_t *a, float ms) { a->normal_decay = ms; }
void acid_set_accent(acid_t *a, float pct) { a->accent = 0.01f * pct; }
void acid_set_waveform(acid_t *a, float w) { a->blend = w; }
void acid_set_mode(acid_t *a, int mode) { a->mode = mode; }

void acid_set_slide_time(acid_t *a, float ms)   /* the slew limiter's time constant, a fifth of it (Open303) */
{
    float tau = 0.2f * ms;                      /* LeakyIntegrator::calculateCoefficient */
    if (ms >= 0.0f)
        a->slew_d = tau > 0.0f ? one_minus_exp_neg(1.0f / ((float)ACID_FS * 0.001f * tau)) : 1.0f;
}

static void main_env_decay(acid_t *a, float ms) /* setMainEnvDecay: DecayEnvelope::setDecayTimeConstant */
{
    float x;
    if (ms <= 0.001f)
        return;
    x = 1.0f / (0.001f * ms * (float)ACID_FS);  /* c = e^-x; y at a trigger 1 / c (normalizeSum false) */
    a->env_d = one_minus_exp_neg(x);
    a->env_y0 = x < 0.01f ? 1.0f + x * (1.0f + x * (0.5f + x * (1.0f / 6.0f))) : acid_exp(x);
}

void acid_reset(acid_t *a) { a->idle = 1; }

void acid_init(acid_t *a)
{
    uint32_t i;
    uint8_t *b = (uint8_t *)a;
    for (i = 0; i < sizeof *a; i++)
        b[i] = 0;
    a->tuning = 440.0f;
    a->cutoff = 1000.0f;
    a->normal_decay = 1000.0f;
    a->osc_freq = a->freq = 440.0f;
    a->idle = 1;
    a->env_y = 1.0f;
    a->amp_stage = 2;
    a->amp_release = ACID_AMP_RELEASE;
    a->slew_d = one_minus_exp_neg(1.0f / ((float)ACID_FS * 0.060f));    /* the constructor's 60 ms (setSlideTime
                                                                         * not run) */
    a->mode = ACID_TB303;
    acid_set_env_mod(a, 25.0f);
    main_env_decay(a, 200.0f);                  /* DecayEnvelope's own 200 ms until a note sets it */
}

/* ------------------------------------------------------------------ notes --- */
static void note_params(acid_t *a, int accent)  /* what triggerNote and slideToNote both set */
{
    a->accent_gain = accent ? a->accent : 0.0f;
    main_env_decay(a, accent ? 200.0f : a->normal_decay);      /* accentDecay 200 ms */
    a->amp_release = accent ? ACID_AMP_RELEASE_ACC : ACID_AMP_RELEASE;
}

void acid_trigger(acid_t *a, float pitch, int accent)
{
    if (a->idle) {                              /* from silence only: else a click */
        uint32_t k;
        a->phase = 0;
        a->fb_x1 = a->fb_y1 = a->y1 = a->y2 = a->y3 = a->y4 = 0.0f;
        a->hp1_x1 = a->hp1_y1 = a->hp2_x1 = a->hp2_y1 = a->ap_x1 = a->ap_y1 = 0.0f;
        for (k = 0; k < 4; k++)
            a->notch[k] = a->declick[k] = 0.0f;
        for (k = 0; k < 6; k++)
            a->ell[k][0] = a->ell[k][1] = 0.0f;
    }
    note_params(a, accent);
    a->osc_freq = pitch_to_freq(pitch, a->tuning);
    a->slew_e = 0.0f;
    a->env_y = a->env_y0;
    a->amp_stage = 0;
    a->note_on = 1;
    a->idle = 0;
}

static void slew_to(acid_t *a, float hz)        /* a new goal for the slew limiter, from where it is */
{
    a->slew_e = a->osc_freq + a->slew_e - hz;
    a->osc_freq = hz;
}

void acid_slide(acid_t *a, float pitch, int accent)
{
    slew_to(a, pitch_to_freq(pitch, a->tuning));
    note_params(a, accent);
    a->idle = 0;
}

void acid_glide(acid_t *a, float pitch) { slew_to(a, pitch_to_freq(pitch, a->tuning)); }

void acid_release(acid_t *a)
{
    a->note_on = 0;
    a->amp_stage = 2;
}

/* ---------------------------------------------------------------- the voice --- */
static float osc_sample(acid_t *a, uint32_t inc, int tab)   /* BlendOscillator::getSample */
{
    uint32_t sh = ACID_TAB_SHIFT[tab], p = a->phase >> sh, i = p >> 21;
    float fr = (float)(p & (PHASE_ONE - 1u)) * (1.0f / (float)PHASE_ONE);
    const int16_t *s1 = ACID_SAW + ACID_TAB_OFS[tab] + i, *s2 = ACID_SQR + ACID_TAB_OFS[tab] + i;
    float o1 = (float)s1[0] + fr * (float)(s1[1] - s1[0]);
    float o2 = (float)s2[0] + fr * (float)(s2[1] - s2[0]);
    a->phase += inc;                            /* wraps at the table's length (2048 in Q21 is 2^32) */
    return ((1.0f - a->blend) * o1 + a->blend * 0.5f * o2) * ACID_TAB_GAIN;
}

void acid_render(acid_t *a, float *out, int n)
{
    while (n-- > 0) {
        float inst, inc_f, env, t1, t2, cut, amp, y, wc, r = a->reso_skewed, b0, k, g, a1 = 0.0f;
        uint32_t inc;
        int tab, j;
        union {
            float f;
            int32_t i;
        } u;
        if (a->idle) {
            *out++ = 0.0f;
            continue;
        }
        /* the pitch through the slew limiter, the oscillator's increment and its table */
        a->slew_e -= a->slew_e * a->slew_d;
        if (a->slew_e < TINY && a->slew_e > -TINY)
            a->slew_e = 0.0f;
        inst = a->osc_freq + a->slew_e;
        if (inst > 0.0f && inst < 20000.0f)
            a->freq = inst;
        inc_f = (float)ACID_TAB_N * a->freq / FSO;
        inc = (uint32_t)(inc_f * (float)PHASE_ONE);
        u.f = inc_f;
        tab = ((u.i >> 23) & 255) - 127 + 2;    /* EXPOFDBL + 2: up to nyquist / 4 on the highest note */
        tab = tab < 0 ? 0 : tab > ACID_NTAB - 1 ? ACID_NTAB - 1 : tab;
        /* the cutoff from the main envelope and the accent's RC */
        a->env_y -= a->env_y * a->env_d;
        if (a->env_y < TINY)
            a->env_y = 0.0f;
        env = a->env_y;
        a->rc2 = (a->accent_gain > 0.0f ? env : 0.0f) + ACID_RC2 * (a->rc2 - (a->accent_gain > 0.0f ? env : 0.0f));
        t1 = a->env_scaler * (env - a->env_offset);
        t2 = a->accent_gain * a->rc2;
        cut = a->cutoff * acid_exp2(t1 + t2);
        cut = cut < 200.0f ? 200.0f : cut > 20000.0f ? 20000.0f : cut;      /* TeeBeeFilter::setCutoff */
        wc = (2.0f * 3.14159265f / FSO) * cut;
        if (a->mode == ACID_TB303) {            /* calculateCoefficientsApprox4, TB_303 */
            float fx = wc * (0.707106781f / (2.0f * 3.14159265f));
            b0 = (0.00045522346f + 6.1922189f * fx) / (1.0f + 12.358354f * fx + 4.4156345f * (fx * fx));
            k = fx * (fx * (fx * (fx * (fx * (fx + 7198.6997f) - 5837.7917f) - 476.47308f) + 614.95611f) +
                      213.87126f) + 16.998792f;
            g = k * 0.0588235294f;
            g = (g - 1.0f) * r + 1.0f;
            g = g * (1.0f + r);
            k = k * r;
        } else {                                /* the same, LP_18 */
            float wc2 = wc * wc, tmp;
            tmp = wc2 * -1.341281325e-02f + 8.168739418e-02f * wc - 2.365036766e-01f;
            tmp = wc2 * tmp + 4.439739665e-01f * wc - 6.297350825e-01f;
            tmp = wc2 * tmp + 7.529691649e-01f * wc - 8.249882474e-01f;
            tmp = wc2 * tmp + 8.736418934e-01f * wc - 9.164580250e-01f;
            tmp = wc2 * tmp + 9.583192456e-01f * wc - 9.999994950e-01f;
            a1 = wc2 * tmp + 9.999999928e-01f * wc - 1.0f;
            b0 = 1.0f + a1;
            tmp = wc2 * -4.554677016e-05f - 2.022131731e-05f * wc + 2.784706718e-03f;
            tmp = wc2 * tmp + 2.079921152e-03f * wc - 8.333236384e-02f;
            tmp = wc2 * tmp - 1.666668203e-01f * wc + 1.000000012f;
            tmp = wc2 * tmp + 4.0f * wc + 4.0f;
            k = r * tmp;
            g = 1.0f;
        }
        /* the amplitude envelope, with the main envelope while the note is held, through the de-clicker */
        if (a->amp_stage == 0) {
            a->amp = 1.0f;
            a->amp_stage = 1;
        } else if (a->amp_stage == 1) {
            a->amp -= ACID_AMP_DECAY * a->amp;
        } else {
            a->amp -= a->amp_release * a->amp;
        }
        if (a->amp < TINY)
            a->amp = 0.0f;
        amp = a->amp;
        if (a->note_on)
            amp += 0.45f * env + a->accent_gain * 4.0f * env;
        {
            float *d = a->declick;
            float o = ACID_DECLICK[0] * amp + ACID_DECLICK[1] * d[0] + ACID_DECLICK[2] * d[1] + ACID_DECLICK[3] * d[2] +
                      ACID_DECLICK[4] * d[3] + TINY;
            d[1] = d[0];
            d[0] = amp;
            d[3] = d[2];
            d[2] = o;
            amp = o;
        }
        /* oversampled: the oscillator, the high-pass before the filter, the filter, the anti-alias filter */
        y = 0.0f;
        for (j = 0; j < ACID_OS; j++) {
            float x = -osc_sample(a, inc, tab), y0, fb;
            float h = ACID_HP1[0] * x + ACID_HP1[1] * a->hp1_x1 + ACID_HP1[2] * a->hp1_y1 + TINY;
            uint32_t s;
            a->hp1_x1 = x;
            a->hp1_y1 = h;
            fb = k * a->y4;                     /* the feedback through its high-pass */
            a->fb_y1 = ACID_FBHP[0] * fb + ACID_FBHP[1] * a->fb_x1 + ACID_FBHP[2] * a->fb_y1 + TINY;
            a->fb_x1 = fb;
            if (a->mode == ACID_TB303) {
                y0 = h - a->fb_y1;
                a->y1 += 2.0f * b0 * (y0 - a->y1 + a->y2);
                a->y2 += b0 * (a->y1 - 2.0f * a->y2 + a->y3);
                a->y3 += b0 * (a->y2 - 2.0f * a->y3 + a->y4);
                a->y4 += b0 * (a->y3 - 2.0f * a->y4);
                x = 2.0f * g * a->y4;
            } else {
                y0 = 0.125f * h - a->fb_y1;
                a->y1 = y0 + a1 * (y0 - a->y1);
                a->y2 = a->y1 + a1 * (a->y1 - a->y2);
                a->y3 = a->y2 + a1 * (a->y2 - a->y3);
                a->y4 = a->y3 + a1 * (a->y3 - a->y4);
                x = 8.0f * a->y3;
            }
            x += TINY;
            for (s = 0; s < 6; s++) {           /* EllipticQuarterBandFilter, in sections */
                const float *c = ACID_ELL[s];
                float *w = a->ell[s], o = c[0] * x + w[0];
                w[0] = c[1] * x - c[3] * o + w[1];
                w[1] = c[2] * x - c[4] * o;
                x = o;
            }
            y = x * ACID_ELL_GAIN;
        }
        /* after the filter, at the rate: an all-pass, a high-pass, a notch; the amplitude */
        {
            float o = ACID_ALLPASS[0] * y + ACID_ALLPASS[1] * a->ap_x1 + ACID_ALLPASS[2] * a->ap_y1 + TINY;
            a->ap_x1 = y;
            a->ap_y1 = o;
            y = o;
            o = ACID_HP2[0] * y + ACID_HP2[1] * a->hp2_x1 + ACID_HP2[2] * a->hp2_y1 + TINY;
            a->hp2_x1 = y;
            a->hp2_y1 = o;
            y = o;
        }
        {
            float *d = a->notch;
            float o = ACID_NOTCH[0] * y + ACID_NOTCH[1] * d[0] + ACID_NOTCH[2] * d[1] + ACID_NOTCH[3] * d[2] +
                      ACID_NOTCH[4] * d[3] + TINY;
            d[1] = d[0];
            d[0] = y;
            d[3] = d[2];
            d[2] = o;
            y = o;
        }
        *out++ = y * amp;
    }
}
