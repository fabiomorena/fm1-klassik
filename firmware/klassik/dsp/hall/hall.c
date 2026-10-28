/* SPDX-License-Identifier: MIT
 * Copyright 2014 Emilie Gillet (Mutable Instruments Rings: rings/dsp/fx/reverb.h, rings/dsp/fx/fx_engine.h; stmlib:
 *   dsp/cosine_oscillator.h)
 * Changed 2026 by fabiomorena: ported from C++ to C, in one file; for the FM-1:
 *   - classes as structs and functions; FxEngine's templates (Reserve, DelayLine, Context) written out: each delay
 *     line's base and length as constants, the context's accumulator as a local
 *   - the LFOs' frequencies given at the rate it runs at (Rings': 48 kHz, so 0.5 / 48000 there): they keep their
 *     0.5 Hz and 0.3 Hz; the delay lines keep their lengths in samples (at 44.1 kHz the rooms are 8.8 % longer)
 *   - only FORMAT_16_BIT and the CosineOscillator's approximate mode (the two Rings' reverb uses)
 *   - Init zeroes every member (Rings' leaves amount, input gain, time and the low passes' states to the setters);
 *     hl_clear zeroes the low passes' states too
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
#include "hall.h"

void *memset(void *d, int c, unsigned n);       /* the FM-1 has no libc: base/bsp/chip/libc.c has this one */

#define MASK (HL_SIZE - 1)

/* the delay lines, Reserve<150, Reserve<214, ... Reserve<6312> > >: each starts a sample after the one before ends */
enum {
    AP1_L = 150, AP2_L = 214, AP3_L = 319, AP4_L = 527, DAP1A_L = 2182, DAP1B_L = 2690, DEL1_L = 4501,
    DAP2A_L = 2525, DAP2B_L = 2197, DEL2_L = 6312,
    AP1 = 0,
    AP2 = AP1 + AP1_L + 1,
    AP3 = AP2 + AP2_L + 1,
    AP4 = AP3 + AP3_L + 1,
    DAP1A = AP4 + AP4_L + 1,
    DAP1B = DAP1A + DAP1A_L + 1,
    DEL1 = DAP1B + DAP1B_L + 1,
    DAP2A = DEL1 + DEL1_L + 1,
    DAP2B = DAP2A + DAP2A_L + 1,
    DEL2 = DAP2B + DAP2B_L + 1,
    HL_USED = DEL2 + DEL2_L + 1
};
_Static_assert(HL_USED <= HL_SIZE, "delay_memory_full");

/* ------------------------------------------------- stmlib's CosineOscillator --- */
static void cos_init(hl_cosine_t *o, float frequency)   /* InitApproximate, then Start */
{
    float sign = 16.0f;
    frequency -= 0.25f;
    if (frequency < 0.0f) {
        frequency = -frequency;
    } else {
        if (frequency > 0.5f)
            frequency -= 0.5f;
        else
            sign = -16.0f;
    }
    o->iir = sign * frequency * (1.0f - 2.0f * frequency);
    o->init = o->iir * 0.25f;
    o->y1 = o->init;
    o->y0 = 0.5f;
}

static float cos_value(const hl_cosine_t *o) { return o->y1 + 0.5f; }

static float cos_next(hl_cosine_t *o)
{
    float temp = o->y0;
    o->y0 = o->iir * o->y0 - o->y1;
    o->y1 = temp;
    return temp + 0.5f;
}

/* ------------------------------------------------------ FxEngine, 16 bits --- */
static float decompress(uint16_t v) { return (float)(int16_t)v / 32768.0f; }

static uint16_t compress(float v)
{
    int32_t i = (int32_t)(v * 32768.0f);
    return (uint16_t)(i > 32767 ? 32767 : i < -32768 ? -32768 : i);   /* stmlib's Clip16 */
}

/* a line's tail (Read(d TAIL, ..)): its oldest sample */
#define TAIL(base, len) buf[(wp + (base) + (len) - 1) & MASK]

static float interpolate(const uint16_t *buf, int32_t wp, int32_t base, float offset)   /* Interpolate(d, offset) */
{
    int32_t oi = (int32_t)offset;               /* MAKE_INTEGRAL_FRACTIONAL */
    float of = offset - (float)oi;
    float a = decompress(buf[(wp + oi + base) & MASK]);
    float b = decompress(buf[(wp + oi + base + 1) & MASK]);
    return a + (b - a) * of;
}

