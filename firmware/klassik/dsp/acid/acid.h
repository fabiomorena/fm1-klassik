/* SPDX-License-Identifier: MIT
 * Copyright (c) 2009 Robin Schmidt (www.rs-met.com) (Open303: rosic_Open303.h, rosic_TeeBeeFilter.h,
 *   rosic_BlendOscillator.h, rosic_AnalogEnvelope.h, rosic_DecayEnvelope.h, rosic_LeakyIntegrator.h,
 *   rosic_OnePoleFilter.h, rosic_BiquadFilter.h, rosic_EllipticQuarterBandFilter.h)
 * Changed 2026 by fabiomorena: ported from C++ to C, see acid.c.
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
/* Open303 (https://github.com/RobinSchmidt/Open303, Source/DSPCode/) for KLASSIK's ACID (dsp/eng_acid.c): Robin
 * Schmidt's monophonic emulation of the TB-303, one voice, in float (the AC79 has a single-precision FPU; no double,
 * no libm). The setters take Open303's units (Hz, percent, milliseconds); the note functions are Open303's
 * triggerNote, slideToNote and releaseNote, the keys' bookkeeping (which is held, which came last) the caller's.
 * Its tables and fixed coefficients come from tools/gen_acid_tables.py (acid_tables.h). */
#pragma once
#include <stdint.h>

#ifndef ACID_FS
#define ACID_FS 44100                   /* the rate the voice runs at (the tables are made for it) */
#endif

enum { ACID_TB303, ACID_LP18 };          /* the filter: TeeBeeFilter's TB_303 (Open303's own) or LP_18 (its VST's) */

typedef struct {
    /* the parameters, as set */
    float cutoff, env_mod, normal_decay, blend, accent, tuning;
    float reso_skewed;                  /* TeeBeeFilter's resonanceSkewed */
    float env_scaler, env_offset;       /* calculateEnvModScalerAndOffset */
    float slew_d;                       /* the pitch slew limiter's 1 - coefficient (the slide) */
    int mode;
    /* the voice */
    int idle;                           /* nothing played yet: the next trigger starts from a clean state */
    float osc_freq, slew_e;             /* the note's frequency, and the slew limiter's distance from it (Hz) */
    float freq;                         /* the oscillator's (BlendOscillator::setFrequency keeps the last good one) */
    uint32_t phase;                     /* the oscillator's phase, the table index in Q21 */
    float env_y, env_d, env_y0;         /* the main envelope (DecayEnvelope): y, 1 - its coefficient, y at a trigger */
    float rc2;                          /* the accent's RC (rc1 is a wire: its time constant is 0) */
    float accent_gain;
    float amp, amp_release;             /* the amplitude envelope (AnalogEnvelope): its output, release coefficient */
    uint8_t amp_stage, note_on;         /* 0 the attack's one sample, 1 decay, 2 release; the note held */
    float fb_x1, fb_y1;                 /* the filter's feedback high-pass */
    float y1, y2, y3, y4;               /* the filter's four stages */
    float hp1_x1, hp1_y1, hp2_x1, hp2_y1, ap_x1, ap_y1;
    float notch[4], declick[4];         /* x1 x2 y1 y2 */
    float ell[6][2];
} acid_t;

void acid_init(acid_t *a);              /* Open303's constructor (sample rate ACID_FS, volume 0 dB) */
void acid_reset(acid_t *a);             /* as silent as at the start: the next trigger resets the voice */
void acid_set_cutoff(acid_t *a, float hz);
void acid_set_resonance(acid_t *a, float pct);
void acid_set_env_mod(acid_t *a, float pct);
void acid_set_decay(acid_t *a, float ms);       /* from the next note */
void acid_set_accent(acid_t *a, float pct);     /* from the next note */
void acid_set_waveform(acid_t *a, float w);     /* 0 saw .. 1 square */
void acid_set_slide_time(acid_t *a, float ms);
void acid_set_mode(acid_t *a, int mode);        /* ACID_TB303 (the default) or ACID_LP18 */

void acid_trigger(acid_t *a, float pitch, int accent);  /* a note from silence or after a release (pitch: MIDI) */
void acid_slide(acid_t *a, float pitch, int accent);    /* a note while another is held: the pitch slides */
void acid_glide(acid_t *a, float pitch);                /* back to a key still held after the latest is let go */
void acid_release(acid_t *a);                           /* the last key let go */
void acid_render(acid_t *a, float *out, int n);         /* n samples, written (not added) */

float acid_exp2(float x);                               /* 2^x without libm (|x| < 126) */
