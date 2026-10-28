/* SPDX-License-Identifier: MIT
 * Copyright 2014 Emilie Gillet (Mutable Instruments Rings: rings/dsp/fx/reverb.h, rings/dsp/fx/fx_engine.h; stmlib:
 *   dsp/cosine_oscillator.h)
 * Changed 2026 by fabiomorena: ported from C++ to C, see hall.c.
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
/* Rings' reverb (https://github.com/pichenettes/eurorack, rings/dsp/fx/reverb.h) for KLASSIK's HALL (dsp/fx.c): the
 * Griesinger topology of Dattorro's paper, four allpasses diffusing the input (left + right), then a loop of two
 * allpasses, a low pass and a delay moved by a slow LFO, twice; the two loops give left and right. Floats inside,
 * the delay lines stored in 16 bits (FxEngine<32768, FORMAT_16_BIT>: 65 536 bytes the caller gives). */
#pragma once
#include <stdint.h>

#define HL_SIZE 32768                   /* the delay lines' buffer, in samples (a power of two) */

typedef struct { float y1, y0, iir, init; } hl_cosine_t;   /* stmlib's CosineOscillator */

typedef struct {
    uint16_t *buffer;
    int32_t write_ptr;
    hl_cosine_t lfo[2];
    float lfo_value[2];
    float amount, input_gain, reverb_time, diffusion, lp;   /* Rings' set_amount .. set_lp */
    float lp_decay_1, lp_decay_2;
} hl_reverb_t;

void hl_init(hl_reverb_t *r, uint16_t *buffer, float fs);   /* buffer: HL_SIZE samples; fs: the rate (Hz) */
void hl_clear(hl_reverb_t *r);          /* every line silent */
void hl_process(hl_reverb_t *r, float *left, float *right, uint32_t size);   /* in place: each output moves from
                                                                             * its input toward the reverb by amount */
