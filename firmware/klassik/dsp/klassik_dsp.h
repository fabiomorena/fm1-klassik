/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's sound, as the app sees it (dsp/engines.c, compiled apart at -O2). The main loop asks (kl_select,
 * kl_key); the audio interrupt plays (kl_render, and kl_key_now for MIDI that arrives there).
 * A sound is an engine, a preset (its envelope, filter envelope and MONO) and eight parameters (EDIT 1, EDIT 2):
 * the preset's own values, or the app's (an edited sound, a USER sound). */
#pragma once
#include <stdint.h>

enum { KL_ANALOG, KL_PHASE, KL_WHEEL, KL_FM6, KL_DRUMS, KL_NENG };
enum { KL_INT, KL_PCT, KL_ENUM };           /* how a parameter's value reads */

typedef struct {
    const char *label;                      /* "CUT" */
    const char *unit;                       /* KL_INT: "ct", or 0 */
    const char *const *names;               /* KL_ENUM: one for each value from min */
    int16_t min, max;
    uint8_t fmt;                            /* KL_* */
} kl_param_t;

const char *kl_engine_name(int e);          /* the engine's own name: "ANALOG", "PHASE", "WHEEL", "FM6", "DRUMS" */
int kl_presets(int e);
const char *kl_preset_name(int e, int p);
void kl_preset_values(int e, int p, int8_t v[8]);   /* the preset's eight parameters */
const char *kl_page_title(int e, int page);  /* EDIT 1 (page 0) and EDIT 2 (page 1): "OSC", "FLT" */
void kl_param(int e, int k, const int8_t v[8], kl_param_t *d);   /* parameter k of a sound with the values v (its
                                                                   * name may depend on another: ANALOG's WAVE) */
void kl_shape(int e, const int8_t v[8], uint8_t out[8]);   /* each value as 0..255 of its range (the glyph) */
const char *kl_drum_name(int pc);           /* DRUMS: the instrument on a key (pc: its note's semitone, C = 0): "BD" */
int kl_drum_pages(int pc);                  /* the edit pages whose knobs change it: bit 0 EDIT 1, bit 1 EDIT 2 */

void kl_select(int e, int p, const int8_t v[8]);   /* main loop: the sound to play, from the next block */
void kl_key(int on, int note, int vel);     /* main loop: a note on (vel 1..127) or off, queued */
void kl_key_now(int on, int note, int vel); /* the audio interrupt's own (MIDI in) */
void kl_silence(void);                      /* every voice off at the next block */
void kl_render(int32_t *mono, uint32_t n);  /* audio interrupt: adds n frames (a multiple of 32) to mono */
int kl_voices(void);                        /* voices sounding */
uint32_t kl_hits(int *note);                /* notes started so far (keys and MIDI), and the last one's note */
int kl_value_now(int k);                    /* parameter k as the render plays it (the host's checks) */
