/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's sound, as the app sees it (dsp/engines.c, compiled apart at -O2). The main loop asks (kl_select,
 * kl_key); the audio interrupt plays (kl_render, and kl_key_now for MIDI that arrives there). */
#pragma once
#include <stdint.h>

enum { KL_ANALOG, KL_PHASE, KL_WHEEL, KL_NENG };

const char *kl_engine_name(int e);          /* Felucca-Engines' name: "ANALOG", "PHASE", "WHEEL" */
int kl_presets(int e);
const char *kl_preset_name(int e, int p);
void kl_shape(int e, int p, uint8_t out[8]);   /* the preset's eight parameters, each 0..255 of its range */

void kl_select(int e, int p);               /* main loop: engine and preset, from the next block */
void kl_key(int on, int note, int vel);     /* main loop: a note on (vel 1..127) or off, queued */
void kl_key_now(int on, int note, int vel); /* the audio interrupt's own (MIDI in) */
void kl_silence(void);                      /* every voice off at the next block */
void kl_render(int32_t *mono, uint32_t n);  /* audio interrupt: adds n frames (a multiple of 32) to mono */
int kl_voices(void);                        /* voices sounding */
