/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2012, 2013 Google Inc. (msfa: dx7note.h, env.h, pitchenv.h, lfo.h, fm_core.h)
 * Changed 2026 by fabiomorena: ported from C++ to C (classes as structs and functions), see msfa.c. */
/* msfa, the FM core of Google's Music Synthesizer for Android (https://github.com/google/music-synthesizer-for-android,
 * app/src/main/jni/): a DX7 voice of six operators, its envelopes, pitch envelope and LFO, in fixed point. KLASSIK's
 * FM6 (dsp/eng_fm6.c) plays it. A patch is the DX7's voice unpacked (msfa's 156 bytes, OP6 first; msfa's
 * UnpackPatch makes it from a cartridge's 128). Not ported: the MIDI and voice handling (synth_unit), the filters,
 * the NEON kernels. */
#pragma once
#include <stdint.h>

#define FM_LG_N 5                       /* a block: 32 samples (msfa's LG_N is 6: 64), the engines' CTL */
#define FM_N (1 << FM_LG_N)
#define FM_PATCH 156

typedef struct {                        /* Env: a DX7 operator's envelope, Q24 / doubling log level */
    uint8_t rates[4], levels[4];        /* 0..99 */
    uint8_t ix, rising, down;
    int32_t outlevel, rate_scaling;
    int32_t level, targetlevel, inc;
} fm_env_t;

typedef struct {                        /* PitchEnv: Q24 / octave */
    uint8_t rates[4], levels[4];
    uint8_t ix, rising, down;
    int32_t level, targetlevel, inc;
} fm_pitchenv_t;

typedef struct {                        /* FmOpParams */
    int32_t gain[2];                    /* the last block's and this block's, Q24 */
    int32_t freq;                       /* the phase increment, Q24 a turn */
    uint32_t phase;
} fm_op_t;

typedef struct {                        /* Dx7Note: one voice */
    fm_env_t env[6];
    fm_op_t op[6];
    fm_pitchenv_t pitchenv;
    int32_t basepitch[6];
    int32_t fb_buf[2];
    int32_t fb_shift;
    int32_t pitchmoddepth, pitchmodsens;
    uint8_t algorithm;                  /* 0..31 */
    uint8_t down;                       /* the key held (added: fm_note_done) */
} fm_note_t;

typedef struct {                        /* Lfo: one for every voice of a part, as on a DX7 */
    uint32_t phase, delta;
    uint32_t delaystate, delayinc, delayinc2;
    uint8_t waveform, randstate, sync;
} fm_lfo_t;

/* a voice: started (velocity 1..127), a block of FM_N samples added to buf, its key let go */
void fm_note_init(fm_note_t *n, const uint8_t patch[FM_PATCH], int midinote, int velocity);
void fm_note_compute(fm_note_t *n, int32_t *buf, int32_t lfo_val, int32_t lfo_delay, int32_t pitch_add);   /* pitch_add:
                                         * Q24 an octave (added: the host's LFO) */
void fm_note_keyup(fm_note_t *n);
/* added: the algorithm (0..31) and feedback (0..7) of a sounding voice, as it plays on; 1 = its key is up and none
 * of its carriers sounds any more */
void fm_note_algorithm(fm_note_t *n, int algorithm, int feedback);
int fm_note_done(const fm_note_t *n);
/* added: fm_note_init for a voice still sounding, going on from where its operators are (no click) */
void fm_note_retrigger(fm_note_t *n, const uint8_t patch[FM_PATCH], int midinote, int velocity);
int fm_carrier(int algorithm, int op);  /* op 0..5 as the patch has them (0 = OP6): 1 = it is heard */

/* the LFO: its six parameters (the patch's bytes 137..142), a value 0..1 (Q24) and the delay's 0..1 a block */
void fm_lfo_reset(fm_lfo_t *l, const uint8_t params[6]);
int32_t fm_lfo_sample(fm_lfo_t *l);
int32_t fm_lfo_delay(fm_lfo_t *l);
void fm_lfo_keydown(fm_lfo_t *l);
