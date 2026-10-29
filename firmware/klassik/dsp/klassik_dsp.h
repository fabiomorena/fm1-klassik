/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's sound, as the app sees it (dsp/engines.c, compiled apart at -O2): six parts, each its own sound. The
 * main loop asks (kl_select, kl_key); the audio interrupt plays (kl_render, and kl_key_now for MIDI that arrives
 * there).
 * A sound is an engine, a preset and KL_NV values: eight parameters (EDIT 1, EDIT 2), then the envelope (ATK, DEC,
 * SUS, REL), ENV > FILTER and VOICE (POLY, MONO): the preset's own values, or the app's (an edited sound, a USER
 * sound). VA's, PD's and ORGAN's envelope, ENV > FILTER and VOICE are the app's to set (kl_env); FM6, DRUMS and ACID
 * have envelopes of their own and play as their preset has them. */
#pragma once
#include <stdint.h>

enum { KL_ANALOG, KL_PHASE, KL_WHEEL, KL_FM6, KL_DRUMS, KL_ACID, KL_NENG };
enum { KL_INT, KL_PCT, KL_ENUM, KL_MS };   /* how a parameter's value reads (KL_MS: a time, kl_env_ms) */
#define KL_NV 14                            /* a sound's values */
enum { KL_ATK = 8, KL_DEC, KL_SUS, KL_REL, KL_FENV, KL_MONO };   /* after the eight parameters: 0..127 each, ENV >
                                                                 * FILTER -64..63, VOICE 0 POLY, 1 MONO */
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
void kl_preset_values(int e, int p, int8_t v[KL_NV]);   /* the preset's values */
int kl_env(int e);                          /* 1: its envelope, ENV > FILTER and VOICE are the sound's to set (VA, PD,
                                             * ORGAN) */
int kl_pages(int e);                        /* its edit pages: EDIT 1, EDIT 2, and with kl_env ENV and MORE */
const char *kl_page_title(int e, int page);  /* "OSC", "FLT", "ENV", "MORE" */
void kl_param(int e, int k, const int8_t v[KL_NV], kl_param_t *d);   /* value k (0..KL_NV-1) of a sound with the
                                                                       * values v (a name may depend on another:
                                                                       * ANALOG's WAVE); label "": none to set (ORGAN's
                                                                       * ENV > FILTER) */
int kl_env_ms(int k, int v);                /* KL_ATK, KL_DEC, KL_REL at v as a time, ms: the attack's rise, the decay's
                                             * and release's fall to -60 dB */
void kl_shape(int e, const int8_t v[8], uint8_t out[8]);   /* each value as 0..255 of its range (the glyph) */
const char *kl_drum_name(int kit, int pc);  /* DRUMS: the instrument on a key in a kit (pc: its note's semitone,
                                             * C = 0): "BD"; G# is the kit's own */
int kl_drum_pages(int pc);                  /* the edit pages whose knobs change it: bit 0 EDIT 1, bit 1 EDIT 2 */

void kl_select(int part, int e, int p, const int8_t v[KL_NV]);   /* main loop: a part's sound to play, from the next
                                                                 * block (VOICE changed: what sounds is let go) */
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
int kl_env_now(int part, int *stage);       /* the part's newest voice: its envelope, Q15 (an engine of its own: -1),
                                             * and *stage 1 attack, 2 decay and sustain, 3 release, 0 none */
uint32_t kl_fm6_algo(int a, uint8_t mods[6], int *fb);   /* FM6's algorithm a (0..31): the carriers (bit i: OP i +
                                             * 1), each operator's modulators (mods[i], the same bits), the feedback
                                             * loop from OP (*fb >> 4) to OP (*fb & 15) */
void kl_fm6_levels(int p, const int8_t v[8], uint8_t ol[6]);   /* FM6's preset p played with the values v: each
                                             * operator's output level, 0..99 (OP1 first) */

/* each part's LFO (dsp/engines.c): a wave a block (32 samples), to one destination of the part's sound. PITCH up to
 * +-12 semitones at DEPTH 100 (DEPTH squared: 10 gives +-12 cents), FILTER and SHAPE +-64 of their 128 steps (ACID's
 * FILTER +-2 octaves), AMP down to silence at the wave's trough */
enum { KL_LFO_PITCH, KL_LFO_FILTER, KL_LFO_SHAPE, KL_LFO_AMP, KL_LFO_NDEST };
enum { KL_SIN, KL_TRI, KL_SAW, KL_SQR, KL_SH, KL_NWAVE };
uint32_t kl_lfo_dests(int e);               /* what engine e takes: bit KL_LFO_* (VA all four; PD PITCH, FILTER (its
                                             * DCW), AMP; ORGAN and FM6 PITCH, AMP; ACID PITCH, FILTER; DRUMS none) */
void kl_lfo(int part, uint32_t inc, int depth, int wave, int dest, int retrig, uint32_t fade);   /* main loop: a
                                             * block's step (Q32 of a turn), DEPTH 0..100 (0 off), KL_SIN.., KL_LFO_*,
                                             * retrig 1 = each note starts the wave, fade: blocks the depth takes to
                                             * come in after a note (0 none) */
void kl_lfo_phase(int part, uint32_t ph);   /* audio interrupt, before kl_render: the wave's place this block (Q32;
                                             * the loop's, for a wave in time with it) */
uint32_t kl_lfo_phase_now(int part);        /* where the wave is (the screen) */
int32_t kl_lfo_now(int part);               /* its value, Q15 (the host's checks) */
int kl_lfo_depth_now(int part);             /* its depth as it plays, FADE in: 0..100 (the host's checks) */

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
