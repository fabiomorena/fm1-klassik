/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2012, 2013 Google Inc. (msfa: dx7note.cc, env.cc, pitchenv.cc, lfo.cc, fm_core.cc, fm_op_kernel.cc,
 *   sin.h, exp2.h, freqlut.cc)
 * Changed 2026 by fabiomorena: ported from C++ to C, in one file; for the FM-1:
 *   - the tables come from the build (tools/gen_fm6_tables.py: fm6_tables.h), not from init() with double maths;
 *     osc_freq's fine frequency is one of them (FM_FINE) instead of a log a note
 *   - a block is 32 samples (LG_N 5, msfa's 6); the envelopes' and the LFO's rates follow it as msfa's do
 *   - no 64-bit multiply inline (felucca-bsp docs/CPU.md: beside loads it now and then leaves a register a bit
 *     off): the sine's interpolation fits 32 bits as it is (its step is under 2^17, the fraction under 2^14); an
 *     operator's gain is applied in 32 bits (the sine to 15 bits, the gain to 16, a shift a block); the
 *     block-rate products (exp2, the frequency, the LFO's pitch) go through mul_shr, a function of its own
 *   - the phases in unsigned arithmetic (a turn wraps: msfa's int32_t overflows); Freqlut's and Exp2's shifts kept
 *     to 0..31
 *   - no controllers (pitch bend at its middle), no NEON kernels; FmCore's two buses shared by every voice (one
 *     render at a time)
 *   - added: fm_note_algorithm, fm_note_done, fm_carrier; the LFO's random state starts at 0
 *   - added: fm_note_retrigger, a sounding voice started again without a step (its operators' phases, gains and
 *     envelope levels kept, and its feedback)
 *   - added: fm_note_compute's pitch_add, a pitch offset the host's own LFO gives (Q24 an octave, as basepitch)
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License in LICENSE beside this file, or at
 * http://www.apache.org/licenses/LICENSE-2.0. Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and limitations under
 * the License. */
#include <stdint.h>
#include "msfa.h"
#include "fm6_tables.h"

#if FM_GEN_LG_N != FM_LG_N
#error "msfa.c: fm6_tables.h was made for another block length (tools/gen_fm6_tables.py LG_N)"
#endif

#define SIN_LG_N_SAMPLES 10
#define SIN_N_SAMPLES (1 << SIN_LG_N_SAMPLES)
#define EXP2_LG_N_SAMPLES 10
#define EXP2_N_SAMPLES (1 << EXP2_LG_N_SAMPLES)
#define FREQ_LG_N_SAMPLES 10
#define FREQ_SAMPLE_SHIFT (24 - FREQ_LG_N_SAMPLES)
#define MAX_LOGFREQ_INT 20

static int min_i(int a, int b) { return a < b ? a : b; }
static int max_i(int a, int b) { return a > b ? a : b; }

/* (a * b) >> sh in 64 bits, out of line (docs/CPU.md): the block-rate products only */
static __attribute__((noinline)) int32_t mul_shr(int64_t a, int32_t b, int sh)
{
    return (int32_t)((a * b) >> sh);
}

static int32_t shr_31(int32_t y, int sh) { return sh > 31 ? 0 : sh < 0 ? y : y >> sh; }

/* ------------------------------------------------------------- sin, exp2, freqlut --- */
/* Q24 a turn in, Q24 out. |dy| <= 102943 and lowbits < 2^14: the product stays under 2^31 */
static inline int32_t sin_lookup(uint32_t phase)
{
    const int shift = 24 - SIN_LG_N_SAMPLES;
    int32_t lowbits = (int32_t)(phase & ((1u << shift) - 1u));
    uint32_t ix = (phase >> (shift - 1)) & ((SIN_N_SAMPLES - 1u) << 1);
    int32_t dy = FM_SINTAB[ix], y0 = FM_SINTAB[ix + 1u];
    return y0 + ((dy * lowbits) >> shift);
}

static int32_t exp2_lookup(int32_t x)   /* Q24 in, Q24 out */
{
    const int shift = 24 - EXP2_LG_N_SAMPLES;
    int32_t lowbits = x & ((1 << shift) - 1);
    int32_t ix = (x >> (shift - 1)) & ((EXP2_N_SAMPLES - 1) << 1);
    int32_t y = FM_EXP2TAB[ix + 1] + mul_shr(FM_EXP2TAB[ix], lowbits, shift);
    return shr_31(y, 6 - (x >> 24));
}

static int32_t freq_lookup(int32_t logfreq)   /* Q24 octaves to a phase increment */
{
    int32_t ix = (logfreq & 0xffffff) >> FREQ_SAMPLE_SHIFT;
    int32_t y0 = FM_FREQLUT[ix], y1 = FM_FREQLUT[ix + 1];
    int32_t lowbits = logfreq & ((1 << FREQ_SAMPLE_SHIFT) - 1);
    int32_t y = y0 + mul_shr(y1 - y0, lowbits, FREQ_SAMPLE_SHIFT);
    return shr_31(y, MAX_LOGFREQ_INT - (logfreq >> 24));
}

/* -------------------------------------------------------------------- env --- */
static const uint8_t LEVELLUT[20] = {0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46};

static int scaleoutlevel(int outlevel) { return outlevel >= 20 ? 28 + outlevel : LEVELLUT[outlevel]; }

static void env_advance(fm_env_t *e, int newix)
{
    e->ix = (uint8_t)newix;
    if (newix < 4) {
        int actuallevel = scaleoutlevel(e->levels[newix]) >> 1, qrate;
        actuallevel = (actuallevel << 6) + e->outlevel - 4256;
        actuallevel = actuallevel < 16 ? 16 : actuallevel;
        e->targetlevel = actuallevel << 16;
        e->rising = e->targetlevel > e->level;
        qrate = (e->rates[newix] * 41) >> 6;
        qrate = min_i(qrate + e->rate_scaling, 63);
        e->inc = (4 + (qrate & 3)) << (2 + FM_LG_N + (qrate >> 2));
    }
}

static void env_init(fm_env_t *e, const int r[4], const int l[4], int32_t ol, int rate_scaling)
{
    int i;
    for (i = 0; i < 4; i++) {
        e->rates[i] = (uint8_t)r[i];
        e->levels[i] = (uint8_t)l[i];
    }
    e->outlevel = ol;
    e->rate_scaling = rate_scaling;
    e->level = 0;
    e->down = 1;
    env_advance(e, 0);
}

static int32_t env_getsample(fm_env_t *e)
{
    if (e->ix < 3 || (e->ix < 4 && !e->down)) {
        if (e->rising) {
            const int jumptarget = 1716;
            if (e->level < (jumptarget << 16))
                e->level = jumptarget << 16;
            e->level += (((17 << 24) - e->level) >> 24) * e->inc;
            if (e->level >= e->targetlevel) {
                e->level = e->targetlevel;
                env_advance(e, e->ix + 1);
            }
        } else {
            e->level -= e->inc;
            if (e->level <= e->targetlevel) {
                e->level = e->targetlevel;
                env_advance(e, e->ix + 1);
            }
        }
    }
    return e->level;
}

static void env_keydown(fm_env_t *e, int d)
{
    if (e->down != d) {
        e->down = (uint8_t)d;
        env_advance(e, d ? 0 : 3);
    }
}

/* --------------------------------------------------------------- pitchenv --- */
static const uint8_t RATETAB[100] = {
    1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 16, 16, 17, 18, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 31, 33, 34, 36, 37, 38, 39, 41, 42, 44, 46, 47, 49, 51, 53, 54, 56,
    58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 79, 82, 85, 88, 91, 94, 98, 102, 106, 110, 115, 120, 125, 130, 135, 141,
    147, 153, 159, 165, 171, 178, 185, 193, 202, 211, 232, 243, 254, 255};

static const int8_t PITCHTAB[100] = {
    -128, -116, -104, -95, -85, -76, -68, -61, -56, -52, -49, -46, -43, -41, -39, -37, -35, -33, -32, -31, -30, -29,
    -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6,
    -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 40, 43, 46, 49, 53, 58, 65, 73, 82, 92, 103, 115, 127};

static void pitchenv_advance(fm_pitchenv_t *e, int newix)
{
    e->ix = (uint8_t)newix;
    if (newix < 4) {
        e->targetlevel = PITCHTAB[e->levels[newix]] * (1 << 19);
        e->rising = e->targetlevel > e->level;
        e->inc = RATETAB[e->rates[newix]] * FM_PITCHENV_UNIT;
    }
}

static void pitchenv_set(fm_pitchenv_t *e, const int r[4], const int l[4])
{
    int i;
    for (i = 0; i < 4; i++) {
        e->rates[i] = (uint8_t)r[i];
        e->levels[i] = (uint8_t)l[i];
    }
    e->level = PITCHTAB[l[3]] * (1 << 19);
    e->down = 1;
    pitchenv_advance(e, 0);
}

static int32_t pitchenv_getsample(fm_pitchenv_t *e)
{
    if (e->ix < 3 || (e->ix < 4 && !e->down)) {
        if (e->rising) {
            e->level += e->inc;
            if (e->level >= e->targetlevel) {
                e->level = e->targetlevel;
                pitchenv_advance(e, e->ix + 1);
            }
        } else {
            e->level -= e->inc;
            if (e->level <= e->targetlevel) {
                e->level = e->targetlevel;
                pitchenv_advance(e, e->ix + 1);
            }
        }
    }
    return e->level;
}

static void pitchenv_keydown(fm_pitchenv_t *e, int d)
{
    if (e->down != d) {
        e->down = (uint8_t)d;
        pitchenv_advance(e, d ? 0 : 3);
    }
}

/* -------------------------------------------------------------------- lfo --- */
void fm_lfo_reset(fm_lfo_t *l, const uint8_t params[6])
{
    int rate = params[0];                           /* 0..99 */
    int sr = rate == 0 ? 1 : (165 * rate) >> 6, a;
    sr *= sr < 160 ? 11 : (11 + ((sr - 160) >> 4));
    l->delta = FM_LFO_UNIT * (uint32_t)sr;
    a = 99 - params[1];                             /* the delay */
    if (a == 99) {
        l->delayinc = ~0u;
        l->delayinc2 = ~0u;
    } else {
        a = (16 + (a & 15)) << (1 + (a >> 4));
        l->delayinc = FM_LFO_UNIT * (uint32_t)a;
        a &= 0xff80;
        a = max_i(0x80, a);
        l->delayinc2 = FM_LFO_UNIT * (uint32_t)a;
    }
    l->waveform = params[5];
    l->sync = params[4] != 0;
}

int32_t fm_lfo_sample(fm_lfo_t *l)
{
    int32_t x;
    l->phase += l->delta;
    switch (l->waveform) {
    case 0:                                         /* triangle */
        x = (int32_t)(l->phase >> 7);
        x ^= -(int32_t)(l->phase >> 31);
        x &= (1 << 24) - 1;
        return x;
    case 1:                                         /* sawtooth down */
        return (int32_t)((~l->phase ^ (1u << 31)) >> 8);
    case 2:                                         /* sawtooth up */
        return (int32_t)((l->phase ^ (1u << 31)) >> 8);
    case 3:                                         /* square */
        return (int32_t)(((~l->phase) >> 7) & (1u << 24));
    case 4:                                         /* sine */
        return (1 << 23) + (sin_lookup(l->phase >> 8) >> 1);
    case 5:                                         /* sample and hold */
        if (l->phase < l->delta)
            l->randstate = (uint8_t)((l->randstate * 179 + 17) & 0xff);
        x = l->randstate ^ 0x80;
        return (x + 1) << 16;
    default:
        return 1 << 23;
    }
}

int32_t fm_lfo_delay(fm_lfo_t *l)
{
    uint32_t delta = l->delaystate < (1u << 31) ? l->delayinc : l->delayinc2;
    uint32_t d = l->delaystate + delta;
    if (d < l->delayinc)
        return 1 << 24;
    l->delaystate = d;
    return d < (1u << 31) ? 0 : (int32_t)((d >> 7) & ((1u << 24) - 1u));
}

void fm_lfo_keydown(fm_lfo_t *l)
{
    if (l->sync)
        l->phase = (1u << 31) - 1u;
    l->delaystate = 0;
}

/* ------------------------------------------------------------- op kernels --- */
/* gain1 -> gain2 over the block (gain for sample i: gain1 + (1 + i) / N of the way). A sample's gain is applied in
 * 32 bits: the sine (|y| <= 2^24) to 15 bits and the gain (under 2^26) to 16, each block's shift s chosen so the
 * gain fits: |(y >> 9) * (gain >> s)| < 2^15 * 2^16 = 2^31, then >> (15 - s) for msfa's >> 24 */
static int gain_shift(int32_t gain1, int32_t gain2)
{
    int32_t g = (gain1 > gain2 ? gain1 : gain2) + (FM_N >> 1);   /* the ramp's rounding may pass gain2 by that */
    int s = 0;
    while ((g >> s) >= (1 << 16))
        s++;
    return s;
}

static void op_compute(int32_t *output, const int32_t *input, uint32_t phase, int32_t freq, int32_t gain1,
                       int32_t gain2, int add)
{
    int32_t dgain = (gain2 - gain1 + (FM_N >> 1)) >> FM_LG_N, gain = gain1;
    int s = gain_shift(gain1, gain2), i;
    for (i = 0; i < FM_N; i++) {
        int32_t y;
        gain += dgain;
        y = (((sin_lookup(phase + (uint32_t)input[i]) >> 9) * (gain >> s)) >> (15 - s));
        output[i] = add ? output[i] + y : y;
        phase += (uint32_t)freq;
    }
}

static void op_compute_pure(int32_t *output, uint32_t phase, int32_t freq, int32_t gain1, int32_t gain2, int add)
{
    int32_t dgain = (gain2 - gain1 + (FM_N >> 1)) >> FM_LG_N, gain = gain1;
    int s = gain_shift(gain1, gain2), i;
    for (i = 0; i < FM_N; i++) {
        int32_t y;
        gain += dgain;
        y = (((sin_lookup(phase) >> 9) * (gain >> s)) >> (15 - s));
        output[i] = add ? output[i] + y : y;
        phase += (uint32_t)freq;
    }
}

static void op_compute_fb(int32_t *output, uint32_t phase, int32_t freq, int32_t gain1, int32_t gain2,
                          int32_t *fb_buf, int fb_shift, int add)
{
    int32_t dgain = (gain2 - gain1 + (FM_N >> 1)) >> FM_LG_N, gain = gain1;
    int32_t y0 = fb_buf[0], y = fb_buf[1];
    int s = gain_shift(gain1, gain2), i;
    for (i = 0; i < FM_N; i++) {
        int32_t scaled_fb = (y0 + y) >> (fb_shift + 1);
        gain += dgain;
        y0 = y;
        y = (((sin_lookup(phase + (uint32_t)scaled_fb) >> 9) * (gain >> s)) >> (15 - s));
        output[i] = add ? output[i] + y : y;
        phase += (uint32_t)freq;
    }
    fb_buf[0] = y0;
    fb_buf[1] = y;
}

/* ---------------------------------------------------------------- fm_core --- */
enum {
    OUT_BUS_ONE = 1 << 0, OUT_BUS_TWO = 1 << 1, OUT_BUS_ADD = 1 << 2, IN_BUS_ONE = 1 << 4, IN_BUS_TWO = 1 << 5,
    FB_IN = 1 << 6, FB_OUT = 1 << 7
};

static const uint8_t ALGORITHMS[32][6] = {
    {0xc1, 0x11, 0x11, 0x14, 0x01, 0x14}, {0x01, 0x11, 0x11, 0x14, 0xc1, 0x14},   /* 1, 2 */
    {0xc1, 0x11, 0x14, 0x01, 0x11, 0x14}, {0x41, 0x11, 0x94, 0x01, 0x11, 0x14},   /* 3, 4 */
    {0xc1, 0x14, 0x01, 0x14, 0x01, 0x14}, {0x41, 0x94, 0x01, 0x14, 0x01, 0x14},   /* 5, 6 */
    {0xc1, 0x11, 0x05, 0x14, 0x01, 0x14}, {0x01, 0x11, 0xc5, 0x14, 0x01, 0x14},   /* 7, 8 */
    {0x01, 0x11, 0x05, 0x14, 0xc1, 0x14}, {0x01, 0x05, 0x14, 0xc1, 0x11, 0x14},   /* 9, 10 */
    {0xc1, 0x05, 0x14, 0x01, 0x11, 0x14}, {0x01, 0x05, 0x05, 0x14, 0xc1, 0x14},   /* 11, 12 */
    {0xc1, 0x05, 0x05, 0x14, 0x01, 0x14}, {0xc1, 0x05, 0x11, 0x14, 0x01, 0x14},   /* 13, 14 */
    {0x01, 0x05, 0x11, 0x14, 0xc1, 0x14}, {0xc1, 0x11, 0x02, 0x25, 0x05, 0x14},   /* 15, 16 */
    {0x01, 0x11, 0x02, 0x25, 0xc5, 0x14}, {0x01, 0x11, 0x11, 0xc5, 0x05, 0x14},   /* 17, 18 */
    {0xc1, 0x14, 0x14, 0x01, 0x11, 0x14}, {0x01, 0x05, 0x14, 0xc1, 0x14, 0x14},   /* 19, 20 */
    {0x01, 0x14, 0x14, 0xc1, 0x14, 0x14}, {0xc1, 0x14, 0x14, 0x14, 0x01, 0x14},   /* 21, 22 */
    {0xc1, 0x14, 0x14, 0x01, 0x14, 0x04}, {0xc1, 0x14, 0x14, 0x14, 0x04, 0x04},   /* 23, 24 */
    {0xc1, 0x14, 0x14, 0x04, 0x04, 0x04}, {0xc1, 0x05, 0x14, 0x01, 0x14, 0x04},   /* 25, 26 */
    {0x01, 0x05, 0x14, 0xc1, 0x14, 0x04}, {0x04, 0xc1, 0x11, 0x14, 0x01, 0x14},   /* 27, 28 */
    {0xc1, 0x14, 0x01, 0x14, 0x04, 0x04}, {0x04, 0xc1, 0x11, 0x14, 0x04, 0x04},   /* 29, 30 */
    {0xc1, 0x14, 0x04, 0x04, 0x04, 0x04}, {0xc4, 0x04, 0x04, 0x04, 0x04, 0x04},   /* 31, 32 */
};

int fm_carrier(int algorithm, int op) { return (ALGORITHMS[algorithm & 31][op % 6] & 3) == 0; }

static int32_t bus[2][FM_N];                        /* FmCore's buses: one voice at a time uses them */

static void core_compute(int32_t *output, fm_op_t *params, int algorithm, int32_t *fb_buf, int feedback_shift)
{
    const int32_t level_thresh = 1120;
    const uint8_t *alg = ALGORITHMS[algorithm & 31];
    int has_contents[3] = {1, 0, 0}, op;
    for (op = 0; op < 6; op++) {
        int flags = alg[op], add = (flags & OUT_BUS_ADD) != 0, inbus = (flags >> 4) & 3, outbus = flags & 3;
        fm_op_t *p = &params[op];
        int32_t *outptr = outbus == 0 ? output : bus[outbus - 1];
        if (p->gain[0] >= level_thresh || p->gain[1] >= level_thresh) {
            if (!has_contents[outbus])
                add = 0;
            if (inbus == 0 || !has_contents[inbus]) {
                if ((flags & 0xc0) == 0xc0 && feedback_shift < 16)   /* (one op in a feedback loop) */
                    op_compute_fb(outptr, p->phase, p->freq, p->gain[0], p->gain[1], fb_buf, feedback_shift, add);
                else
                    op_compute_pure(outptr, p->phase, p->freq, p->gain[0], p->gain[1], add);
            } else {
                op_compute(outptr, bus[inbus - 1], p->phase, p->freq, p->gain[0], p->gain[1], add);
            }
            has_contents[outbus] = 1;
        } else if (!add) {
            has_contents[outbus] = 0;
        }
        p->phase += (uint32_t)p->freq << FM_LG_N;
    }
}

/* ---------------------------------------------------------------- dx7note --- */
static int32_t midinote_to_logfreq(int midinote)
{
    const int32_t base = 50857777;                  /* (1 << 24) * (log(440) / log(2) - 69/12) */
    const int32_t step = (1 << 24) / 12;
    return base + step * midinote;
}

static const int32_t COARSEMUL[32] = {
    -16777216, 0, 16777216, 26591258, 33554432, 38955489, 43368474, 47099600, 50331648, 53182516, 55732705,
    58039632, 60145690, 62083076, 63876816, 65546747, 67108864, 68576247, 69959732, 71268397, 72509921, 73690858,
    74816848, 75892776, 76922906, 77910978, 78860292, 79773775, 80654032, 81503396, 82323963, 83117622};

static int32_t osc_freq(int midinote, int mode, int coarse, int fine, int detune)
{
    int32_t logfreq;
    if (mode == 0) {
        logfreq = midinote_to_logfreq(midinote);
        logfreq += COARSEMUL[coarse & 31];
        logfreq += FM_FINE[fine < 0 ? 0 : fine > 99 ? 99 : fine];   /* (1 << 24) / log(2) * log(1 + fine / 100) */
        logfreq += 12606 * (detune - 7);            /* measured at 7.213 Hz a count at 9600 Hz (msfa) */
    } else {
        logfreq = (4458616 * ((coarse & 3) * 100 + fine)) >> 3;   /* ((1 << 24) * log(10) / log(2) * .01) << 3 */
        logfreq += detune > 7 ? 13457 * (detune - 7) : 0;
    }
    return logfreq;
}

static const uint8_t VELOCITY_DATA[64] = {
    0, 70, 86, 97, 106, 114, 121, 126, 132, 138, 142, 148, 152, 156, 160, 163, 166, 170, 173, 174, 178, 181, 184,
    186, 189, 190, 194, 196, 198, 200, 202, 205, 206, 209, 211, 214, 216, 218, 220, 222, 224, 225, 227, 229, 230,
    232, 233, 235, 237, 238, 240, 241, 242, 243, 244, 246, 246, 248, 249, 250, 251, 252, 253, 254};

static int scale_velocity(int velocity, int sensitivity)   /* the velocity's change in microsteps */
{
    int clamped_vel = max_i(0, min_i(127, velocity));
    int vel_value = VELOCITY_DATA[clamped_vel >> 1] - 239;
    return ((sensitivity * vel_value + 7) >> 3) << 4;
}

static int scale_rate(int midinote, int sensitivity)
{
    int x = min_i(31, max_i(0, midinote / 3 - 7));
    return (sensitivity * x) >> 3;
}

static const uint8_t EXP_SCALE_DATA[33] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 14, 16, 19, 23, 27, 33, 39, 47, 56, 66, 80, 94, 110, 126, 142, 158, 174, 190,
    206, 222, 238, 250};

static int scale_curve(int group, int depth, int curve)
{
    int scale;
    if (curve == 0 || curve == 3) {                 /* linear */
        scale = (group * depth * 329) >> 12;
    } else {                                        /* exponential */
        int raw_exp = EXP_SCALE_DATA[min_i(group, (int)sizeof EXP_SCALE_DATA - 1)];
        scale = (raw_exp * depth * 329) >> 15;
    }
    return curve < 2 ? -scale : scale;
}

static int scale_level(int midinote, int break_pt, int left_depth, int right_depth, int left_curve,
                       int right_curve)
{
    int offset = midinote - break_pt - 17;
    if (offset >= 0)
        return scale_curve(offset / 3, right_depth, right_curve);
    return scale_curve((-offset) / 3, left_depth, left_curve);
}

static const uint8_t PITCHMODSENSTAB[8] = {0, 10, 20, 33, 55, 92, 153, 255};

void fm_note_init(fm_note_t *n, const uint8_t patch[FM_PATCH], int midinote, int velocity)
{
    int rates[4], levels[4], op, i, feedback;
    for (op = 0; op < 6; op++) {
        int off = op * 21, outlevel;
        for (i = 0; i < 4; i++) {
            rates[i] = patch[off + i];
            levels[i] = patch[off + 4 + i];
        }
        outlevel = scaleoutlevel(patch[off + 16]);
        outlevel += scale_level(midinote, patch[off + 8], patch[off + 9], patch[off + 10], patch[off + 11],
                                patch[off + 12]);
        outlevel = min_i(127, outlevel);
        outlevel = outlevel << 5;
        outlevel += scale_velocity(velocity, patch[off + 15]);
        outlevel = max_i(0, outlevel);
        env_init(&n->env[op], rates, levels, outlevel, scale_rate(midinote, patch[off + 13]));
        n->basepitch[op] = osc_freq(midinote, patch[off + 17], patch[off + 18], patch[off + 19], patch[off + 20]);
        n->op[op].phase = 0;
        n->op[op].gain[1] = 0;
    }
    for (i = 0; i < 4; i++) {
        rates[i] = patch[126 + i];
        levels[i] = patch[130 + i];
    }
    pitchenv_set(&n->pitchenv, rates, levels);
    n->algorithm = (uint8_t)(patch[134] & 31);
    feedback = patch[135];
    n->fb_shift = feedback != 0 ? 8 - feedback : 16;
    n->pitchmoddepth = (patch[139] * 165) >> 6;
    n->pitchmodsens = PITCHMODSENSTAB[patch[143] & 7];
    n->fb_buf[0] = n->fb_buf[1] = 0;
    n->down = 1;
}

/* added: fm_note_init for a voice that still sounds (its key again, or the voice taken for another note). There
 * every operator would start at phase 0 and silence: a step in the wave as large as the operator, a click. Here an
 * operator still above RETRIG_KEEP goes on from its phase, gain and envelope level (the attack rises from there, or
 * falls to its first level); a quieter one starts afresh, its step under RETRIG_KEEP, and keeps the attack that
 * oscillator key sync gives a note. The feedback goes on (its samples are the quiet operator's when it starts). */
#define RETRIG_KEEP (1 << 18)                       /* 1/128 of an operator at full level (2^25): -42 dB */
void fm_note_retrigger(fm_note_t *n, const uint8_t patch[FM_PATCH], int midinote, int velocity)
{
    uint32_t phase[6];
    int32_t gain[6], level[6], fb0 = n->fb_buf[0], fb1 = n->fb_buf[1];
    int op;
    for (op = 0; op < 6; op++) {
        phase[op] = n->op[op].phase;
        gain[op] = n->op[op].gain[1];
        level[op] = n->env[op].level;
    }
    fm_note_init(n, patch, midinote, velocity);
    for (op = 0; op < 6; op++) {
        if (gain[op] < RETRIG_KEEP)
            continue;
        n->op[op].phase = phase[op];
        n->op[op].gain[1] = gain[op];
        n->env[op].level = level[op];
        env_advance(&n->env[op], 0);                /* rising or falling from that level */
    }
    n->fb_buf[0] = fb0;
    n->fb_buf[1] = fb1;
}

void fm_note_compute(fm_note_t *n, int32_t *buf, int32_t lfo_val, int32_t lfo_delay, int32_t pitch_add)
{
    int32_t pitchmod = pitchenv_getsample(&n->pitchenv) + pitch_add;
    uint32_t pmd = (uint32_t)n->pitchmoddepth * (uint32_t)lfo_delay;   /* Q32 */
    int32_t senslfo = n->pitchmodsens * (lfo_val - (1 << 23));
    int op;
    pitchmod += mul_shr(pmd, senslfo, 39);
    for (op = 0; op < 6; op++) {
        int32_t level = env_getsample(&n->env[op]);
        n->op[op].gain[0] = n->op[op].gain[1];
        n->op[op].gain[1] = exp2_lookup(level - (14 * (1 << 24)));
        n->op[op].freq = freq_lookup(n->basepitch[op] + pitchmod);
    }
    core_compute(buf, n->op, n->algorithm, n->fb_buf, n->fb_shift);
}

void fm_note_keyup(fm_note_t *n)
{
    int op;
    for (op = 0; op < 6; op++)
        env_keydown(&n->env[op], 0);
    pitchenv_keydown(&n->pitchenv, 0);
    n->down = 0;
}

void fm_note_algorithm(fm_note_t *n, int algorithm, int feedback)
{
    n->algorithm = (uint8_t)(algorithm & 31);
    n->fb_shift = feedback != 0 ? 8 - feedback : 16;
}

int fm_note_done(const fm_note_t *n)
{
    int op;
    if (n->down)
        return 0;
    for (op = 0; op < 6; op++)                      /* a carrier under core_compute's threshold is silent */
        if (fm_carrier(n->algorithm, op) && (n->op[op].gain[0] >= 1120 || n->op[op].gain[1] >= 1120))
            return 0;
    return 1;
}