/* -------------------------------------------------------------- the reverb --- */
void hl_init(hl_reverb_t *r, uint16_t *buffer, float fs)
{
    memset(r, 0, sizeof *r);
    r->buffer = buffer;
    hl_clear(r);
    cos_init(&r->lfo[0], 0.5f / fs * 32.0f);    /* SetLFOFrequency: a step every 32 samples */
    cos_init(&r->lfo[1], 0.3f / fs * 32.0f);
    r->lp = 0.7f;
    r->diffusion = 0.625f;
}

void hl_clear(hl_reverb_t *r)
{
    memset(r->buffer, 0, HL_SIZE * sizeof r->buffer[0]);
    r->write_ptr = 0;
    r->lp_decay_1 = r->lp_decay_2 = 0.0f;
}

void hl_process(hl_reverb_t *r, float *left, float *right, uint32_t size)
{
    /* This is the Griesinger topology described in the Dattorro paper (4 AP diffusers on the input, then a loop of
     * 2x 2AP+1Delay). Modulation is applied in the loop of the first diffuser AP for additional smearing; and to the
     * two long delays for a slow shimmer/chorus effect. (Rings' comment; the first diffuser's modulation is
     * commented out there.) */
    uint16_t *buf = r->buffer;
    const float kap = r->diffusion, klp = r->lp, krt = r->reverb_time, amount = r->amount, gain = r->input_gain;
    float lp_1 = r->lp_decay_1, lp_2 = r->lp_decay_2;
    while (size--) {
        float acc, prev, wet, apout;
        int32_t wp;
        if (--r->write_ptr < 0)                 /* FxEngine::Start */
            r->write_ptr += HL_SIZE;
        wp = r->write_ptr;
        if ((wp & 31) == 0) {
            r->lfo_value[0] = cos_next(&r->lfo[0]);
            r->lfo_value[1] = cos_next(&r->lfo[1]);
        } else {
            r->lfo_value[0] = cos_value(&r->lfo[0]);
            r->lfo_value[1] = cos_value(&r->lfo[1]);
        }

        acc = (*left + *right) * gain;

        /* Diffuse through 4 allpasses. */
        prev = decompress(TAIL(AP1, AP1_L));
        acc += prev * kap;
        buf[(wp + AP1) & MASK] = compress(acc);
        acc = acc * -kap + prev;
        prev = decompress(TAIL(AP2, AP2_L));
        acc += prev * kap;
        buf[(wp + AP2) & MASK] = compress(acc);
        acc = acc * -kap + prev;
        prev = decompress(TAIL(AP3, AP3_L));
        acc += prev * kap;
        buf[(wp + AP3) & MASK] = compress(acc);
        acc = acc * -kap + prev;
        prev = decompress(TAIL(AP4, AP4_L));
        acc += prev * kap;
        buf[(wp + AP4) & MASK] = compress(acc);
        acc = acc * -kap + prev;
        apout = acc;

        /* Main reverb loop. */
        acc = apout + interpolate(buf, wp, DEL2, 6261.0f + 50.0f * r->lfo_value[1]) * krt;
        lp_1 += klp * (acc - lp_1);
        acc = lp_1;
        prev = decompress(TAIL(DAP1A, DAP1A_L));
        acc += prev * -kap;
        buf[(wp + DAP1A) & MASK] = compress(acc);
        acc = acc * kap + prev;
        prev = decompress(TAIL(DAP1B, DAP1B_L));
        acc += prev * kap;
        buf[(wp + DAP1B) & MASK] = compress(acc);
        acc = acc * -kap + prev;
        buf[(wp + DEL1) & MASK] = compress(acc);
        wet = acc * 2.0f;

        *left += (wet - *left) * amount;

        acc = apout + interpolate(buf, wp, DEL1, 4460.0f + 40.0f * r->lfo_value[0]) * krt;
        lp_2 += klp * (acc - lp_2);
        acc = lp_2;
        prev = decompress(TAIL(DAP2A, DAP2A_L));
        acc += prev * kap;
        buf[(wp + DAP2A) & MASK] = compress(acc);
        acc = acc * -kap + prev;
        prev = decompress(TAIL(DAP2B, DAP2B_L));
        acc += prev * -kap;
        buf[(wp + DAP2B) & MASK] = compress(acc);
        acc = acc * kap + prev;
        buf[(wp + DEL2) & MASK] = compress(acc);
        wet = acc * 2.0f;

        *right += (wet - *right) * amount;

        ++left;
        ++right;
    }
    r->lp_decay_1 = lp_1;
    r->lp_decay_2 = lp_2;
}
