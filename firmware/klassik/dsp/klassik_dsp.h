/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's sound, as the app sees it (dsp/engines.c, compiled apart at -O2): six parts, each its own sound. The
 * main loop asks (kl_select, kl_key); the audio interrupt plays (kl_render, and kl_key_now for MIDI that arrives
 * there).
 * A sound is an engine, a preset (its envelope, filter envelope and MONO) and eight parameters (EDIT 1, EDIT 2):
 * the preset's own values, or the app's (an edited sound, a USER sound). */
#pragma once
#include <stdint.h>

enum { KL_ANALOG, KL_PHASE, KL_WHEEL, KL_FM6, KL_DRUMS, KL_ACID, KL_NENG };
enum { KL_INT, KL_PCT, KL_ENUM };           /* how a parameter's value reads */
#define KL_NPART 6                          /* parts, each its own sound (MIDI channels 1..6) */
#define KL_BUDGET 240                       /* what all parts may sound at once, in tenths of an FM6 voice (engines.c
                                             * COST; an estimate until measured on an FM-1) */

typedef struct {
    const char *label;                      /* "CUT" */
    const char *unit;                       /* KL_INT: "ct", or 0 */
    const char *const *names;               /* KL_ENUM: one for each value from min */
    int16_t min, max;
    uint8_t fmt;                            /* KL_* */
} kl_param_t;

const char *kl_engine_name(int e);          /* the engine's own name: "ANALOG", "PHASE", "WHEEL", "FM6", "DRUMS",
                                             * "ACID" */
int kl_presets(int e);
const char *kl_preset_name(int e, int p);
void kl_preset_values(int e, int p, int8_t v[8]);   /* the preset's eight parameters */
const char *kl_page_title(int e, int page);  /* EDIT 1 (page 0) and EDIT 2 (page 1): "OSC", "FLT" */
void kl_param(int e, int k, const int8_t v[8], kl_param_t *d);   /* parameter k of a sound with the values v (its
                                                                   * name may depend on another: ANALOG's WAVE) */
void kl_shape(int e, const int8_t v[8], uint8_t out[8]);   /* each value as 0..255 of its range (the glyph) */
const char *kl_drum_name(int kit, int pc);  /* DRUMS: the instrument on a key in a kit (pc: its note's semitone,
                                             * C = 0): "BD"; G# is the kit's own */
int kl_drum_pages(int pc);                  /* the edit pages whose knobs change it: bit 0 EDIT 1, bit 1 EDIT 2 */

void kl_select(int part, int e, int p, const int8_t v[8]);   /* main loop: a part's sound to play, from the next block */
void kl_key(int part, int on, int note, int vel);   /* main loop: a note on (vel 1..127; ACID: 100 or more accents)
                                                     * or off, queued */
void kl_key_now(int part, int on, int note, int vel);   /* the audio interrupt's own (MIDI in) */
void kl_silence(void);                      /* every voice of every part off at the next block */
void kl_render(int32_t *const out[KL_NPART], uint32_t n);   /* audio interrupt: adds n frames (a multiple of 32) of
                                                             * each part to its out */
int kl_voices(int part);                    /* voices sounding: of a part, -1 all */
int kl_cost_now(void);                      /* what they cost together (tenths of an FM6 voice; KL_BUDGET at most) */
uint32_t kl_hits(int part, int *note);      /* a part's notes started so far (keys and MIDI), and the last one's note */
int kl_value_now(int part, int k);          /* parameter k as the render plays it (the host's checks) */
int kl_note_now(int part);                  /* a part's first voice's note (a MONO sound's), -1 silent (the host's
                                             * checks) */

/* the send effects (dsp/fx.c, compiled apart at -O2): DELAY, a ping-pong delay (the first echo left) of up to 0.5 s a
 * side, and HALL, Rings' reverb (dsp/hall/, MIT) behind PRE, a delay of up to 100 ms. Each takes the parts' sends
 * mixed mono and gives stereo (L, R interleaved), in kl_render's scale (a part's clamp: 2^19). The app gives the
 * buffers (its pool). */
#define FX_DLY_N 22050                      /* the delay's samples a side: 0.5 s at 44.1 kHz */
#define FX_PRE_N 4410                       /* PRE's: 100 ms */
#define FX_HALL_N 32768                     /* the hall's, 16 bits each (Rings' FxEngine<32768, FORMAT_16_BIT>) */
void fx_init(int16_t *dly, int16_t *pre, uint16_t *hall);   /* dly: 2 * FX_DLY_N (L, then R); all cleared */
void fx_delay_set(uint32_t d, int fdbk, int tone);   /* main loop: the time in samples (1 .. FX_DLY_N; a new one fades
                                                     * in), FDBK and TONE 0..100 */
void fx_hall_set(uint32_t pre, int decay, int tone); /* main loop: PRE in samples (0 .. FX_PRE_N), DECAY and TONE
                                                     * 0..100 */
void fx_render(const int32_t *dly_in, const int32_t *hall_in, int32_t *dly_out, int32_t *hall_out, uint32_t n,
               int dly_on, int hall_on);    /* audio interrupt: n frames (a multiple of 32); an effect no part sends
                                             * to (on 0) sleeps once its echoes have died away */
int fx_awake(int which);                    /* 0 DELAY, 1 HALL: it runs (the host's checks) */
