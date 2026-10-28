/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena
 * From fm1-base's template, Copyright (C) 2026 Charles Vestal (MPL-2.0) */
/* KLASSIK v0.6 (parts, mixer, looper, step sequencer, effects): the rack and its edit pages. Six engines, FM6 (msfa's six operators), three of
 * Felucca-Engines (ANALOG, PHASE, WHEEL), DRUMS (four kits on Plaits' drum models) and ACID (Open303's TB-303), on
 * six parts that play at once, each its own sound and octave: SEL, or SELECT turned, goes to the next part (SELECT
 * to the left: the one before), and the keys, K1 (the engine, in the rack's order), PRESET (its sound), OCT- / OCT+
 * (its octave) and EDIT are the chosen part's; MIDI
 * channel 1..6 plays part 1..6 (others the chosen part), the keys send on their part's channel. On ACID a key
 * pressed while ENV is held is accented (ACC in the header). EDIT opens the engine's two pages of four parameters
 * on K1..K4 (EDIT or OCT+ the other page, OCT- back to the rack); an edit plays at once (FM6's MOD, DTUN, ATK, DEC,
 * SUS and REL and ACID's DECAY and ACCENT from the next note). HOME held, a knob turned goes back to its sound's
 * value (the preset's, or the USER sound's), marked on its bar while it is elsewhere.
 * SAVE keeps the parts (the one chosen, each one's engine, sounds and octave) across a power cycle, an edited sound
 * as its engine's USER sound (every part's).
 * GLO opens the mixer: a strip a part, three a page (the chosen part's: 1-3 or 4-6; LEVEL, PAN, MUTE, SOLO on K1..K4
 * for the chosen part) and the master's meters; GLO or OCT+ there: its tab SEND, the chosen part's sends to the effects (K1 DELAY, K2 HALL). The output is
 * stereo, through a limiter.
 * FX opens the effects' page: DELAY (a ping-pong delay in time with the looper's TEMPO: TIME, FDBK, TONE, LEVEL),
 * FX or OCT+ HALL (Rings' reverb: PRE, DECAY, TONE, LEVEL). Both are send effects: the parts' sends feed them (after
 * LEVEL, MUTE and SOLO, before PAN), their returns go into the master before the limiter.
 * The looper records notes (not sound) on one loop for all six parts: PLAY starts and stops it, REC records the
 * chosen part on top of what it has (stopped: after a bar's count-in), the first take sets the length; HOME + REC
 * takes the last take back, HOME + PLAY empties a part. SEQ opens its page: a lane a part, TEMPO, LENGTH, QUANT and
 * CLICK on K1..K4; the header shows it on every page. SEQ or OCT+ there: the step sequencer (STEP), a bar of the same
 * notes as 16 steps a part, the white keys the chosen part's steps (DRUMS: one instrument's), K1 BAR, K2 NOTE (INST),
 * K3 VEL, K4 LEN, a key held and K2..K4 turned: that step's.
 * The screen: the header (the part and its engine, the sound's glyph and name, the octave, the battery), then the
 * six engine tiles (each with the parts that play it) or an edit page (its tabs, the sound's glyph large, its
 * spokes named; DRUMS: the octave's keys as pads, each with its instrument), and a row of what the knobs hold.
 * A sound's glyph: its eight parameters as eight spokes, each as long as its value in its range, in the engine's
 * colour. Part of the unity build (base/device/unity.c, base/host/plat_host.c), after the platform: gfx.c and
 * announce.c are in scope. */
#include "fastmath.h"
#include "dsp/klassik_dsp.h"

/* ---------------------------------------------------------------- colours --- */
#define K_GROUND RGB(11, 13, 17)
#define K_HEAD RGB(17, 21, 28)
#define K_SURF RGB(21, 25, 34)
#define K_SURF2 RGB(28, 34, 48)
#define K_LINE RGB(42, 50, 66)
#define K_TEXT RGB(232, 234, 237)
#define K_MUTED RGB(154, 163, 178)
#define K_GREEN RGB(126, 224, 138)
#define K_REC RGB(255, 92, 92)                       /* the looper recording */

/* the rack: six tiles, the engines that play now and those to come (eng -1) */
typedef struct {
    const char *name, *sub;
    int eng;
    uint16_t col;
} tile_t;
static const tile_t TILES[6] = {
    {"FM6", "6-OP FM", KL_FM6, RGB(92, 200, 255)},
    {"VA", "ANALOG", KL_ANALOG, RGB(255, 180, 84)},
    {"PD", "PHASE DIST", KL_PHASE, RGB(255, 122, 182)},
    {"ORGAN", "DRAWBAR", KL_WHEEL, RGB(126, 224, 138)},
    {"DRUMS", "DRUM KITS", KL_DRUMS, RGB(185, 162, 255)},
    {"ACID", "MONO BASS", KL_ACID, RGB(227, 244, 94)},
};
static const uint8_t TILE_OF[KL_NENG] = {1, 2, 3, 0, 4, 5};   /* engine -> its tile */

static int engine_pos(int e)                         /* its place among the playing tiles, as the rack shows them */
{
    int i, n = 0;
    for (i = 0; i < TILE_OF[e]; i++)
        n += TILES[i].eng >= 0;
    return n;
}

static int engine_at(int pos)                        /* the engine at that place */
{
    int i;
    for (i = 0; i < 6; i++)
        if (TILES[i].eng >= 0 && pos-- == 0)
            return TILES[i].eng;
    return KL_ANALOG;
}

/* -------------------------------------------------------------- settings --- */
#define SET_MAGIC 0x4B4C5339u                        /* "KLS9": six parts. The effects' KLS8 and the mixer's KLS7
                                                     * (four parts; KLS7 without sends) are taken over, parts 5 and 6
                                                     * as at first; another magic (the parts' KLS6, v0.3's KLS5,
                                                     * DRUMS' KLS4, v0.2's KLS3, v0.1's KLS2, v0's KLS1) loads
                                                     * defaults */
#define SET_MAGIC8 0x4B4C5338u
#define SET_MAGIC7 0x4B4C5337u
#define OLD_NPART 4                                  /* KLS7's and KLS8's parts */
typedef struct {
    int8_t v[8];                                     /* the eight parameters */
    uint8_t base;                                    /* the preset it was made from: its envelopes and MONO */
    uint8_t saved;                                   /* 1 = there is one */
} user_t;
typedef struct {
    uint8_t engine;                                  /* KL_* */
    uint8_t preset[KL_NENG];                         /* each engine's last sound on the part; kl_presets(e) is its
                                                     * USER sound */
    int8_t octave;                                   /* -2 .. +2 */
    int8_t level;                                    /* the mixer: dB, -40 .. +6, LEVEL_OFF below */
    int8_t pan;                                      /* -50 (left) .. 50 (right) */
    uint8_t mute, solo;
    uint8_t send[2];                                 /* to DELAY and HALL: 0..100 */
} part_set_t;
enum { FX_DELAY, FX_HALL };
enum { FK_TIME, FK_FDBK, FK_TONE, FK_LEVEL };        /* DELAY's knobs (HALL's: PRE, DECAY, TONE, LEVEL) */
typedef struct {
    uint32_t magic;
    uint8_t part;                                    /* the part chosen: the keys, K1, PRESET, OCT and EDIT are its */
    uint8_t rsv[3];
    part_set_t parts[KL_NPART];
    user_t user[KL_NENG];                            /* each engine's own sound (SAVE after an edit), every part's */
    int8_t fx[2][4];                                 /* the effects' knobs: DELAY TIME (DLY_K's index), FDBK %, TONE
                                                     * %, LEVEL (dB, -40 .. +6, LEVEL_OFF below); HALL PRE (ms),
                                                     * DECAY %, TONE %, LEVEL */
} settings_t;
_Static_assert(sizeof(settings_t) <= PLAT_STORE_MAX, "the settings are one store object");
typedef struct {                                     /* KLS8's: four parts */
    uint32_t magic;
    uint8_t part, rsv[3];
    part_set_t parts[OLD_NPART];
    user_t user[KL_NENG];
    int8_t fx[2][4];
} settings8_t;
typedef struct {                                     /* KLS7's, without the sends */
    uint8_t engine, preset[KL_NENG];
    int8_t octave, level, pan;
    uint8_t mute, solo;
} part_set7_t;
typedef struct {
    uint32_t magic;
    uint8_t part, rsv[3];
    part_set7_t parts[OLD_NPART];
    user_t user[KL_NENG];
} settings7_t;
_Static_assert(sizeof(settings7_t) != sizeof(settings_t) && sizeof(settings8_t) != sizeof(settings_t) &&
               sizeof(settings7_t) != sizeof(settings8_t), "KLS7 and KLS8 tell themselves by their sizes");
static settings_t set, set_kept;                     /* as it is, and as the store has it (or the defaults at boot) */
static uint8_t set_dirty;
static const uint8_t PART_ENGINE[KL_NPART] = {KL_ANALOG, KL_DRUMS, KL_ACID, KL_FM6, KL_PHASE, KL_WHEEL};   /* the
                                                     * parts at first: each engine once */
#define LEVEL_OFF (-41)                              /* the mixer's LEVEL below -40 dB: off */
static const int8_t FX_MIN[2][4] = {{0, 0, 0, LEVEL_OFF}, {0, 0, 0, LEVEL_OFF}};
static const int8_t FX_MAX[2][4] = {{8, 100, 100, 6}, {100, 100, 100, 6}};
static const int8_t FX_DEF[2][4] = {{7, 35, 70, 0}, {20, 50, 60, 0}};   /* 1/8., 35 %, 70 %, 0 dB; 20 ms, 50 %, 60 %,
                                                                         * 0 dB: where HOME takes them */

/* each part's sound of each engine as it plays: its preset's values or its USER sound's (orig), and whether EDIT has
 * any of them elsewhere (not saved) */
static int8_t work[KL_NPART][KL_NENG][8], orig[KL_NPART][KL_NENG][8];
static uint8_t edited[KL_NPART][KL_NENG];

static part_set_t *part_cur(void) { return &set.parts[set.part]; }
static int eng_cur(void) { return set.parts[set.part].engine; }   /* the chosen part's engine */

static void settings_defaults(void)
{
    uint32_t i;
    uint8_t *b = (uint8_t *)&set;
    for (i = 0; i < sizeof set; i++)                 /* no USER sounds, every engine at its first preset */
        b[i] = 0;
    set.magic = SET_MAGIC;
    for (i = 0; i < KL_NPART; i++)
        set.parts[i].engine = PART_ENGINE[i];
    for (i = 0; i < 8u; i++)
        set.fx[i / 4u][i % 4u] = FX_DEF[i / 4u][i % 4u];
}

static int values_ok(int e, const int8_t v[8])
{
    kl_param_t d;
    int k;
    for (k = 0; k < 8; k++) {
        kl_param(e, k, v, &d);
        if (v[k] < d.min || v[k] > d.max)
            return 0;
    }
    return 1;
}

static int settings_ok(void)
{
    uint32_t i, p;
    if (set.magic != SET_MAGIC || set.part >= KL_NPART)
        return 0;
    for (p = 0; p < KL_NPART; p++) {
        const part_set_t *ps = &set.parts[p];
        if (ps->engine >= KL_NENG || ps->octave < -2 || ps->octave > 2 || ps->level < LEVEL_OFF || ps->level > 6 ||
            ps->pan < -50 || ps->pan > 50 || ps->mute > 1 || ps->solo > 1 || ps->send[0] > 100 || ps->send[1] > 100)
            return 0;
        for (i = 0; i < KL_NENG; i++)
            if (ps->preset[i] >= kl_presets((int)i) + set.user[i].saved)
                return 0;
    }
    for (i = 0; i < 8u; i++)
        if (set.fx[i / 4u][i % 4u] < FX_MIN[i / 4u][i % 4u] || set.fx[i / 4u][i % 4u] > FX_MAX[i / 4u][i % 4u])
            return 0;
    for (i = 0; i < KL_NENG; i++) {
        const user_t *u = &set.user[i];
        if (u->saved > 1 || (u->saved && (u->base >= kl_presets((int)i) || !values_ok((int)i, u->v))))
            return 0;
    }
    return 1;
}

static void settings_load(void)                      /* the store's, KLS8's or KLS7's taken over, or the defaults */
{
    int n = plat_store_load(OBJ_SETTINGS, &set, sizeof set);
    int v8 = n == (int)sizeof(settings8_t) && set.magic == SET_MAGIC8;
    if (v8 || (n == (int)sizeof(settings7_t) && set.magic == SET_MAGIC7)) {   /* four parts: 5 and 6 as at first */
        union {
            settings8_t s8;                          /* v0.6's effects */
            settings7_t s7;                          /* v0.5's mixer, no sends */
        } old;
        uint32_t i, e;
        const uint8_t *src = (const uint8_t *)&set;
        uint8_t *d = (uint8_t *)&old;
        for (i = 0; i < (uint32_t)n; i++)
            d[i] = src[i];
        settings_defaults();
        set.part = v8 ? old.s8.part : old.s7.part;
        for (i = 0; i < OLD_NPART; i++) {
            part_set_t *ps = &set.parts[i];
            const part_set7_t *o = &old.s7.parts[i];
            if (v8) {
                *ps = old.s8.parts[i];
                continue;
            }
            ps->engine = o->engine;
            for (e = 0; e < KL_NENG; e++)
                ps->preset[e] = o->preset[e];
            ps->octave = o->octave;
            ps->level = o->level;
            ps->pan = o->pan;
            ps->mute = o->mute;
            ps->solo = o->solo;
        }
        for (e = 0; e < KL_NENG; e++)
            set.user[e] = v8 ? old.s8.user[e] : old.s7.user[e];
        for (i = 0; v8 && i < 8u; i++)
            set.fx[i / 4u][i % 4u] = old.s8.fx[i / 4u][i % 4u];
        n = (int)sizeof set;
    }
    if (n != (int)sizeof set || !settings_ok())
        settings_defaults();
}

static int is_user(int p, int e) { return set.parts[p].preset[e] >= kl_presets(e); }
static int sound_base(int p, int e) { return is_user(p, e) ? set.user[e].base : set.parts[p].preset[e]; }   /* its
                                                                                                             * preset */
static int sound_count(int e) { return kl_presets(e) + set.user[e].saved; }   /* PRESET's choices */
static const char *sound_name(int p, int e) { return is_user(p, e) ? "USER" : kl_preset_name(e, set.parts[p].preset[e]); }

static void sound_load(int p, int e)                 /* a part's sound of an engine as chosen: an edit is forgotten */
{
    uint32_t k;
    if (is_user(p, e))
        for (k = 0; k < 8u; k++)
            work[p][e][k] = set.user[e].v[k];
    else
        kl_preset_values(e, set.parts[p].preset[e], work[p][e]);
    for (k = 0; k < 8u; k++)
        orig[p][e][k] = work[p][e][k];
    edited[p][e] = 0;
}

static void edited_update(int p, int e)              /* edited while a value is not its sound's */
{
    uint32_t k;
    edited[p][e] = 0;
    for (k = 0; k < 8u; k++)
        edited[p][e] |= work[p][e][k] != orig[p][e][k];
}

static void dirty_update(void)                       /* SAVE lit while the settings or a sound differ from the store */
{
    uint32_t i, e;
    const uint8_t *a = (const uint8_t *)&set, *b = (const uint8_t *)&set_kept;
    set_dirty = 0;
    for (i = 0; i < sizeof set; i++)
        set_dirty |= a[i] != b[i];
    for (i = 0; i < KL_NPART; i++)
        for (e = 0; e < KL_NENG; e++)
            set_dirty |= edited[i][e];
}

static void sound_select(int p)                      /* a part's sound to the DSP */
{
    int e = set.parts[p].engine;
    kl_select(p, e, sound_base(p, e), work[p][e]);
}

/* ------------------------------------------------------------------ the mixer --- */
static const int16_t LEVEL_Q11[47] = {               /* 2048 * 10^(dB / 20), rounded, for dB -40 .. +6 */
    20, 23, 26, 29, 32, 36, 41, 46, 51, 58, 65, 73,
    82, 91, 103, 115, 129, 145, 163, 183, 205, 230, 258, 289,
    325, 364, 409, 458, 514, 577, 648, 727, 815, 915, 1026, 1152,
    1292, 1450, 1627, 1825, 2048, 2298, 2578, 2893, 3246, 3642, 4086};
static volatile int32_t mix_tgt[KL_NPART][2];        /* each part's gain to L and R, Q11: the main loop sets them */
static volatile int32_t snd_tgt[KL_NPART][2];        /* each part's sends to DELAY and HALL, Q11 */
static volatile int32_t ret_tgt[2];                  /* the effects' returns, Q11 */

static int32_t level_q11(int32_t db) { return db < -40 ? 0 : LEVEL_Q11[db + 40]; }

/* the gains from LEVEL, PAN, MUTE and SOLO: a soloed part mutes every part not soloed; PAN keeps the near side and
 * turns the far one down (the middle: both at LEVEL, as without a mixer). The sends follow LEVEL, MUTE and SOLO, not
 * PAN (the effects take them mono), each send squared (50: -12 dB, as an audio taper); the returns the effects'
 * LEVEL */
static void mix_update(void)
{
    int p, c, solo = 0;
    for (p = 0; p < KL_NPART; p++)
        solo |= set.parts[p].solo;
    for (p = 0; p < KL_NPART; p++) {
        const part_set_t *ps = &set.parts[p];
        int32_t g = ps->mute || (solo && !ps->solo) ? 0 : level_q11(ps->level);
        mix_tgt[p][0] = g * (ps->pan > 0 ? 50 - ps->pan : 50) / 50;
        mix_tgt[p][1] = g * (ps->pan < 0 ? 50 + ps->pan : 50) / 50;
        for (c = 0; c < 2; c++)
            snd_tgt[p][c] = g * ps->send[c] * ps->send[c] / 10000;
    }
    for (c = 0; c < 2; c++)
        ret_tgt[c] = level_q11(set.fx[c][FK_LEVEL]);
}

/* ------------------------------------------------------------- the looper --- */
/* v0.5's note looper: what the keys and MIDI play into the part recording, as notes (a tick, a note, a velocity), on
 * one loop all four parts share. The render keeps it (app_render, a step of 32 frames at a time, so a note lands
 * within 0.73 ms): it plays the notes, records, counts in and clicks. The main loop asks through a queue (lp_ask) and
 * reads what it shows; the loops live in RAM only (gone at power-off). */
#define LP_PPQN 192                                  /* ticks a quarter note */
#define LP_BAR (4 * LP_PPQN)                         /* 4/4 */
#define LP_MAXBARS 16                                /* 12288 ticks: a tick fits 16 bits */
#define LP_EV 2048                                   /* a part's notes on and off: 1024 notes */
#define LP_STEP16(bpm) ((uint32_t)(bpm) * 152174u / 1000u)   /* ticks a step, Q16: bpm / 60 * 192 * 32 / 44100 *
                                                             * 65536 = bpm * 152.174 */
enum { LP_STOP, LP_COUNT, LP_PLAY };                 /* stopped, counting in (a bar), playing */
enum { LC_PLAY, LC_STOP, LC_REC, LC_UNREC, LC_PART, LC_UNDO, LC_CLEAR, LC_BARS,   /* the main loop's asks */
       LC_SSET, LC_SCLR, LC_SEDIT };                 /* the step sequencer's: a step set, cleared, changed */
enum { SE_NOTE, SE_VEL, SE_GATE };                   /* LC_SEDIT: what */
#define LP_STEP (LP_PPQN / 4)                        /* a step: 1/16 */
#define LP_ANY 0xFFu                                 /* a step's notes, all of them (else those of one semitone: a
                                                     * DRUMS instrument) */
static const uint8_t QUANT_TICKS[5] = {0, 24, 48, 96, 192};   /* QUANT OFF, 1/32, 1/16, 1/8, 1/4 */
typedef struct {
    uint16_t tick;
    uint8_t note;
    uint8_t vel;                                     /* 1..127 on, 0 off; bit 7: recorded ahead of the play line (it
                                                     * sounded live), passed over once */
} lp_ev_t;
static lp_ev_t lp_ev[KL_NPART][LP_EV] PLAT_POOL;     /* each part's, in tick order */
static lp_ev_t lp_undo[LP_EV] PLAT_POOL;             /* the last take's part as it was before (after, once undone) */
static struct {
    volatile uint16_t n[KL_NPART];                   /* each part's events */
    uint16_t cur[KL_NPART];                          /* each one's next to play */
    volatile uint8_t state, rec, rpart, first;       /* LP_*; recording, into which part; the first take runs (LENGTH
                                                     * AUTO: its end sets the length) */
    volatile uint8_t bars, first_bars;               /* the length (0: AUTO, none yet), and the first take's (HOME +
                                                     * K2 goes back to it) */
    volatile uint8_t tempo, quant, click;            /* the main loop's: 40..240 BPM, QUANT_TICKS' index, CLICK OFF,
                                                     * REC (counting in and recording), ON */
    volatile uint32_t pos16, count16;                /* where it plays, and the count-in so far: ticks, Q16 */
    uint32_t on[KL_NPART][4];                        /* the notes it has sounding, a bit a note */
    uint32_t held[4];                                /* the notes recorded on and not off yet */
    uint16_t nheld;
    int16_t shift[128];                              /* how far QUANT moved each one's start (its end moves alike) */
    volatile int8_t undo_part;                       /* the last take's part (-1: none) */
    volatile uint8_t undone;                         /* 1: it is taken back (lp_undo holds the take) */
    uint16_t undo_n;
    uint8_t undo_first, undo_bars, undo_first_bars;  /* it was the first take: the length goes with it */
    uint8_t click_now;                               /* a click due at the next step (the loop starts) */
    volatile uint8_t full;                           /* notes left out, the part full: a count (the main loop says so) */
} lp;
#define LC_N 16u
static volatile uint32_t lc_w, lc_r;                 /* main loop writes lc_w, the render lc_r */
static struct {
    uint8_t cmd, arg, a, b;                          /* the ask, its part (or value); a step's: a, b, t, c */
    uint16_t t;
    int16_t c;
} lc_q[LC_N];
#define LK_N 64u
static volatile uint32_t lk_w, lk_r;                 /* the keys played, for the recording */
static uint8_t lk_part[LK_N], lk_on[LK_N], lk_note[LK_N], lk_vel[LK_N];
static int32_t ck_buf[APP_BLOCK];                    /* the click, as the render mixes it */
static int32_t ck_s, ck_c, ck_e, ck_env;             /* its sine (Q15 steps of ck_e), its envelope (Q15) */

static void loop_init(void)
{
    uint32_t p;
    uint8_t *b = (uint8_t *)&lp;
    for (p = 0; p < sizeof lp; p++)
        b[p] = 0;
    lp.tempo = 120;
    lp.quant = 2;                                    /* 1/16 */
    lp.click = 1;                                    /* REC */
    lp.undo_part = -1;
    lc_r = lc_w;
    lk_r = lk_w;
}

/* main loop: an ask for the render, in order; a step's: its part, its tick t, and a, b, c (LC_SSET: note, velocity,
 * gate in ticks; LC_SCLR: the semitone or LP_ANY; LC_SEDIT: the semitone or LP_ANY, SE_*, the value) */
static void lp_ask_step(int cmd, int arg, uint32_t t, int a, int b, int c)
{
    uint32_t w = lc_w;
    if (w - lc_r >= LC_N)
        return;
    lc_q[w % LC_N].cmd = (uint8_t)cmd;
    lc_q[w % LC_N].arg = (uint8_t)arg;
    lc_q[w % LC_N].t = (uint16_t)t;
    lc_q[w % LC_N].a = (uint8_t)a;
    lc_q[w % LC_N].b = (uint8_t)b;
    lc_q[w % LC_N].c = (int16_t)c;
    lc_w = w + 1u;
}

static void lp_ask(int cmd, int arg) { lp_ask_step(cmd, arg, 0, 0, 0, 0); }

static void lp_key(int part, int on, int note, int vel)   /* main loop: a key played, for the recording */
{
    uint32_t w = lk_w;
    if (w - lk_r >= LK_N)
        return;
    lk_part[w % LK_N] = (uint8_t)part;
    lk_on[w % LK_N] = (uint8_t)(on != 0);
    lk_note[w % LK_N] = (uint8_t)note;
    lk_vel[w % LK_N] = (uint8_t)vel;
    lk_w = w + 1u;
}

static int lp_empty(void)                            /* no notes anywhere, no take running */
{
    int p;
    for (p = 0; p < KL_NPART; p++)
        if (lp.n[p])
            return 0;
    return !lp.rec && !lp.first;
}

static uint32_t lp_len(void) { return (lp.bars ? lp.bars : LP_MAXBARS) * (uint32_t)LP_BAR; }   /* ticks */

static uint32_t lp_lower(int p, uint32_t at16)       /* a part's first event at at16 (ticks Q16) or after */
{
    uint32_t i = 0, n = lp.n[p];
    while (i < n && ((uint32_t)lp_ev[p][i].tick << 16) < at16)
        i++;
    return i;
}

static void lp_notes_off(int p)                      /* the part's notes the looper has sounding, off */
{
    uint32_t w, b;
    for (w = 0; w < 4u; w++)
        for (b = 0; lp.on[p][w] && b < 32u; b++)
            if (lp.on[p][w] & 1u << b) {
                kl_key_now(p, 0, (int)(w * 32u + b), 0);
                lp.on[p][w] &= ~(1u << b);
            }
}

/* an event into its place in a part: after those at its tick. Recorded live, ahead of the play line it is passed over
 * once (it has sounded); behind the line the next to play stays the same */
static void lp_insert(int p, uint32_t tick, int note, int vel, int live)
{
    lp_ev_t *e = lp_ev[p];
    uint32_t n = lp.n[p], i = n;
    while (i && e[i - 1u].tick > tick) {
        e[i] = e[i - 1u];
        i--;
    }
    e[i].tick = (uint16_t)tick;
    e[i].note = (uint8_t)note;
    e[i].vel = (uint8_t)vel;
    if (live && (lp.state == LP_COUNT || (lp.state == LP_PLAY && (tick << 16) >= lp.pos16)))
        e[i].vel |= 0x80u;
    else if (live ? i <= lp.cur[p] : i < lp.cur[p] || (i == lp.cur[p] && (tick << 16) < lp.pos16))
        lp.cur[p]++;
    lp.n[p] = (uint16_t)(n + 1u);
}

static void lp_remove(int p, uint32_t i)             /* an event out (the next to play stays the same) */
{
    lp_ev_t *e = lp_ev[p];
    uint32_t n = lp.n[p], k;
    if (i >= n)
        return;
    for (k = i; k + 1u < n; k++)
        e[k] = e[k + 1u];
    if (i < lp.cur[p])
        lp.cur[p]--;
    lp.n[p] = (uint16_t)(n - 1u);
}

/* a note into the part recording: its start moved to QUANT's grid, its end as far; counting in, a note just before
 * the loop starts lands on its first tick */
static void lp_record(int p, int on, int note, int vel)
{
    int32_t len = (int32_t)lp_len(), t, q = QUANT_TICKS[lp.quant], sh = 0;
    uint32_t w = (uint32_t)note >> 5, bit = 1u << (note & 31);
    if (!lp.rec || p != lp.rpart || note < 0 || note > 127)
        return;
    if (lp.state == LP_COUNT) {
        int32_t left = (int32_t)(LP_BAR - (lp.count16 >> 16));
        if (on && left > (q ? q / 2 : 24))
            return;
        t = 0;
    } else if (lp.state == LP_PLAY) {
        t = (int32_t)((lp.pos16 + 0x8000u) >> 16);
    } else {
        return;
    }
    if (on) {
        if (lp.held[w] & bit)
            return;
        if (lp.n[p] + 2u + lp.nheld > LP_EV) {      /* room kept for every held note's end */
            lp.full++;
            return;
        }
        if (q && lp.state == LP_PLAY) {
            sh = (t + q / 2) / q * q - t;
            t += sh;
        }
        lp.held[w] |= bit;
        lp.nheld++;
        lp.shift[note] = (int16_t)sh;
    } else {
        if (!(lp.held[w] & bit))
            return;
        t += lp.shift[note];
        lp.held[w] &= ~bit;
        lp.nheld--;
    }
    t = (t % len + len) % len;
    lp_insert(p, (uint32_t)t, note, on ? clamp(vel, 1, 127) : 0, 1);
}

/* the first take ends (LENGTH AUTO): the length in whole bars, the nearest; what is past its end moves to its start */
static void lp_first_end(void)
{
    uint32_t at = lp.pos16 >> 16, bars = (uint32_t)clamp((int32_t)((at + LP_BAR / 2) / LP_BAR), 1, LP_MAXBARS);
    uint32_t len = bars * LP_BAR, p = lp.rpart, n = lp.n[p], k = 0, i, j;
    lp_ev_t *e = lp_ev[p];
    while (k < n && e[n - 1u - k].tick >= len)
        k++;
    for (i = 0; i < k; i++)
        e[n - k + i].tick = (uint16_t)(e[n - k + i].tick - len);
    for (i = 0, j = n - 1u; k && i < j; i++, j--) {  /* the tail before the rest: three reversals */
        lp_ev_t x = e[i];
        e[i] = e[j];
        e[j] = x;
    }
    for (i = 0, j = k - 1u; k && i < j; i++, j--) {
        lp_ev_t x = e[i];
        e[i] = e[j];
        e[j] = x;
    }
    for (i = k, j = n - 1u; k && i < j; i++, j--) {
        lp_ev_t x = e[i];
        e[i] = e[j];
        e[j] = x;
    }
    if (lp.pos16 >= len << 16)
        lp.pos16 -= len << 16;
    lp.bars = lp.first_bars = (uint8_t)bars;
    lp.first = 0;
    lp.cur[p] = (uint16_t)lp_lower((int)p, lp.pos16);
}

static void lp_take_end(void)                        /* the recording stops: each held note ends where it is now */
{
    uint32_t w, b;
    if (!lp.rec)
        return;
    for (w = 0; w < 4u; w++)
        for (b = 0; lp.held[w] && b < 32u; b++)
            if (lp.held[w] & 1u << b)
                lp_record(lp.rpart, 0, (int)(w * 32u + b), 0);
    lp.nheld = 0;
    if (lp.first && lp.state == LP_COUNT) {          /* the first take given up while counting in: nothing kept */
        lp.n[lp.rpart] = 0;
        lp.first = 0;
    } else if (lp.first) {
        lp_first_end();
    }
    lp.rec = 0;
}

static void lp_keep(int p)                           /* a part as it is now, for HOME + REC */
{
    uint32_t i;
    for (i = 0; i < lp.n[p]; i++)
        lp_undo[i] = lp_ev[p][i];
    lp.undo_n = lp.n[p];
    lp.undo_part = (int8_t)p;
    lp.undo_first = lp_empty();
    lp.undo_bars = lp.bars;
    lp.undo_first_bars = lp.first_bars;
    lp.undone = 0;
}

static void lp_take_start(int p)                     /* the recording starts on a part (a take) */
{
    uint32_t w;
    if (lp.rec)
        return;
    lp_keep(p);
    if (lp_empty()) {                                /* the first take: AUTO sets the length at its end */
        lp.first = lp.bars == 0;
        lp.first_bars = lp.bars;
    }
    for (w = 0; w < 4u; w++)
        lp.held[w] = 0;
    lp.nheld = 0;
    lp.rpart = (uint8_t)p;
    lp.rec = 1;
}

static void lp_restart(void)                         /* from the loop's start */
{
    int p;
    lp.pos16 = 0;
    for (p = 0; p < KL_NPART; p++)
        lp.cur[p] = 0;
    lp.click_now = 1;
    lp.state = LP_PLAY;
}

static void lp_stop(void)
{
    int p;
    lp_take_end();
    for (p = 0; p < KL_NPART; p++)
        lp_notes_off(p);
    lp.state = LP_STOP;
    lp.pos16 = lp.count16 = 0;
}

static void lp_swap(void)                            /* HOME + REC: the last take out, or back in */
{
    uint32_t i, m;
    int p = lp.undo_part;
    if (p < 0)
        return;
    lp_take_end();
    lp_notes_off(p);
    m = lp.n[p] > lp.undo_n ? lp.n[p] : lp.undo_n;
    for (i = 0; i < m; i++) {
        lp_ev_t x = lp_ev[p][i];
        lp_ev[p][i] = lp_undo[i];
        lp_undo[i] = x;
    }
    i = lp.n[p];
    lp.n[p] = lp.undo_n;
    lp.undo_n = (uint16_t)i;
    if (lp.undo_first) {                             /* the first take: its length goes with it */
        uint8_t b = lp.bars, f = lp.first_bars;
        lp.bars = lp.undo_bars;
        lp.first_bars = lp.undo_first_bars;
        lp.undo_bars = b;
        lp.undo_first_bars = f;
    }
    lp.undone ^= 1u;
    if (lp.bars == 0)
        lp_stop();
    lp.cur[p] = (uint16_t)lp_lower(p, lp.pos16);
}

static void lp_set_bars(uint32_t bars)               /* K2: shorter hides the bars after, longer adds empty ones */
{
    int p;
    if (lp.first || (bars == 0 && !lp_empty()) || bars > LP_MAXBARS)
        return;
    lp.bars = (uint8_t)bars;
    for (p = 0; p < KL_NPART; p++)
        lp_notes_off(p);
    if (bars == 0)
        lp_stop();
    else if (lp.pos16 >= lp_len() << 16)
        lp.pos16 %= lp_len() << 16;
    for (p = 0; p < KL_NPART; p++)
        lp.cur[p] = (uint16_t)lp_lower(p, lp.pos16);
}

/* ------------------------------------------------- the step sequencer's steps --- */
/* a step is 1/16 of the loop at tick t; a note starting within half a step of it (either side, round the loop's end)
 * is the step's. sel: LP_ANY, or a semitone (DRUMS: one instrument) */
static int lp_in_step(uint32_t tick, uint32_t t, uint32_t len)
{
    return (tick + len + LP_STEP / 2u - t) % len < LP_STEP;
}

static int lp_step_note(const lp_ev_t *e, uint32_t t, uint32_t len, uint32_t sel)
{
    return (e->vel & 0x7Fu) && e->tick < len && lp_in_step(e->tick, t, len) && (sel == LP_ANY || e->note % 12u == sel);
}

static int32_t lp_end_of(int p, uint32_t i)          /* the index of note i's end: the next off of its note, round the
                                                     * loop (-1: none) */
{
    const lp_ev_t *e = lp_ev[p];
    uint32_t n = lp.n[p], k, j;
    for (k = 1; k < n; k++) {
        j = (i + k) % n;
        if (e[j].note == e[i].note && !(e[j].vel & 0x7Fu))
            return (int32_t)j;
    }
    return -1;
}

static void lp_unsound(int p, int note)              /* the note off now, if the looper has it sounding */
{
    uint32_t w = (uint32_t)note >> 5, bit = 1u << (note & 31);
    if (lp.on[p][w] & bit) {
        kl_key_now(p, 0, note, 0);
        lp.on[p][w] &= ~bit;
    }
}

/* a step's notes changed: the last take's part can no longer be taken back on its own (HOME + REC would undo these
 * too) */
static void lp_stepped(int p)
{
    if (lp.undo_part == p)
        lp.undo_part = -1;
}

static void lp_step_set(int p, uint32_t t, int note, int vel, int gate)   /* a note on the step, as long as gate */
{
    uint32_t len;
    if (lp.first || (lp.rec && lp.state == LP_COUNT))
        return;
    if (!lp.bars) {                                  /* empty: the first step makes a bar */
        if (!lp_empty())
            return;
        lp_set_bars(1);
        lp.first_bars = 1;
    }
    len = lp_len();
    if (t >= len || lp.n[p] + 2u + lp.nheld > LP_EV) {
        lp.full++;
        return;
    }
    lp_insert(p, t, note, vel, 0);
    lp_insert(p, (t + (uint32_t)gate) % len, note, 0, 0);
    lp_stepped(p);
}

static void lp_step_clear(int p, uint32_t t, uint32_t sel)   /* the step's notes out, with their ends */
{
    uint32_t len = lp_len(), i;
    for (;;) {
        int32_t j;
        for (i = 0; i < lp.n[p] && !lp_step_note(&lp_ev[p][i], t, len, sel); i++)
            ;
        if (i >= lp.n[p])
            break;
        lp_unsound(p, lp_ev[p][i].note);
        j = lp_end_of(p, i);
        if (j > (int32_t)i) {
            lp_remove(p, (uint32_t)j);
            lp_remove(p, i);
        } else {
            lp_remove(p, i);
            if (j >= 0)
                lp_remove(p, (uint32_t)j);
        }
        lp_stepped(p);
    }
}

/* a step's notes changed: SE_NOTE by v semitones, SE_VEL to v, SE_GATE to v ticks (the end moved) */
static void lp_step_edit(int p, uint32_t t, uint32_t sel, int what, int v)
{
    uint32_t len = lp_len(), i, k, m = 0;
    uint16_t at[16];
    uint8_t nt[16];
    for (i = 0; i < lp.n[p] && m < 16u; i++)        /* the step's notes, as they are */
        if (lp_step_note(&lp_ev[p][i], t, len, sel)) {
            at[m] = lp_ev[p][i].tick;
            nt[m++] = lp_ev[p][i].note;
        }
    for (k = 0; k < m; k++) {
        int32_t j;
        for (i = 0; i < lp.n[p] && !(lp_ev[p][i].tick == at[k] && lp_ev[p][i].note == nt[k] &&
                                     (lp_ev[p][i].vel & 0x7Fu)); i++)
            ;
        if (i >= lp.n[p])
            continue;
        j = lp_end_of(p, i);
        if (what == SE_VEL) {
            lp_ev[p][i].vel = (uint8_t)((lp_ev[p][i].vel & 0x80u) | (uint32_t)clamp(v, 1, 127));
        } else if (what == SE_NOTE) {
            int to = clamp(nt[k] + v, 0, 127);
            lp_unsound(p, nt[k]);
            lp_ev[p][i].note = (uint8_t)to;
            if (j >= 0)
                lp_ev[p][j].note = (uint8_t)to;
        } else if (j >= 0) {                         /* the end out, and in again where the gate ends */
            lp_unsound(p, nt[k]);
            lp_remove(p, (uint32_t)j);
            lp_insert(p, (at[k] + (uint32_t)v) % len, nt[k], 0, 0);
        }
    }
    if (m)
        lp_stepped(p);
}

static void lp_asks(void)                            /* render: the main loop's asks, then the keys it played */
{
    while (lc_r != lc_w) {
        uint32_t j = lc_r % LC_N;
        int a = lc_q[j].arg;
        switch (lc_q[j].cmd) {
        case LC_PLAY:
            if (lp.state == LP_STOP && lp.bars)
                lp_restart();
            break;
        case LC_STOP:
            lp_stop();
            break;
        case LC_REC:                                 /* stopped: a bar's count-in first (CLICK OFF: none) */
            if (lp.state == LP_STOP && lp.click) {
                lp.state = LP_COUNT;
                lp.count16 = 0;
                lp.click_now = 1;
            } else if (lp.state == LP_STOP) {
                lp_restart();
            }
            lp_take_start(a);
            break;
        case LC_UNREC:                               /* counting in: given up */
            if (lp.state == LP_COUNT)
                lp_stop();
            else
                lp_take_end();
            break;
        case LC_PART:                                /* SEL while recording: the recording goes along */
            if (lp.rec && lp.rpart != a) {
                lp_take_end();
                lp_take_start(a);
            }
            break;
        case LC_UNDO:
            lp_swap();
            break;
        case LC_CLEAR:
            if (lp.rec && lp.rpart == a)
                lp_take_end();
            lp_keep(a);
            lp_notes_off(a);
            lp.n[a] = lp.cur[a] = 0;
            break;
        case LC_BARS:
            lp_set_bars((uint32_t)a);
            break;
        case LC_SSET:
            lp_step_set(a, lc_q[j].t, lc_q[j].a, lc_q[j].b, lc_q[j].c);
            break;
        case LC_SCLR:
            lp_step_clear(a, lc_q[j].t, lc_q[j].a);
            break;
        case LC_SEDIT:
            lp_step_edit(a, lc_q[j].t, lc_q[j].a, lc_q[j].b, lc_q[j].c);
            break;
        default:
            break;
        }
        lc_r = lc_r + 1u;
    }
    while (lk_r != lk_w) {
        uint32_t j = lk_r % LK_N;
        lp_record(lk_part[j], lk_on[j], lk_note[j], lk_vel[j]);
        lk_r = lk_r + 1u;
    }
}

static void lp_fire(int p, uint32_t to16, uint32_t len)   /* a part's notes before to16 (ticks Q16) and len */
{
    lp_ev_t *e = lp_ev[p];
    uint32_t i = lp.cur[p], n = lp.n[p];
    while (i < n && e[i].tick < len && ((uint32_t)e[i].tick << 16) < to16) {
        uint32_t w = (uint32_t)e[i].note >> 5, bit = 1u << (e[i].note & 31u);
        if (e[i].vel & 0x80u) {                      /* it sounded live as it was recorded */
            e[i].vel &= 0x7Fu;
        } else if (e[i].vel) {
            kl_key_now(p, 1, e[i].note, e[i].vel);
            lp.on[p][w] |= bit;
        } else if (lp.on[p][w] & bit) {              /* only a note it began (the engine ends every voice of it) */
            kl_key_now(p, 0, e[i].note, 0);
            lp.on[p][w] &= ~bit;
        }
        i++;
    }
    lp.cur[p] = (uint16_t)i;
}

/* the click: a sine that dies away in ~40 ms, 1.5 kHz on a bar's first beat, 1 kHz on the others (ck_e is
 * 2 sin(pi f / 44100), Q15) */
static void click_start(int accent)
{
    ck_e = accent ? 6990 : 4665;
    ck_s = 0;
    ck_c = accent ? 20000 : 12000;                   /* about a voice's level (RESEARCH.md: Looper) */
    ck_env = 32767;
}

static void lp_step(int32_t *ck)                     /* render: one step of 32 frames; the click into ck */
{
    uint32_t inc = LP_STEP16(lp.tempo), beat16 = (uint32_t)LP_PPQN << 16, i;
    int p, clicks = lp.click == 2 || (lp.click == 1 && (lp.rec || lp.state == LP_COUNT));
    if (lp.state == LP_COUNT) {
        uint32_t a = lp.count16, b = a + inc;
        if (lp.click_now || a / beat16 != b / beat16) {
            if (clicks)
                click_start(lp.click_now || b / beat16 == 4u);
            lp.click_now = 0;
        }
        lp.count16 = b;
        if (b >= (uint32_t)LP_BAR << 16) {           /* the loop starts */
            lp.pos16 = b - ((uint32_t)LP_BAR << 16);
            for (p = 0; p < KL_NPART; p++)
                lp.cur[p] = 0;
            lp.state = LP_PLAY;
        }
    } else if (lp.state == LP_PLAY) {
        uint32_t len = lp_len(), a = lp.pos16, b = a + inc;
        if (lp.click_now || a / beat16 != b / beat16) {
            if (clicks)
                click_start(lp.click_now || (b / beat16) % 4u == 0);
            lp.click_now = 0;
        }
        if (lp.first && b >= len << 16) {            /* the first take reached 16 bars: that is its length */
            lp.bars = lp.first_bars = LP_MAXBARS;
            lp.first = 0;
        }
        if (b >= len << 16) {                        /* the loop's end: the rest, then from its start */
            for (p = 0; p < KL_NPART; p++) {
                lp_fire(p, len << 16, len);
                lp.cur[p] = 0;
                if (lp.n[p] && lp_ev[p][lp.n[p] - 1u].tick >= len)   /* bars hidden by a shorter LENGTH: what
                                                                      * they would end, ends */
                    lp_notes_off(p);
            }
            b -= len << 16;
        }
        for (p = 0; p < KL_NPART; p++)
            lp_fire(p, b, len);
        lp.pos16 = b;
    }
    for (i = 0; i < 32u; i++) {
        if (ck_env > 30) {
            ck_s += ck_e * ck_c >> 15;
            ck_c -= ck_e * ck_s >> 15;
            ck_env -= ck_env >> 9;                   /* 11.6 ms a step of 1/e */
            ck[i] = ck_s * ck_env >> 15;
        } else {
            ck[i] = 0;
        }
    }
}

/* ------------------------------------------------------------ the effects --- */
/* DELAY's TIME: a note value of the looper's TEMPO, in 24ths of a quarter (1/32, 1/16T, 1/16, 1/8T, 1/16., 1/8,
 * 1/4T, 1/8., 1/4); longer than its line (0.5 s: a slow tempo), half of it, until it fits (still in time) */
static const uint8_t DLY_K[9] = {3, 4, 6, 8, 9, 12, 16, 18, 24};
static const char *const DLY_NAME[9] = {"1/32", "1/16T", "1/16", "1/8T", "1/16.", "1/8", "1/4T", "1/8.", "1/4"};
static uint8_t fx_tempo;                             /* the TEMPO DELAY's time was last set at */

static uint32_t dly_samples(int *halved)             /* 44100 * 60 / bpm * k / 24 = 110250 * k / bpm samples */
{
    uint32_t d = 110250u * DLY_K[set.fx[FX_DELAY][FK_TIME]] / lp.tempo;
    int h = 0;
    while (d > FX_DLY_N) {
        d /= 2u;
        h++;
    }
    if (halved)
        *halved = h;
    return d;
}

static const char *dly_played(void)                  /* the note value it plays: TIME's, or a half or quarter of it
                                                     * (48ths of a quarter; 40 BPM halves twice at most) */
{
    static const uint8_t K48[10] = {6, 8, 9, 12, 16, 18, 24, 32, 36, 48};
    static const char *const N48[10] = {"1/32", "1/16T", "1/32.", "1/16", "1/8T", "1/16.", "1/8", "1/4T", "1/8.",
                                        "1/4"};
    int h, i;
    uint32_t k;
    dly_samples(&h);
    k = 2u * DLY_K[set.fx[FX_DELAY][FK_TIME]] >> h;
    for (i = 0; i < 10; i++)
        if (K48[i] == k)
            return N48[i];
    return "?";
}

static void fx_apply(void)                           /* the effects' knobs (and TEMPO) to the DSP */
{
    const int8_t *d = set.fx[FX_DELAY], *h = set.fx[FX_HALL];
    fx_tempo = lp.tempo;
    fx_delay_set(dly_samples(0), d[FK_FDBK], d[FK_TONE]);
    fx_hall_set((uint32_t)h[0] * 441u / 10u, h[1], h[2]);   /* PRE: ms * 44.1 */
}

/* ------------------------------------------------------------------ audio --- */
static int32_t part_buf[KL_NPART][APP_BLOCK];
static int32_t gain_q8;                              /* MASTER's gain, gliding: no step at a block's edge */
static int32_t mix_q19[KL_NPART][2];                 /* the parts' gains as they glide, Q19 (Q11 << 8) */
static int32_t snd_q19[KL_NPART][2], ret_q19[2];     /* the sends and returns, alike */
static int32_t snd_buf[2][APP_BLOCK];                /* the sends mixed, mono: DELAY's, HALL's */
static int32_t ret_buf[2][2 * APP_BLOCK];            /* the effects' outputs, L and R */
static int16_t fx_dly_buf[2 * FX_DLY_N] PLAT_POOL;   /* DELAY's lines, L then R: 88 200 B */
static int16_t fx_pre_buf[FX_PRE_N] PLAT_POOL;       /* PRE's: 8 820 B */
static uint16_t fx_hall_buf[FX_HALL_N] PLAT_POOL;    /* HALL's: 65 536 B */
static int32_t lim_q24 = 1 << 24;                    /* the limiter's gain, Q24 */
static volatile int32_t out_peak[2];                 /* the output's peak, L and R, since the screen last looked */
static volatile int32_t lim_low = 1 << 16;           /* the limiter's lowest gain (Q16) since the screen last looked */
#ifdef PLAT_HOST
static int32_t test_peak[2], test_lim = 1 << 16;     /* the same for the host's checks (app_expect reads and resets) */
#endif
static volatile uint32_t midi_seen;
#define LIM_TOP 7919000                              /* the limiter's ceiling: -0.5 dBFS (8388607 * 10^(-0.5 / 20)) */

/* (a * b) >> 7 in 64 bits, out of line: an inlined 64-bit multiply beside loads now and then leaves a register a bit
 * off on the FM-1 (base/bsp/docs/CPU.md) */
static __attribute__((noinline)) int64_t mul_shr7(int32_t a, int32_t b) { return ((int64_t)a * b) >> 7; }
static __attribute__((noinline)) int32_t mul_q16(int32_t a, int32_t g) { return (int32_t)(((int64_t)a * g) >> 16); }

static int32_t clamp30(int64_t v) { return v > (1 << 30) ? 1 << 30 : v < -(1 << 30) ? -(1 << 30) : (int32_t)v; }

static void midi_in(void)                            /* audio interrupt: the packets that came in, channel 1..6 to part
                                                     * 1..6, the others to the part chosen (and to the looper) */
{
    uint32_t pkt;
    int port;
    while (plat_midi_in(&pkt, &port)) {
        uint32_t st = (pkt >> 8) & 0xFFu, d1 = (pkt >> 16) & 0x7Fu, d2 = (pkt >> 24) & 0x7Fu;
        int part = (st & 0x0Fu) < KL_NPART ? (int)(st & 0x0Fu) : set.part;
        if ((st & 0xF0u) == 0x90u && d2) {
            kl_key_now(part, 1, (int)d1, (int)d2);
            lp_record(part, 1, (int)d1, (int)d2);
        } else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u) {
            kl_key_now(part, 0, (int)d1, 0);
            lp_record(part, 0, (int)d1, 0);
        }
        if (st != 0xF8u && st != 0xFEu)
            midi_seen++;
    }
}

_Static_assert(APP_BLOCK % 32u == 0, "the render goes in steps of 32 frames (the looper's)");
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    uint32_t i, p, b;
    int32_t target = (int32_t)master_q12 << 8;
    if (n > APP_BLOCK)
        n = APP_BLOCK;
    midi_in();
    lp_asks();
    for (p = 0; p < KL_NPART; p++)
        for (i = 0; i < n; i++)
            part_buf[p][i] = 0;
    for (b = 0; b + 32u <= n; b += 32u) {           /* a step at a time: the looper's notes land in theirs */
        int32_t *out[KL_NPART];
        for (p = 0; p < KL_NPART; p++)
            out[p] = part_buf[p] + b;
        lp_step(ck_buf + b);
        kl_render(out, 32u);
    }
    {   /* the sends mixed mono (after LEVEL, MUTE and SOLO), then the effects */
        int32_t c, on[2] = {0, 0};
        for (p = 0; p < KL_NPART; p++)
            for (c = 0; c < 2; c++)
                on[c] |= snd_tgt[p][c] != 0 || snd_q19[p][c] != 0;
        for (i = 0; i < n; i++) {
            int32_t s[2] = {0, 0};
            if (on[0] || on[1])
                for (p = 0; p < KL_NPART; p++) {
                    int32_t x = clamp(part_buf[p][i], -524287, 524287);
                    for (c = 0; c < 2; c++) {
                        snd_q19[p][c] += ((snd_tgt[p][c] << 8) - snd_q19[p][c]) >> 8;   /* ~6 ms */
                        s[c] += x * (snd_q19[p][c] >> 8) >> 11;
                    }
                }
            snd_buf[0][i] = s[0];
            snd_buf[1][i] = s[1];
        }
        fx_render(snd_buf[0], snd_buf[1], ret_buf[0], ret_buf[1], n, on[0], on[1]);
    }
    for (i = 0; i < n; i++) {
        int32_t c, e, sum[2] = {0, 0}, y[2], pk, lim;
        for (p = 0; p < KL_NPART; p++) {             /* each part at its gains: L and R */
            int32_t x = clamp(part_buf[p][i], -524287, 524287);   /* 2^19 * the most gain, 4086, fits 32 bits */
            for (c = 0; c < 2; c++) {
                mix_q19[p][c] += ((mix_tgt[p][c] << 8) - mix_q19[p][c]) >> 8;   /* ~6 ms */
                sum[c] += x * (mix_q19[p][c] >> 8) >> 11;
            }
        }
        for (e = 0; e < 2; e++) {                    /* the effects' returns */
            ret_q19[e] += ((ret_tgt[e] << 8) - ret_q19[e]) >> 8;
            for (c = 0; c < 2; c++)
                sum[c] += clamp(ret_buf[e][2 * i + (uint32_t)c], -524287, 524287) * (ret_q19[e] >> 8) >> 11;
        }
        sum[0] += ck_buf[i];                         /* the looper's click, in the middle */
        sum[1] += ck_buf[i];
        gain_q8 += (target - gain_q8) >> 8;          /* ~6 ms */
        for (c = 0; c < 2; c++)
            y[c] = clamp30(mul_shr7(sum[c], gain_q8 >> 8));   /* x32 at full MASTER, toward 24-bit */
        /* the limiter: down at once where a sample would pass the ceiling, back up in ~46 ms (2048 samples) */
        pk = y[0] < 0 ? -y[0] : y[0];
        pk = y[1] > pk ? y[1] : -y[1] > pk ? -y[1] : pk;
        lim = lim_q24 >> 8;
        if (mul_q16(pk, lim) > LIM_TOP) {
            lim = (LIM_TOP << 4) / (pk >> 12);       /* LIM_TOP * 2^16 / pk, in 32 bits */
            if (lim > 1 << 16)
                lim = 1 << 16;
            lim_q24 = lim << 8;
        } else {
            lim_q24 += ((1 << 24) - lim_q24) >> 11;
        }
        if (lim < lim_low)
            lim_low = lim;
        for (c = 0; c < 2; c++) {
            int32_t v = clamp(mul_q16(y[c], lim), -8388607, 8388607), a = v < 0 ? -v : v;
            lr[2 * i + (uint32_t)c] = v;
            if (a > out_peak[c])
                out_peak[c] = a;
#ifdef PLAT_HOST
            if (a > test_peak[c])
                test_peak[c] = a;
#endif
        }
#ifdef PLAT_HOST
        if (lim < test_lim)
            test_lim = lim;
#endif
    }
}

void app_panic(void) { kl_silence(); }

static void step_init(void);                         /* the step sequencer's (below) */
static uint32_t step_sel(void);
static uint32_t step_mask(int p, uint32_t sel);

void app_init(void)
{
    int e, p;
    settings_load();
    set_kept = set;
    for (p = 0; p < KL_NPART; p++) {
        for (e = 0; e < KL_NENG; e++)
            sound_load(p, e);
        sound_select(p);
    }
    mix_update();
    loop_init();
    step_init();
    fx_init(fx_dly_buf, fx_pre_buf, fx_hall_buf);
    fx_apply();
}

/* --------------------------------------------------------------------- UI --- */
static const char *const BTN_NAME[NB] = {"FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ",
                                         "PLAY", "REC", "OCT-", "OCT+"};
static const char *const KEY_NAME[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static struct {
    char msg[2][24];                                 /* a short message over the header */
    uint32_t msg_until;
    uint32_t keys, btn;
    int8_t knote[NKEYS];                             /* the note each held key started (the octave may change) */
    uint8_t kpart[NKEYS];                            /* and on which part (SEL may change it meanwhile) */
    uint8_t edit, page;                              /* an edit page is open, which (0: EDIT 1, 1: EDIT 2) */
    uint8_t mix, redraw_mix;                         /* the mixer is open; its strips again (the meters moved) */
    uint8_t loop, redraw_loop, redraw_head;          /* the looper's page is open; its lanes again (the play line
                                                     * moved); the header alone (its bar and beat) */
    uint8_t ask, ask_part;                           /* HOME + PLAY asks before a part is emptied (OCT+ yes) */
    uint8_t fx;                                      /* the effects' page is open */
    uint8_t tab, bar, turned;                        /* the tab of the page open: the looper's LOOP (0) or STEP (1),
                                                     * the mixer's MIX or SEND, the effects' DELAY or HALL; STEP's bar
                                                     * shown; a knob turned while a step was held */
    uint16_t held, held_new;                         /* the steps held (white keys), those set as they went down */
    uint32_t kstep;                                  /* the keys that went down as steps (their release is a step's) */
    uint32_t loop_at, lp_sig;                        /* the lanes last drawn; what the looper did, as last seen */
    uint8_t full_seen;                               /* lp.full as last said */
    char ind[8];                                     /* the header's bar and beat, as last drawn */
    uint32_t meter_at, lim_until;                    /* the meters last read; LIM lit until */
    int32_t meter[2];                                /* the output's meters, L and R, in pixels */
    uint8_t redraw, redraw_pads;                     /* the whole screen; DRUMS' pads only (a hit lit or gone) */
    uint32_t hits, hit_until;                        /* the DSP's note count as last seen; the hit pad lit until */
    int8_t hit_pc;                                   /* the last note's semitone (C = 0) */
} ui;

static int text_eq(const char *a, const char *b)
{
    while (*a && *a == *b)
        a++, b++;
    return *a == *b;
}

static void text_cat(char *d, const char *s)
{
    while (*d)
        d++;
    while (*s)
        *d++ = *s++;
    *d = 0;
}

static void int_text(int32_t v, char *b)             /* signed, no padding */
{
    char t[12];
    uint32_t n = 0, u = (uint32_t)(v < 0 ? -v : v);
    if (v < 0)
        *b++ = '-';
    do {
        t[n++] = (char)('0' + u % 10u);
        u /= 10u;
    } while (u);
    while (n)
        *b++ = t[--n];
    *b = 0;
}

static void two_digits(int32_t v, char *b)           /* 0..99 as "07" */
{
    b[0] = (char)('0' + (v / 10) % 10);
    b[1] = (char)('0' + v % 10);
    b[2] = 0;
}

static void octave_text(char *t)                     /* the chosen part's */
{
    int32_t o = part_cur()->octave;
    t[0] = o < 0 ? '-' : '+';
    t[1] = (char)('0' + (o < 0 ? -o : o));
    t[2] = 0;
}

static void part_text(char *t)                       /* the chosen part's number onto t: "2" */
{
    char n[2] = {(char)('1' + set.part), 0};
    text_cat(t, n);
}

static void sound_title(int p, int e, char *t)       /* its name, with "*" while an edit is not saved */
{
    t[0] = 0;
    text_cat(t, sound_name(p, e));
    if (edited[p][e])
        text_cat(t, "*");
}

/* a parameter's value as the screen shows it: "SAW", "50%", "+2", "10ct" */
static void value_text(const kl_param_t *d, int32_t v, char *b)
{
    int32_t span = d->max - d->min;
    char t[12];
    b[0] = 0;
    if (d->fmt == KL_ENUM) {
        text_cat(b, d->names[clamp(v, d->min, d->max) - d->min]);
    } else if (d->fmt == KL_PCT) {
        int_text(span > 0 ? ((v - d->min) * 100 + span / 2) / span : 0, b);
        text_cat(b, "%");
    } else {
        if (d->min < 0 && v > 0)
            text_cat(b, "+");
        int_text(v, t);
        text_cat(b, t);
        if (d->unit)
            text_cat(b, d->unit);
    }
}

static void param_cat(char *t, int32_t k)            /* "CUT 50%" onto t: parameter k of the sound playing */
{
    kl_param_t d;
    char v[12];
    kl_param(eng_cur(), k, work[set.part][eng_cur()], &d);
    value_text(&d, work[set.part][eng_cur()][k], v);
    text_cat(t, d.label);
    text_cat(t, " ");
    text_cat(t, v);
}

static void announce(const char *t)                  /* to a screen reader only (the screen shows it already) */
{
    if (ann_on())
        ann_say(t, 0, ANN_NOW);
}

void app_say(const char *a, const char *b)
{
    uint32_t i;
    char t[48];
    for (i = 0; i < 23u && a && a[i]; i++)
        ui.msg[0][i] = a[i];
    ui.msg[0][i] = 0;
    for (i = 0; i < 23u && b && b[i]; i++)
        ui.msg[1][i] = b[i];
    ui.msg[1][i] = 0;
    ui.msg_until = plat_ms() + 1300u;
    ui.redraw = 1;
    t[0] = 0;
    text_cat(t, ui.msg[0]);
    if (ui.msg[1][0]) {
        text_cat(t, " ");
        text_cat(t, ui.msg[1]);
    }
    announce(t);
}

static void db_text(int32_t db, char *b, int short_form)   /* a LEVEL: "-6 dB", "+3 dB", "OFF"; short: "-6" */
{
    char t[8];
    b[0] = 0;
    if (db < -40) {
        text_cat(b, "OFF");
        return;
    }
    if (db > 0)
        text_cat(b, "+");
    int_text(db, t);
    text_cat(b, t);
    if (!short_form)
        text_cat(b, " dB");
}

/* a mixer knob's value as the screen shows it: K1 LEVEL "-6 dB" / "OFF", K2 PAN "C" / "L10" / "R25", K3 MUTE and
 * K4 SOLO "ON" / "OFF"; short: as under a strip ("-6", "OFF"). The tab SEND's (k 4, 5): DELAY and HALL, "30" */
static void mix_text(const part_set_t *ps, int32_t k, char *b, int short_form)
{
    char t[8];
    b[0] = 0;
    if (k == 0) {
        db_text(ps->level, b, short_form);
    } else if (k >= 4) {
        int_text(ps->send[k - 4], b);
    } else if (k == 1) {
        text_cat(b, ps->pan < 0 ? "L" : ps->pan > 0 ? "R" : "C");
        if (ps->pan) {
            int_text(ps->pan < 0 ? -ps->pan : ps->pan, t);
            text_cat(b, t);
        }
    } else {
        text_cat(b, (k == 2 ? ps->mute : ps->solo) ? "ON" : "OFF");
    }
}

static const char *const MIX_LABEL[6] = {"LEVEL", "PAN", "MUTE", "SOLO", "DELAY", "HALL"};   /* MIX's, SEND's */
static const char *const FX_LABEL[2][4] = {{"TIME", "FDBK", "TONE", "LEVEL"}, {"PRE", "DECAY", "TONE", "LEVEL"}};

static void fx_text(int e, int32_t k, char *b)       /* an effect's knob: "1/8.", "35%", "20 MS", "0 dB" */
{
    int32_t v = set.fx[e][k];
    b[0] = 0;
    if (k == FK_LEVEL) {
        db_text(v, b, 0);
    } else if (e == FX_DELAY && k == FK_TIME) {
        text_cat(b, DLY_NAME[v]);
    } else {
        int_text(v, b);
        text_cat(b, e == FX_HALL && k == 0 ? " MS" : "%");
    }
}

static const char *const LOOP_LABEL[4] = {"TEMPO", "LENGTH", "QUANT", "CLICK"};
static const char *const QUANT_NAME[5] = {"OFF", "1/32", "1/16", "1/8", "1/4"};
static const char *const CLICK_NAME[3] = {"OFF", "REC", "ON"};

static void part_name(int p, char *t)                /* "2 DRUMS" onto t */
{
    char n[3] = {(char)('1' + p), ' ', 0};
    text_cat(t, n);
    text_cat(t, TILES[TILE_OF[set.parts[p].engine]].name);
}

static uint32_t loop_home_bars(void) { return lp_empty() ? 0u : lp.first_bars; }   /* where HOME + K2 takes LENGTH */

static void bars_text(int32_t bars, char *b)         /* "4 BARS", "1 BAR", "AUTO" */
{
    b[0] = 0;
    if (!bars) {
        text_cat(b, "AUTO");
        return;
    }
    int_text(bars, b);
    text_cat(b, bars == 1 ? " BAR" : " BARS");
}

static void loop_text(int32_t k, char *b)            /* "98 BPM", LENGTH's, "1/16", "REC" */
{
    b[0] = 0;
    if (k == 0) {
        int_text(lp.tempo, b);
        text_cat(b, " BPM");
    } else if (k == 1) {
        bars_text(lp.bars, b);
    } else {
        text_cat(b, k == 2 ? QUANT_NAME[lp.quant] : CLICK_NAME[lp.click]);
    }
}

static void loop_state_text(char *t)                 /* what the looper does: "REC 2 DRUMS", "PLAY", "STOP" */
{
    t[0] = 0;
    if (lp.state == LP_COUNT) {
        text_cat(t, "COUNT-IN");
    } else if (lp.rec) {
        text_cat(t, "REC ");
        part_name(lp.rpart, t);
    } else {
        text_cat(t, lp.state == LP_PLAY ? "PLAY" : lp_empty() ? "EMPTY" : "STOP");
    }
}


/* a screen reader that has just started listening (or asks) hears the whole screen */
static void speak(void)
{
    char t[192], o[4];
    int32_t k;
    if (ann_on() && ann_where()) {
        octave_text(o);
        t[0] = 0;
        text_cat(t, APP_NAME ". PART ");
        part_text(t);
        text_cat(t, ". ENGINE ");
        text_cat(t, TILES[TILE_OF[eng_cur()]].name);
        text_cat(t, ". SOUND ");
        text_cat(t, sound_name(set.part, eng_cur()));
        if (edited[set.part][eng_cur()])
            text_cat(t, ", EDITED");
        text_cat(t, ". OCTAVE ");
        text_cat(t, o);
        if (ui.mix) {                                /* the mixer: the chosen part's strip (SEND: its sends) */
            text_cat(t, ui.tab ? ". SEND" : ". MIX");
            for (k = ui.tab ? 4 : 0; k < (ui.tab ? 6 : 4); k++) {
                char v[12];
                text_cat(t, ". ");
                text_cat(t, MIX_LABEL[k]);
                text_cat(t, " ");
                mix_text(part_cur(), k, v, 0);
                text_cat(t, v);
            }
        }
        if (ui.fx) {                                 /* the effects' page: the tab, what its knobs hold */
            text_cat(t, ui.tab ? ". HALL" : ". DELAY");
            for (k = 0; k < 4; k++) {
                char v[12];
                text_cat(t, ". ");
                text_cat(t, FX_LABEL[ui.tab][k]);
                text_cat(t, " ");
                fx_text(ui.tab, k, v);
                text_cat(t, v);
            }
        }
        if (ui.loop && ui.tab) {                     /* the step sequencer: the bar, the steps set */
            char v[8];
            uint32_t m = step_mask(set.part, step_sel());
            text_cat(t, ". STEP. BAR ");
            int_text(ui.bar + 1, v);
            text_cat(t, v);
            text_cat(t, ". STEPS");
            for (k = 0; k < 16; k++)
                if (m >> k & 1u) {
                    text_cat(t, " ");
                    int_text(k + 1, v);
                    text_cat(t, v);
                }
        } else if (ui.loop) {                        /* the looper: what it does, what its knobs hold */
            char v[24];
            loop_state_text(v);
            text_cat(t, ". LOOP ");
            text_cat(t, v);
            for (k = 0; k < 4; k++) {
                text_cat(t, ". ");
                text_cat(t, LOOP_LABEL[k]);
                text_cat(t, " ");
                loop_text(k, v);
                text_cat(t, v);
            }
        }
        if (ui.edit) {                               /* the page and what its knobs hold */
            text_cat(t, ". EDIT ");
            text_cat(t, kl_page_title(eng_cur(), ui.page));
            for (k = 0; k < 4; k++) {
                text_cat(t, ". ");
                param_cat(t, ui.page * 4 + k);
            }
        }
        ann_say(t, 0, ANN_NOW);
    }
    ann_tick(plat_ms());
}

static void part_step(int32_t d)                     /* SEL, SELECT: the next part (1, 2, 3, 4, 1), or the one before;
                                                     * an edit page closes */
{
    char t[48] = "PART ";
    int hit;
    set.part = (uint8_t)(((set.part + d) % KL_NPART + KL_NPART) % KL_NPART);
    dirty_update();
    if (lp.rec)
        lp_ask(LC_PART, set.part);                   /* the recording goes along */
    ui.edit = 0;
    ui.redraw = 1;
    ui.hits = kl_hits(set.part, &hit);               /* its own notes: no pad lit from the other part's */
    ui.hit_until = 0;
    part_text(t);
    text_cat(t, ". ");
    text_cat(t, TILES[TILE_OF[eng_cur()]].name);
    text_cat(t, ". ");
    text_cat(t, sound_name(set.part, eng_cur()));
    announce(t);
}

static void engine_step(int32_t d)
{
    int32_t e = (engine_pos(eng_cur()) + d) % KL_NENG;
    char t[48];
    if (e < 0)
        e += KL_NENG;
    e = engine_at(e);
    part_cur()->engine = (uint8_t)e;
    set_dirty = 1;
    sound_select(set.part);
    ui.redraw = 1;
    t[0] = 0;
    text_cat(t, "ENGINE ");
    text_cat(t, TILES[TILE_OF[e]].name);
    text_cat(t, ". ");
    text_cat(t, sound_name(set.part, e));
    announce(t);
}

static void preset_step(int32_t d)
{
    int e = eng_cur();
    int32_t n = sound_count(e), p = ((int32_t)part_cur()->preset[e] + d) % n;
    if (p < 0)
        p += n;
    part_cur()->preset[e] = (uint8_t)p;
    sound_load(set.part, e);
    set_dirty = 1;
    sound_select(set.part);
    ui.redraw = 1;
    announce(sound_name(set.part, e));
}

static void page_show(uint32_t page)                 /* EDIT 1 (0) or EDIT 2 (1) of the engine playing */
{
    char t[24] = "EDIT ";
    ui.edit = 1;
    ui.mix = ui.loop = ui.fx = 0;
    ui.page = (uint8_t)page;
    ui.redraw = 1;
    text_cat(t, kl_page_title(eng_cur(), (int)page));
    announce(t);
}

static void edit_close(void)
{
    ui.edit = 0;
    ui.redraw = 1;
    announce("RACK");
}

static void param_step(int32_t k, int32_t d)         /* K1..K4 on an edit page: it plays from the next block */
{
    int e = eng_cur();
    int8_t *w = work[set.part][e];
    kl_param_t p;
    int32_t v;
    char t[32];
    kl_param(e, k, w, &p);
    v = clamp(w[k] + d, p.min, p.max);
    if (v == w[k])
        return;
    w[k] = (int8_t)v;
    edited_update(set.part, e);
    dirty_update();
    sound_select(set.part);
    ui.redraw = 1;
    t[0] = 0;
    param_cat(t, k);
    announce(t);
}

static void param_reset(int32_t k)                   /* HOME + K1..K4 on an edit page: back to its sound's value */
{
    int e = eng_cur();
    char t[32];
    if (work[set.part][e][k] == orig[set.part][e][k])
        return;
    work[set.part][e][k] = orig[set.part][e][k];
    edited_update(set.part, e);
    dirty_update();
    sound_select(set.part);
    ui.redraw = 1;
    t[0] = 0;
    param_cat(t, k);
    announce(t);
}

static void mix_said(int32_t k)                      /* a screen reader hears the knob: "LEVEL -6 dB", "DELAY 30"
                                                     * (k 4, 5: SEND's) */
{
    char t[32], v[12];
    t[0] = 0;
    text_cat(t, MIX_LABEL[k]);
    text_cat(t, " ");
    mix_text(part_cur(), k, v, 0);
    text_cat(t, v);
    announce(t);
}

static void mix_show(int on)                         /* GLO: the mixer open on MIX (another page closes), or shut
                                                     * (OCT-) */
{
    ui.mix = (uint8_t)(on != 0);
    ui.edit = ui.loop = ui.fx = ui.tab = 0;
    ui.redraw = 1;
    announce(on ? "MIX" : "RACK");
}

static void page_tab(int t)                          /* the mixer's or the effects' page: the other tab */
{
    ui.tab = (uint8_t)t;
    ui.redraw = 1;
    announce(ui.mix ? (t ? "SEND" : "MIX") : t ? "HALL" : "DELAY");
}

static void mix_changed(int32_t k)
{
    mix_update();
    dirty_update();
    ui.redraw = 1;
    mix_said(k);
}

/* K1..K4 on the mixer, the chosen part's: LEVEL a dB a detent (below -40 off), PAN a step of 50 a side, MUTE and
 * SOLO on to the right, off to the left; on SEND, K1 DELAY and K2 HALL one a detent (0..100), K3 and K4 nothing */
static void mix_step(int32_t k, int32_t d)
{
    part_set_t *ps = part_cur();
    if (ui.tab) {
        if (k >= 2)
            return;
        ps->send[k] = (uint8_t)clamp(ps->send[k] + d, 0, 100);
        k += 4;
    } else if (k == 0)
        ps->level = (int8_t)clamp(ps->level + d, LEVEL_OFF, 6);
    else if (k == 1)
        ps->pan = (int8_t)clamp(ps->pan + d, -50, 50);
    else if (k == 2)
        ps->mute = d > 0;
    else
        ps->solo = d > 0;
    mix_changed(k);
}

static void mix_reset(int32_t k)                     /* HOME + K1..K4 on the mixer: 0 dB, the middle, off; SEND: 0 */
{
    part_set_t *ps = part_cur();
    if (ui.tab) {
        if (k >= 2)
            return;
        ps->send[k] = 0;
        k += 4;
    } else if (k == 0)
        ps->level = 0;
    else if (k == 1)
        ps->pan = 0;
    else if (k == 2)
        ps->mute = 0;
    else
        ps->solo = 0;
    mix_changed(k);
}

/* ------------------------------------------------------- the effects' page --- */
static void fx_said(int32_t k)                       /* "FDBK 55%", the tab's */
{
    char t[32], v[12];
    t[0] = 0;
    text_cat(t, FX_LABEL[ui.tab][k]);
    text_cat(t, " ");
    fx_text(ui.tab, k, v);
    text_cat(t, v);
    announce(t);
}

static void fx_show(int on)                          /* FX: the effects' page open on DELAY (another page closes), or
                                                     * shut (OCT-) */
{
    ui.fx = (uint8_t)(on != 0);
    ui.edit = ui.mix = ui.loop = ui.tab = 0;
    ui.redraw = 1;
    announce(on ? "DELAY" : "RACK");
}

static void fx_changed(int32_t k)
{
    fx_apply();
    mix_update();                                    /* LEVEL: the return */
    dirty_update();
    ui.redraw = 1;
    fx_said(k);
}

static void fx_step(int32_t k, int32_t d)            /* K1..K4 on the effects' page: a step a detent */
{
    int8_t *v = &set.fx[ui.tab][k];
    int32_t n = clamp(*v + d, FX_MIN[ui.tab][k], FX_MAX[ui.tab][k]);
    if (n == *v)
        return;
    *v = (int8_t)n;
    fx_changed(k);
}

static void fx_reset(int32_t k)                      /* HOME + K1..K4: 1/8., 35 %, 70 %, 0 dB; 20 ms, 50 %, 60 %, 0 dB */
{
    set.fx[ui.tab][k] = FX_DEF[ui.tab][k];
    fx_changed(k);
}

/* -------------------------------------------------------- the looper's page --- */
static void loop_said(int32_t k)                     /* "TEMPO 98 BPM" */
{
    char t[32], v[12];
    t[0] = 0;
    text_cat(t, LOOP_LABEL[k]);
    text_cat(t, " ");
    loop_text(k, v);
    text_cat(t, v);
    announce(t);
}

static void loop_show(int on)                        /* SEQ: the looper's page open on LOOP (another closes), or
                                                     * shut (OCT-) */
{
    ui.loop = (uint8_t)(on != 0);
    ui.edit = ui.mix = ui.fx = ui.tab = 0;
    ui.held = ui.held_new = 0;
    ui.redraw = 1;
    announce(on ? "LOOP" : "RACK");
}

/* K1..K4 on the looper's page: TEMPO a BPM a detent, LENGTH a bar (AUTO only while it is empty; not while the first
 * take sets it), QUANT and CLICK a choice */
static void loop_step(int32_t k, int32_t d)
{
    if (k == 0) {
        lp.tempo = (uint8_t)clamp(lp.tempo + d, 40, 240);
    } else if (k == 1) {                            /* the render sets it (the screen follows) */
        int32_t b = clamp(lp.bars + d, lp_empty() ? 0 : 1, LP_MAXBARS);
        char t[24] = "LENGTH ", v[12];
        if (lp.first) {
            app_say("LENGTH", "AFTER TAKE");
            return;
        }
        if (b != lp.bars)
            lp_ask(LC_BARS, b);
        bars_text(b, v);
        text_cat(t, v);
        announce(t);
        return;
    } else if (k == 2) {
        lp.quant = (uint8_t)clamp(lp.quant + d, 0, 4);
    } else {
        lp.click = (uint8_t)clamp(lp.click + d, 0, 2);
    }
    ui.redraw = 1;
    loop_said(k);
}

static void loop_reset(int32_t k)                    /* HOME + K1..K4: 120 BPM, the first take's length, 1/16, REC */
{
    if (k == 0)
        lp.tempo = 120;
    else if (k == 1)
        loop_step(1, (int32_t)loop_home_bars() - lp.bars);
    else if (k == 2)
        lp.quant = 2;
    else
        lp.click = 1;
    ui.redraw = 1;
    if (k != 1)
        loop_said(k);
}

static void loop_play(void)                          /* PLAY: the loop from its start, or stopped */
{
    if (lp.state != LP_STOP) {
        lp_ask(LC_STOP, 0);
        announce("STOP");
    } else if (!lp.bars) {
        app_say("EMPTY", "REC FIRST");
    } else {
        lp_ask(LC_PLAY, 0);
        announce("PLAY");
    }
}

static void loop_rec(void)                           /* REC: the chosen part recording, or not */
{
    char t[24] = "REC ";
    if (lp.rec) {
        lp_ask(LC_UNREC, 0);
        announce("REC OFF");
        return;
    }
    lp_ask(LC_REC, set.part);
    part_name(set.part, t);
    announce(t);
}

static void loop_undo(void)                          /* HOME + REC: the last take out, or back in */
{
    char t[24] = "";
    if (lp.undo_part < 0) {
        app_say("UNDO", "NOTHING");
        return;
    }
    part_name(lp.undo_part, t);
    app_say(lp.undone ? "REDO" : "UNDO", t);
    lp_ask(LC_UNDO, 0);
}

static void loop_clear_ask(void)                     /* HOME + PLAY: the chosen part emptied, once OCT+ says so */
{
    char t[24] = "";
    part_name(set.part, t);
    if (!lp.n[set.part] && !(lp.rec && lp.rpart == set.part)) {
        app_say("EMPTY", t);
        return;
    }
    text_cat(t, "? OCT+");
    app_say("CLEAR", t);
    ui.msg_until = plat_ms() + 5000u;                /* the question stands 5 s */
    ui.ask = 1;
    ui.ask_part = set.part;
}

static void loop_answer(int yes)                     /* OCT+ or OCT- while the question stands */
{
    char t[24] = "";
    part_name(ui.ask_part, t);
    ui.ask = 0;
    if (yes)
        lp_ask(LC_CLEAR, ui.ask_part);
    app_say(yes ? "CLEARED" : "KEPT", t);
}

/* the header's looper: the bar and beat while it plays ("3.2"), the beats left while it counts in ("3"); 0 = none,
 * 1 = playing, 2 = recording */
static int loop_ind(char *t)
{
    uint32_t pos = lp.pos16 >> 16;
    char b[4];
    t[0] = 0;
    if (lp.state == LP_COUNT) {
        int_text((int32_t)(4u - (lp.count16 >> 16) / LP_PPQN), t);
        return 2;
    }
    if (lp.state != LP_PLAY)
        return 0;
    int_text((int32_t)(pos / LP_BAR + 1u), t);
    text_cat(t, ".");
    int_text((int32_t)(pos % LP_BAR / LP_PPQN + 1u), b);
    text_cat(t, b);
    return lp.rec ? 2 : 1;
}

static uint32_t loop_sig(void)                       /* what the looper did, for the screen to follow */
{
    uint32_t h = lp.state | (uint32_t)lp.rec << 2 | (uint32_t)lp.rpart << 3 | (uint32_t)lp.bars << 6 |
                 (uint32_t)lp.first << 11 | (uint32_t)lp.undone << 12;
    int p;
    for (p = 0; p < KL_NPART; p++)
        h = h * 31u + lp.n[p];
    return h;
}

/* ---------------------------------------------------- the step sequencer's tab --- */
/* STEP: one bar of the loop as 16 steps a part; the white keys are the chosen part's steps (DRUMS: those of one
 * instrument, K2). A step is set as its key goes down, cleared as it comes up (unless a knob turned meanwhile); a key
 * held, K2..K4 change its notes (the render does it: LC_SSET, LC_SCLR, LC_SEDIT) */
static const uint8_t WHITE_KEY[16] = PLAT_WHITE_KEYS;
static const uint16_t LEN_TICKS[9] = {24, 48, 96, 144, 192, 288, 384, 576, 768};
static const char *const LEN_NAME[9] = {"1/32", "1/16", "1/8", "3/16", "1/4", "3/8", "1/2", "3/4", "1/1"};
#define STEP_GAP 4                                   /* a step's note ends this many ticks before its length: the
                                                     * next one starts anew (longer, ACID slides) */
static struct {
    uint8_t note[KL_NPART], inst[KL_NPART], vel[KL_NPART], len[KL_NPART];   /* a new step's: its note (DRUMS: the
                                                     * instrument's semitone), velocity, LEN_TICKS' index */
} stp;

static void step_init(void)
{
    int p;
    for (p = 0; p < KL_NPART; p++) {
        stp.note[p] = 60;                            /* C4 */
        stp.inst[p] = 0;                             /* BD */
        stp.vel[p] = 100;
        stp.len[p] = 1;                              /* 1/16 */
    }
}

static int step_home_note(void) { return clamp(60 + 12 * part_cur()->octave, 24, 96); }   /* C in the octave */
static uint32_t step_sel(void) { return eng_cur() == KL_DRUMS ? stp.inst[set.part] : LP_ANY; }
static uint32_t step_bars(void) { return lp.bars ? lp.bars : 1u; }
static uint32_t step_tick(int s) { return (uint32_t)ui.bar * LP_BAR + (uint32_t)s * LP_STEP; }

static int step_of_key(uint32_t k)                   /* the step a key is (0..15), -1 a black key */
{
    int s;
    for (s = 0; s < 16; s++)
        if (WHITE_KEY[s] == k)
            return s;
    return -1;
}

static uint32_t step_mask(int p, uint32_t sel)       /* the steps of the bar shown with a note (sel: LP_ANY or one
                                                     * semitone) */
{
    uint32_t len = lp_len(), m = 0, i, t0 = (uint32_t)ui.bar * LP_BAR;
    const lp_ev_t *e = lp_ev[p];
    if (!lp.bars)
        return 0;
    for (i = 0; i < lp.n[p]; i++)
        if ((e[i].vel & 0x7Fu) && e[i].tick < len && (sel == LP_ANY || e[i].note % 12u == sel)) {
            uint32_t rel = (e[i].tick + len + LP_STEP / 2u - t0) % len;
            if (rel < 16u * LP_STEP)
                m |= 1u << (rel / LP_STEP);
        }
    return m;
}

static int step_len_index(uint32_t gate)             /* the LEN nearest a gate in ticks */
{
    int i, best = 0;
    int32_t bd = 1 << 30;
    for (i = 0; i < 9; i++) {
        int32_t d = (int32_t)LEN_TICKS[i] - STEP_GAP - (int32_t)gate;
        d = d < 0 ? -d : d;
        if (d < bd) {
            bd = d;
            best = i;
        }
    }
    return best;
}

/* a step's first note (sel's): its note, velocity and LEN; 0 = the step is empty */
static int step_get(int s, int *note, int *vel, int *len)
{
    uint32_t p = set.part, i, l = lp_len(), sel = step_sel();
    for (i = 0; lp.bars && i < lp.n[p]; i++)
        if (lp_step_note(&lp_ev[p][i], step_tick(s), l, sel)) {
            int32_t j = lp_end_of((int)p, i);
            *note = lp_ev[p][i].note;
            *vel = lp_ev[p][i].vel & 0x7F;
            *len = j < 0 ? 1 : step_len_index((lp_ev[p][j].tick + l - lp_ev[p][i].tick) % l);
            return 1;
        }
    return 0;
}

static int step_first_held(void)                     /* the lowest step held, -1 none */
{
    int s;
    for (s = 0; s < 16; s++)
        if (ui.held & 1u << s)
            return s;
    return -1;
}

static void note_text(int n, char *b)                /* "C4", "F#2" (MIDI 60 = C4) */
{
    char o[4];
    b[0] = 0;
    text_cat(b, KEY_NAME[n % 12]);
    int_text(n / 12 - 1, o);
    text_cat(b, o);
}

static void inst_text(int pc, char *b)               /* DRUMS: the chosen part's kit's instrument on a semitone */
{
    b[0] = 0;
    text_cat(b, kl_drum_name(sound_base(set.part, KL_DRUMS), pc));
}

static void step_said(int s, const char *what)       /* "STEP 9 C4", "STEP 9 OFF" */
{
    char t[32] = "STEP ", v[8];
    int_text(s + 1, v);
    text_cat(t, v);
    text_cat(t, " ");
    text_cat(t, what);
    announce(t);
}

static void step_key(int s, int down)                /* a white key on STEP: its step set (down) or cleared (up) */
{
    uint32_t bit = 1u << s, sel = step_sel();
    int p = set.part, drums = eng_cur() == KL_DRUMS;
    char v[8];
    ui.redraw = 1;
    if (down) {
        ui.held |= (uint16_t)bit;
        if (step_mask(p, sel) & bit)
            return;
        if (lp.first) {
            app_say("STEP", "AFTER TAKE");
            return;
        }
        lp_ask_step(LC_SSET, p, step_tick(s), drums ? 36 + (int)sel : stp.note[p], stp.vel[p],
                    LEN_TICKS[stp.len[p]] - STEP_GAP);
        ui.held_new |= (uint16_t)bit;
        if (drums)
            inst_text((int)sel, v);
        else
            note_text(stp.note[p], v);
        step_said(s, v);
        return;
    }
    if ((ui.held & bit) && !(ui.held_new & bit) && !ui.turned) {
        lp_ask_step(LC_SCLR, p, step_tick(s), (int)sel, 0, 0);
        step_said(s, "OFF");
    }
    ui.held &= (uint16_t)~bit;
    ui.held_new &= (uint16_t)~bit;
    if (!ui.held)
        ui.turned = 0;
}

static void step_tab(int t)                          /* the looper's page: LOOP or STEP */
{
    ui.tab = (uint8_t)t;
    ui.held = ui.held_new = 0;
    ui.turned = 0;
    ui.redraw = 1;
    announce(t ? "STEP" : "LOOP");
}

/* K1..K4 on STEP: K1 the bar shown; K2 NOTE (DRUMS: the instrument), K3 VEL, K4 LEN, a new step's, or the held
 * steps' own */
static void step_knob(int32_t k, int32_t d)
{
    int p = set.part, drums = eng_cur() == KL_DRUMS, s;
    char t[24] = "", v[8];
    ui.redraw = 1;
    if (k == 0) {
        ui.bar = (uint8_t)clamp(ui.bar + d, 0, (int32_t)step_bars() - 1);
        text_cat(t, "BAR ");
        int_text(ui.bar + 1, v);
        text_cat(t, v);
        announce(t);
        return;
    }
    if (k == 1 && drums) {                           /* the instrument, held or not */
        stp.inst[p] = (uint8_t)(((stp.inst[p] + d) % 12 + 12) % 12);
        inst_text(stp.inst[p], v);
        announce(v);
        return;
    }
    if (ui.held) {
        for (s = 0; s < 16; s++) {
            int note, vel, len;
            if (!(ui.held & 1u << s) || !step_get(s, &note, &vel, &len))
                continue;
            if (k == 1)
                lp_ask_step(LC_SEDIT, p, step_tick(s), LP_ANY, SE_NOTE, clamp(note + d, 24, 96) - note);
            else if (k == 2)
                lp_ask_step(LC_SEDIT, p, step_tick(s), (int)step_sel(), SE_VEL, clamp(vel + d, 1, 127));
            else
                lp_ask_step(LC_SEDIT, p, step_tick(s), (int)step_sel(), SE_GATE,
                            LEN_TICKS[clamp(len + d, 0, 8)] - STEP_GAP);
        }
        ui.turned = 1;
        announce(k == 1 ? "NOTE" : k == 2 ? "VEL" : "LEN");
        return;
    }
    if (k == 1) {
        stp.note[p] = (uint8_t)clamp(stp.note[p] + d, 24, 96);
        note_text(stp.note[p], v);
    } else if (k == 2) {
        stp.vel[p] = (uint8_t)clamp(stp.vel[p] + d, 1, 127);
        int_text(stp.vel[p], v);
    } else {
        stp.len[p] = (uint8_t)clamp(stp.len[p] + d, 0, 8);
        v[0] = 0;
        text_cat(v, LEN_NAME[stp.len[p]]);
    }
    text_cat(t, k == 1 ? "NOTE " : k == 2 ? "VEL " : "LEN ");
    text_cat(t, v);
    announce(t);
}

/* HOME + K2..K4 on STEP: C in the part's octave (DRUMS: BD), 100, 1/16; a held step's too. K1 (a place, not a
 * value): none */
static void step_reset(int32_t k)
{
    int p = set.part, s, note, vel, len;
    if (k == 0)
        return;
    if (k == 1 && eng_cur() == KL_DRUMS) {
        step_knob(1, -(int32_t)stp.inst[p]);
        return;
    }
    if (ui.held) {
        for (s = 0; s < 16; s++)
            if ((ui.held & 1u << s) && step_get(s, &note, &vel, &len)) {
                if (k == 1)
                    lp_ask_step(LC_SEDIT, p, step_tick(s), LP_ANY, SE_NOTE, step_home_note() - note);
                else if (k == 2)
                    lp_ask_step(LC_SEDIT, p, step_tick(s), (int)step_sel(), SE_VEL, 100);
                else
                    lp_ask_step(LC_SEDIT, p, step_tick(s), (int)step_sel(), SE_GATE, LEN_TICKS[1] - STEP_GAP);
            }
        ui.turned = 1;
        ui.redraw = 1;
        return;
    }
    if (k == 1)
        step_knob(1, step_home_note() - stp.note[p]);
    else if (k == 2)
        step_knob(2, 100 - stp.vel[p]);
    else
        step_knob(3, 1 - stp.len[p]);
}

static uint32_t step_play(void)                      /* the step playing in the bar shown: its bit, 0 none */
{
    uint32_t pos = lp.pos16 >> 16;
    if (lp.state != LP_PLAY || !lp.bars || pos / LP_BAR != ui.bar)
        return 0;
    return 1u << (pos % LP_BAR / LP_STEP);
}

static uint32_t step_lights(void)                    /* STEP's keys: the steps set lit, the one playing flips */
{
    uint32_t m = step_mask(set.part, step_sel()) ^ step_play(), k = 0;
    int s;
    for (s = 0; s < 16; s++)
        if ((m | ui.held) & 1u << s)
            k |= 1u << WHITE_KEY[s];
    return k;
}

/* each edited sound becomes its engine's USER sound (two parts with the same engine edited: the chosen part's, else
 * the higher's); a part that plays that USER sound unedited plays the new one */
static void save(void)
{
    uint32_t e, k, i, users = 0;
    for (i = 1; i <= KL_NPART; i++) {                /* the chosen part last */
        uint32_t p = (set.part + i) % KL_NPART;
        for (e = 0; e < KL_NENG; e++) {
            user_t *u = &set.user[e];
            if (!edited[p][e])
                continue;
            for (k = 0; k < 8u; k++)
                u->v[k] = orig[p][e][k] = work[p][e][k];
            u->base = (uint8_t)sound_base((int)p, (int)e);
            u->saved = 1;
            set.parts[p].preset[e] = (uint8_t)kl_presets((int)e);
            edited[p][e] = 0;
            users++;
        }
    }
    for (i = 0; i < KL_NPART; i++)
        for (e = 0; e < KL_NENG; e++)
            if (is_user((int)i, (int)e) && !edited[i][e]) {
                sound_load((int)i, (int)e);
                if (set.parts[i].engine == e)
                    sound_select((int)i);
            }
    if (plat_store_save(OBJ_SETTINGS, &set, sizeof set) == 0) {
        set_kept = set;
        set_dirty = 0;
        app_say("SAVED", users ? "USER" : "");
    } else {
        app_say("SAVE", "FAILED");                   /* SAVE stays lit: the next one tries again */
    }
}

static void octave_step(int32_t d)
{
    char t[16] = "OCTAVE ", o[4];
    int32_t v = clamp(part_cur()->octave + d, -2, 2);
    if (v == part_cur()->octave)
        return;
    part_cur()->octave = (int8_t)v;
    set_dirty = 1;
    ui.redraw = 1;
    octave_text(o);
    text_cat(t, o);
    announce(t);
}

static void input(void)
{
    uint32_t keys, btn = plat_buttons(), k;
    int32_t e;
    touch_poll();                                    /* the sensed keys' presses and velocities */
    keys = touch_keys();
    for (k = 0; k < NKEYS; k++) {                    /* key 0 is F3 (MIDI 53) */
        uint32_t bit = 1u << k;
        if ((keys & bit) && !(ui.keys & bit) && ui.loop && ui.tab) {   /* STEP: a white key is a step, a black one
                                                                       * nothing */
            ui.kstep |= bit;
            if (step_of_key(k) >= 0)
                step_key(step_of_key(k), 1);
        } else if (!(keys & bit) && (ui.kstep & bit)) {
            ui.kstep &= ~bit;
            if (step_of_key(k) >= 0)
                step_key(step_of_key(k), 0);
        } else if ((keys & bit) && !(ui.keys & bit)) {
            int32_t note = clamp(53 + (int32_t)k + 12 * part_cur()->octave, 0, 127);
            uint32_t v = touch_vel(k);
            if (eng_cur() == KL_ACID)                /* its accent is ENV held, as Open303's: velocity 100 or more */
                v = btn & 1u << B_ENV ? 127u : 64u;
            ui.knote[k] = (int8_t)note;
            ui.kpart[k] = set.part;
            kl_key(set.part, 1, note, (int)v);
            lp_key(set.part, 1, note, (int)v);
            plat_midi_send(0x09u | (0x90u | set.part) << 8 | (uint32_t)note << 16 | v << 24, TO_USB);   /* its
                                                                                                         * channel */
        } else if (!(keys & bit) && (ui.keys & bit)) {
            int32_t note = ui.knote[k];
            kl_key(ui.kpart[k], 0, note, 0);
            lp_key(ui.kpart[k], 0, note, 0);
            plat_midi_send(0x08u | (0x80u | ui.kpart[k]) << 8 | (uint32_t)note << 16, TO_USB);
        }
    }
    for (k = 0; k < NB; k++) {
        uint32_t bit = 1u << k;
        if (!(btn & bit) || (ui.btn & bit))
            continue;
        if (ui.ask && (k == B_OCTUP || k == B_OCTDN))
            loop_answer(k == B_OCTUP);               /* HOME + PLAY's question */
        else if (k == B_OCTDN && ui.edit)
            edit_close();                            /* back, as on Felucca's pages */
        else if (k == B_OCTDN && ui.loop)
            loop_show(0);                            /* back */
        else if ((k == B_SEQ || k == B_OCTUP) && ui.loop)
            step_tab(!ui.tab);                       /* the other tab, as EDIT and OCT+ on the edit pages */
        else if (k == B_SEQ)
            loop_show(1);
        else if (k == B_OCTDN && ui.mix)
            mix_show(0);
        else if (k == B_OCTDN && ui.fx)
            fx_show(0);
        else if ((k == B_GLO || k == B_OCTUP) && ui.mix)
            page_tab(!ui.tab);                       /* MIX and SEND, as SEQ on the looper's page */
        else if ((k == B_FX || k == B_OCTUP) && ui.fx)
            page_tab(!ui.tab);                       /* DELAY and HALL */
        else if (k == B_GLO)
            mix_show(1);
        else if (k == B_FX)
            fx_show(1);
        else if (k == B_PLAY && (btn & 1u << B_HOME))
            loop_clear_ask();
        else if (k == B_PLAY)
            loop_play();                             /* on every page */
        else if (k == B_REC && (btn & 1u << B_HOME))
            loop_undo();
        else if (k == B_REC)
            loop_rec();
        else if (k == B_OCTDN)
            octave_step(-1);
        else if (k == B_EDIT || (k == B_OCTUP && ui.edit))
            page_show(ui.edit ? ui.page ^ 1u : 0u);  /* the edit pages, then the other one */
        else if (k == B_OCTUP)
            octave_step(1);
        else if (k == B_SAVE)
            save();
        else if (k == B_SEL)
            part_step(1);
        else if (k == B_ENV && eng_cur() == KL_ACID)
            announce("ACCENT");                      /* held: the keys pressed now are accented */
        else if (k == B_HOME && (ui.edit || ui.mix || ui.loop || ui.fx))
            app_say("RESET", "TURN K1-K4");          /* held: a knob turned goes back to its sound's value */
        else if (k == B_HOME)
            app_say("HOME", "+ REC UNDO");           /* held: REC takes the last take back, PLAY empties a part */
        else
            app_say(BTN_NAME[k], "SOON");            /* the pages behind the other buttons come later */
    }
    for (k = 0; k < NE; k++) {                       /* every one read, so none piles up */
        if ((e = plat_enc((int)k)) == 0)
            continue;
        if (k == EN_PRESET)
            preset_step(e);
        else if (k == EN_SELECT)
            part_step(e);                            /* the part, as SEL: to the right the next */
        else if (ui.loop && ui.tab && k >= EN_K1 && (btn & 1u << B_HOME))
            step_reset((int32_t)(k - EN_K1));
        else if (ui.loop && ui.tab && k >= EN_K1)
            step_knob((int32_t)(k - EN_K1), e);
        else if (ui.loop && k >= EN_K1 && (btn & 1u << B_HOME))
            loop_reset((int32_t)(k - EN_K1));
        else if (ui.loop && k >= EN_K1)
            loop_step((int32_t)(k - EN_K1), e);
        else if (ui.fx && k >= EN_K1 && (btn & 1u << B_HOME))
            fx_reset((int32_t)(k - EN_K1));
        else if (ui.fx && k >= EN_K1)
            fx_step((int32_t)(k - EN_K1), e);
        else if (ui.mix && k >= EN_K1 && (btn & 1u << B_HOME))
            mix_reset((int32_t)(k - EN_K1));
        else if (ui.mix && k >= EN_K1)
            mix_step((int32_t)(k - EN_K1), e);
        else if (ui.edit && k >= EN_K1 && (btn & 1u << B_HOME))
            param_reset(ui.page * 4 + (int32_t)(k - EN_K1));
        else if (ui.edit && k >= EN_K1)
            param_step(ui.page * 4 + (int32_t)(k - EN_K1), e);
        else if (k == EN_K1)
            engine_step(e);
    }
    if (keys != ui.keys || btn != ui.btn)
        ui.redraw = 1;
    ui.keys = keys;
    ui.btn = btn;
    plat_glow(1);                                    /* the unlit buttons glow, readable in the dark */
    plat_play_red(lp.rec);                           /* PLAY green while it plays, red while it records */
    plat_leds(btn | (set_dirty ? 1u << B_SAVE : 0u) | (ui.mix ? 1u << B_GLO : 0u) | (ui.loop ? 1u << B_SEQ : 0u) |
                  (ui.fx ? 1u << B_FX : 0u) | (lp.state != LP_STOP ? 1u << B_PLAY : 0u) | (lp.rec ? 1u << B_REC : 0u),
              ui.loop && ui.tab ? step_lights() : keys);   /* STEP: the steps set, a running light */
}

void app_input_only(void) { input(); }

/* ------------------------------------------------------------- the glyph --- */
static uint16_t mix565(uint16_t a, uint16_t b, uint32_t t)   /* a over b, t / 256 of a */
{
    uint32_t ar = a >> 11, ag = (a >> 5) & 63u, ab = a & 31u, br = b >> 11, bg = (b >> 5) & 63u, bb = b & 31u;
    return (uint16_t)((((ar * t + br * (256u - t)) >> 8) << 11) | (((ag * t + bg * (256u - t)) >> 8) << 5) |
                      ((ab * t + bb * (256u - t)) >> 8));
}

/* the eight spokes, from the top clockwise (x, y) * 256 */
static const int16_t GDIR[8][2] = {{0, -256}, {181, -181}, {256, 0}, {181, 181},
                                   {0, 256}, {-181, 181}, {-256, 0}, {-181, -181}};

/* a sound's glyph around (cx, cy), r the longest spoke: the shape filled at 3/8 strength, its edge in col */
static void glyph_draw(int32_t cx, int32_t cy, int32_t r, const uint8_t sh[8], uint16_t col, uint16_t bg)
{
    int32_t px[8], py[8], y, i, miny = 10000, maxy = -10000;
    uint16_t fill = mix565(col, bg, 96);
    for (i = 0; i < 8; i++) {                        /* a spoke is a quarter of r at 0, all of r at 255 */
        int32_t rk = r * (64 + (192 * (int32_t)sh[i]) / 255) / 256;
        px[i] = cx + GDIR[i][0] * rk / 256;
        py[i] = cy + GDIR[i][1] * rk / 256;
        if (py[i] < miny)
            miny = py[i];
        if (py[i] > maxy)
            maxy = py[i];
    }
    for (y = miny; y <= maxy; y++) {                 /* the fill, row by row: each edge's crossing */
        int32_t xs[8], n = 0, a, b;
        for (i = 0; i < 8; i++) {
            int32_t j = (i + 1) & 7, y0 = py[i], y1 = py[j];
            if ((y0 <= y && y1 > y) || (y1 <= y && y0 > y))
                xs[n++] = px[i] + (px[j] - px[i]) * (y - y0) / (y1 - y0);
        }
        for (a = 1; a < n; a++)
            for (b = a; b > 0 && xs[b - 1] > xs[b]; b--) {
                int32_t t = xs[b];
                xs[b] = xs[b - 1];
                xs[b - 1] = t;
            }
        for (a = 0; a + 1 < n; a += 2)
            cv_rect(xs[a], y, xs[a + 1] - xs[a] + 1, 1, fill);
    }
    for (i = 0; i < 8; i++) {                        /* the edge, two pixels wide */
        int32_t j = (i + 1) & 7;
        cv_line(px[i], py[i], px[j], py[j], col);
        cv_line(px[i] + 1, py[i], px[j] + 1, py[j], col);
    }
    cv_rect(cx - 1, cy - 1, 3, 3, col);
}

/* --------------------------------------------------------------- drawing --- */
static void box_edge(int32_t x, int32_t y, int32_t w, int32_t h, int32_t t, uint16_t c)
{
    cv_rect(x, y, w, t, c);
    cv_rect(x, y + h - t, w, t, c);
    cv_rect(x, y, t, h, c);
    cv_rect(x + w - t, y, t, h, c);
}

static void text_fit(int32_t x, int32_t y, int32_t w, const felucca_font_t *f, const char *s, uint16_t c)
{
    char t[24];
    uint32_t n = 0;
    while (s[n] && n < sizeof t - 1u) {
        t[n] = s[n];
        n++;
    }
    t[n] = 0;
    while (n && text_w(f, t) > w)                    /* cut until it fits */
        t[--n] = 0;
    cv_text(x, y, f, t, c);
}

static void draw_battery(int32_t x, int32_t y)
{
    int b = plat_battery(), i;
    box_edge(x, y, 18, 10, 1, K_MUTED);
    cv_rect(x + 18, y + 3, 2, 4, K_MUTED);
    if (b == 4)                                      /* a USB host powers the FM-1 */
        cv_rect(x + 3, y + 3, 12, 4, K_GREEN);
    else
        for (i = 0; i < b && i < 3; i++)
            cv_rect(x + 3 + i * 4, y + 3, 3, 4, K_TEXT);
}

static int acc_on(void) { return eng_cur() == KL_ACID && (ui.btn & 1u << B_ENV); }

static void disc(int32_t cx, int32_t cy, int32_t r, uint16_t c);

static void draw_ind(int32_t right)                  /* the looper on the header's right, up to right: a red dot
                                                     * recording (a green triangle playing), the bar and beat */
{
    char t[8];
    int k = loop_ind(t), i;
    int32_t x;
    if (!k)
        return;
    x = right - text_w(&FONT_XS, t) - 9;
    if (k == 2)
        disc(x + 3, 11, 3, K_REC);
    else
        for (i = 0; i < 7; i++)
            cv_rect(x + 1, 8 + i, i < 4 ? i + 1 : 7 - i, 1, K_GREEN);
    cv_text(x + 9, 4, &FONT_XS, t, K_TEXT);
}

static void draw_header(void)
{
    const tile_t *tl = &TILES[TILE_OF[eng_cur()]];
    char o[8] = "OCT ", t[4], chip[12] = "", it[8];
    int msg = plat_ms() < ui.msg_until;
    int32_t iw = !msg && loop_ind(it) ? text_w(&FONT_XS, it) + 13 : 0;   /* the looper's room on the right */
    cv_begin(240, 22, K_HEAD);
    cv_rect(0, 21, 240, 1, K_LINE);
    if (msg) {
        int32_t x = cv_text(8, 3, &FONT_B, ui.msg[0], K_TEXT);
        cv_text(x + 6, 3, &FONT_B, ui.msg[1], tl->col);
    } else if (ui.mix || ui.loop || ui.fx) {         /* the mixer, the looper, the effects: all four parts */
        const char *c = ui.mix ? "MIX" : ui.fx ? "FX" : ui.tab ? "STEP" : "LOOP";
        char st[24] = "";
        int32_t w = text_w(&FONT_XS, c) + 8;
        if (ui.mix) {
            text_cat(st, set.part < 3 ? "PARTS 1-3" : "PARTS 4-6");   /* the page: the chosen part's three */
        } else if (ui.fx) {
            text_cat(st, "DELAY + HALL");
        } else if (ui.tab) {                         /* STEP: the part whose steps the keys are (DRUMS: and the
                                                     * instrument) */
            part_name(set.part, st);
            if (eng_cur() == KL_DRUMS) {
                char v[8];
                inst_text(stp.inst[set.part], v);
                text_cat(st, " - ");
                text_cat(st, v);
            }
        } else {
            loop_state_text(st);
        }
        cv_rect(6, 4, w, 15, K_TEXT);
        cv_text(10, 4, &FONT_XS, c, K_GROUND);
        text_fit(6 + w + 8, 3, 192 - w - iw, &FONT_B, st, K_TEXT);
        draw_ind(210);
        draw_battery(214, 6);
        cv_blit(0, 0);
        return;
    } else {
        uint8_t sh[8];
        char name[24];
        int32_t w;
        part_text(chip);                             /* "1 ACID": the part and its engine */
        text_cat(chip, " ");
        text_cat(chip, tl->name);
        w = text_w(&FONT_XS, chip) + 8;
        cv_rect(6, 4, w, 15, tl->col);
        cv_text(10, 4, &FONT_XS, chip, K_GROUND);
        kl_shape(eng_cur(), work[set.part][eng_cur()], sh);   /* the sound as it plays: an edit shows at once */
        glyph_draw(6 + w + 11, 11, 8, sh, tl->col, K_HEAD);
        sound_title(set.part, eng_cur(), name);
        text_fit(6 + w + 23, 3, (acc_on() ? 124 : 150) - w - iw, &FONT_B, name, K_TEXT);
    }
    octave_text(t);
    text_cat(o, t);
    cv_text(206 - iw - text_w(&FONT_XS, o), 4, &FONT_XS, o, K_MUTED);
    if (acc_on()) {                                  /* ACID's accent held */
        int32_t x = 202 - iw - text_w(&FONT_XS, o) - text_w(&FONT_T, "ACC") - 6;
        box_edge(x, 5, text_w(&FONT_T, "ACC") + 6, 13, 1, tl->col);
        cv_text(x + 3, 6, &FONT_T, "ACC", tl->col);
    }
    if (iw)
        draw_ind(210);
    draw_battery(214, 6);
    cv_blit(0, 0);
}

static void disc(int32_t cx, int32_t cy, int32_t r, uint16_t c)   /* a filled circle */
{
    int32_t dy;
    for (dy = -r; dy <= r; dy++) {
        int32_t dx = 0;
        while ((dx + 1) * (dx + 1) + dy * dy <= r * r + r)
            dx++;
        cv_rect(cx - dx, cy + dy, 2 * dx + 1, 1, c);
    }
}

/* a tile's part numbers, top right: each part that plays its engine, the chosen one filled in the engine's colour,
 * the others ringed */
static void draw_badges(const tile_t *tl, int32_t x, int32_t y, int32_t w, uint16_t bg)
{
    int32_t p, cx = x + w - 9;
    for (p = KL_NPART - 1; p >= 0; p--) {
        char n[2] = {(char)('1' + p), 0};
        int32_t tx;
        if (set.parts[p].engine != tl->eng || cx < x + 9)   /* (six on one tile: as many as fit) */
            continue;
        tx = cx - text_w(&FONT_T, n) / 2;
        if (p == set.part) {
            disc(cx, y + 9, 6, tl->col);
            cv_text(tx, y + 3, &FONT_T, n, K_GROUND);
        } else {
            disc(cx, y + 9, 6, K_MUTED);
            disc(cx, y + 9, 5, bg);
            cv_text(tx, y + 3, &FONT_T, n, K_TEXT);
        }
        cx -= 15;
    }
}

static int tile_part(int e)                          /* whose sound a tile shows: the chosen part's if it plays the
                                                     * engine, else the first part that does, else the chosen one's
                                                     * own for it */
{
    int p;
    if (eng_cur() == e)
        return set.part;
    for (p = 0; p < KL_NPART; p++)
        if (set.parts[p].engine == e)
            return p;
    return set.part;
}

static void draw_row(int32_t row)                    /* three tiles, 93 rows high */
{
    int32_t c;
    cv_begin(240, 93, K_GROUND);
    for (c = 0; c < 3; c++) {
        const tile_t *tl = &TILES[row * 3 + c];
        int32_t x = 6 + c * 77, y = 2, w = 73, h = 89;
        int sel = tl->eng >= 0 && tl->eng == eng_cur();
        uint16_t bg = sel ? K_SURF2 : K_SURF;
        cv_rect(x, y, w, h, bg);
        if (sel)
            box_edge(x, y, w, h, 2, tl->col);
        if (tl->eng >= 0) {
            uint8_t sh[8];
            char name[24];
            int tp = tile_part(tl->eng);
            kl_shape(tl->eng, work[tp][tl->eng], sh);
            glyph_draw(x + 23, y + 21, 18, sh, tl->col, bg);
            cv_text(x + 6, y + 40, &FONT_B, tl->name, sel ? tl->col : K_TEXT);
            cv_text(x + 6, y + 57, &FONT_T, tl->sub, K_MUTED);
            sound_title(tp, tl->eng, name);
            text_fit(x + 6, y + 71, w - 10, &FONT_T, name, K_TEXT);
            draw_badges(tl, x, y, w, bg);
        } else {
            cv_rect(x + 6, y + 20, 30, 3, mix565(tl->col, bg, 110));
            cv_text(x + 6, y + 40, &FONT_B, tl->name, K_MUTED);
            cv_text(x + 6, y + 57, &FONT_T, tl->sub, K_MUTED);
            cv_text(x + 6, y + 71, &FONT_T, "SOON", mix565(tl->col, bg, 200));
        }
    }
    cv_blit(0, 22 + row * 93);
}

static void footer_rack(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    char t[8];
    int e = eng_cur();
    int32_t n = sound_count(e), p = part_cur()->preset[e];
    uint32_t pct = plat_master() * 100u / 4096u;
    static const char *const LAB[4] = {"K1 ENGINE", "PRESET", "OCTAVE", "MASTER"};
    int32_t i;
    for (i = 0; i < 4; i++) {
        lab[i][0] = v[i][0] = 0;
        mark[i] = -1;
        text_cat(lab[i], LAB[i]);
    }
    text_cat(v[0], TILES[TILE_OF[e]].name);
    frac[0] = engine_pos(e) * 256 / (KL_NENG - 1);
    if (is_user(set.part, e)) {
        text_cat(v[1], "USER");
    } else {
        two_digits(p + 1, v[1]);
        text_cat(v[1], "/");
        two_digits(kl_presets(e), t);
        text_cat(v[1], t);
    }
    frac[1] = n > 1 ? p * 256 / (n - 1) : 256;
    octave_text(v[2]);
    frac[2] = (part_cur()->octave + 2) * 64;
    int_text((int32_t)pct, v[3]);
    text_cat(v[3], "%");
    frac[3] = (int32_t)pct * 256 / 100;
}

/* K1..K4: the page's parameters, each marked where its sound has it while it is elsewhere */
static void footer_edit(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    int e = eng_cur();
    int32_t i;
    for (i = 0; i < 4; i++) {
        int32_t k = ui.page * 4 + i, x = work[set.part][e][k], o = orig[set.part][e][k];
        kl_param_t d;
        kl_param(e, k, work[set.part][e], &d);
        lab[i][0] = 'K';
        lab[i][1] = (char)('1' + i);
        lab[i][2] = ' ';
        lab[i][3] = 0;
        text_cat(lab[i], d.label);
        value_text(&d, x, v[i]);
        frac[i] = d.max > d.min ? (x - d.min) * 256 / (d.max - d.min) : 0;
        mark[i] = o != x && d.max > d.min ? (o - d.min) * 256 / (d.max - d.min) : -1;
    }
}

/* K1..K4: the chosen part's strip; LEVEL and PAN marked at 0 dB and the middle (where HOME takes them) while they are
 * elsewhere, MUTE and SOLO not (OFF or ON says it). SEND: K1 DELAY, K2 HALL, marked at 0 while elsewhere; K3 and K4
 * free ("-") */
static void footer_mix(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    const part_set_t *ps = part_cur();
    int32_t i;
    if (ui.tab) {
        for (i = 0; i < 4; i++) {
            lab[i][0] = 'K';
            lab[i][1] = (char)('1' + i);
            lab[i][2] = ' ';
            lab[i][3] = 0;
            v[i][0] = '-';
            v[i][1] = 0;
            frac[i] = 0;
            mark[i] = -1;
        }
        for (i = 0; i < 2; i++) {
            text_cat(lab[i], MIX_LABEL[4 + i]);
            mix_text(ps, 4 + i, v[i], 0);
            frac[i] = ps->send[i] * 256 / 100;
            mark[i] = ps->send[i] ? 0 : -1;
        }
        return;
    }
    for (i = 0; i < 4; i++) {
        lab[i][0] = 'K';
        lab[i][1] = (char)('1' + i);
        lab[i][2] = ' ';
        lab[i][3] = 0;
        text_cat(lab[i], MIX_LABEL[i]);
        mix_text(ps, i, v[i], 0);
        mark[i] = -1;
    }
    frac[0] = (ps->level - LEVEL_OFF) * 256 / (6 - LEVEL_OFF);
    frac[1] = (ps->pan + 50) * 256 / 100;
    if (ps->level)
        mark[0] = -LEVEL_OFF * 256 / (6 - LEVEL_OFF);
    if (ps->pan)
        mark[1] = 128;
    frac[2] = ps->mute ? 256 : 0;
    frac[3] = ps->solo ? 256 : 0;
}

/* K1..K4: the effects' tab's, each marked where HOME takes it while it is elsewhere */
static void footer_fx(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    int e = ui.tab;
    int32_t i;
    for (i = 0; i < 4; i++) {
        int32_t lo = FX_MIN[e][i], span = FX_MAX[e][i] - lo, x = set.fx[e][i], h = FX_DEF[e][i];
        lab[i][0] = 'K';
        lab[i][1] = (char)('1' + i);
        lab[i][2] = ' ';
        lab[i][3] = 0;
        text_cat(lab[i], FX_LABEL[e][i]);
        fx_text(e, i, v[i]);
        frac[i] = (x - lo) * 256 / span;
        mark[i] = x != h ? (h - lo) * 256 / span : -1;
    }
}

/* K1..K4: TEMPO, LENGTH, QUANT, CLICK, each marked where HOME takes it (120 BPM, the first take's length, 1/16,
 * REC) while it is elsewhere */
static void footer_loop(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    int32_t i, home = (int32_t)loop_home_bars();
    for (i = 0; i < 4; i++) {
        lab[i][0] = 'K';
        lab[i][1] = (char)('1' + i);
        lab[i][2] = ' ';
        lab[i][3] = 0;
        text_cat(lab[i], LOOP_LABEL[i]);
        loop_text(i, v[i]);
    }
    frac[0] = (lp.tempo - 40) * 256 / 200;
    frac[1] = lp.bars * 256 / LP_MAXBARS;
    frac[2] = lp.quant * 64;
    frac[3] = lp.click * 128;
    mark[0] = lp.tempo != 120 ? 80 * 256 / 200 : -1;
    mark[1] = lp.bars != home ? home * 256 / LP_MAXBARS : -1;
    mark[2] = lp.quant != 2 ? 128 : -1;
    mark[3] = lp.click != 1 ? 128 : -1;
}

/* K1 BAR, K2 NOTE (DRUMS: INST), K3 VEL, K4 LEN: a new step's, or the held step's; marked where HOME takes them (C in
 * the part's octave or BD, 100, 1/16) */
static void footer_step(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    static const char *const LAB[4] = {"K1 BAR", "K2 NOTE", "K3 VEL", "K4 LEN"};
    int p = set.part, drums = eng_cur() == KL_DRUMS, s0 = step_first_held();
    int note = stp.note[p], vel = stp.vel[p], len = stp.len[p], home = step_home_note();
    int32_t i, bars = (int32_t)step_bars();
    char t[8];
    if (s0 >= 0)
        step_get(s0, &note, &vel, &len);
    for (i = 0; i < 4; i++) {
        lab[i][0] = v[i][0] = 0;
        text_cat(lab[i], i == 1 && drums ? "K2 INST" : LAB[i]);
    }
    int_text(ui.bar + 1, v[0]);
    text_cat(v[0], "/");
    int_text(bars, t);
    text_cat(v[0], t);
    frac[0] = bars > 1 ? ui.bar * 256 / (bars - 1) : 256;
    mark[0] = -1;
    if (drums) {
        inst_text(stp.inst[p], v[1]);
        frac[1] = stp.inst[p] * 256 / 11;
        mark[1] = stp.inst[p] ? 0 : -1;
    } else {
        note_text(note, v[1]);
        frac[1] = (note - 24) * 256 / 72;
        mark[1] = note != home ? (home - 24) * 256 / 72 : -1;
    }
    int_text(vel, v[2]);
    frac[2] = vel * 256 / 127;
    mark[2] = vel != 100 ? 100 * 256 / 127 : -1;
    text_cat(v[3], LEN_NAME[len]);
    frac[3] = len * 32;
    mark[3] = len != 1 ? 32 : -1;
}

static void draw_footer(void)                        /* what the controls hold, each with a bar of where it is */
{
    const tile_t *tl = &TILES[TILE_OF[eng_cur()]];
    char lab[4][16], v[4][12];
    int32_t frac[4], mark[4], i;
    if (ui.mix)
        footer_mix(lab, v, frac, mark);
    else if (ui.fx)
        footer_fx(lab, v, frac, mark);
    else if (ui.loop && ui.tab)
        footer_step(lab, v, frac, mark);
    else if (ui.loop)
        footer_loop(lab, v, frac, mark);
    else if (ui.edit)
        footer_edit(lab, v, frac, mark);
    else
        footer_rack(lab, v, frac, mark);
    cv_begin(240, 32, K_HEAD);
    cv_rect(0, 0, 240, 1, K_LINE);
    for (i = 0; i < 4; i++) {
        int32_t x = i * 60;
        if (i)
            cv_rect(x, 1, 1, 31, K_LINE);
        text_fit(x + 6, 3, 52, &FONT_T, lab[i], K_MUTED);
        text_fit(x + 6, 13, 52, &FONT_XS, v[i], K_TEXT);
        cv_rect(x + 6, 28, 48, 2, K_LINE);
        cv_rect(x + 6, 28, 48 * clamp(frac[i], 0, 256) / 256, 2, tl->col);
        if (mark[i] >= 0)                            /* its sound's value */
            cv_rect(x + 5 + 48 * clamp(mark[i], 0, 256) / 256, 25, 2, 7, K_TEXT);
    }
    cv_blit(0, 208);
}

/* ------------------------------------------------------------ edit pages --- */
#define EDIT_CX 120                                  /* the live glyph's centre on the screen */
#define EDIT_CY 124

static void ring_draw(int32_t cx, int32_t cy, int32_t r, uint16_t c)   /* a circle, one pixel wide */
{
    int32_t x = r, y = 0, err = 1 - r;
    while (x >= y) {
        cv_pset(cx + x, cy + y, c);
        cv_pset(cx - x, cy + y, c);
        cv_pset(cx + x, cy - y, c);
        cv_pset(cx - x, cy - y, c);
        cv_pset(cx + y, cy + x, c);
        cv_pset(cx - y, cy + x, c);
        cv_pset(cx + y, cy - x, c);
        cv_pset(cx - y, cy - x, c);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

/* DRUMS: one octave as the keys lie, each key with its instrument; those the page's knobs change are outlined in the
 * engine's colour, the one just played is filled (the same on every octave) */
static void draw_pad(int32_t x, int32_t y, int32_t w, int32_t h, int pc, uint16_t col, int lit)
{
    const char *name = kl_drum_name(sound_base(set.part, KL_DRUMS), pc);
    int page = (kl_drum_pages(pc) >> ui.page) & 1;
    cv_rect(x, y, w, h, lit ? col : K_SURF2);
    if (!lit)
        box_edge(x, y, w, h, page ? 2 : 1, page ? col : K_LINE);
    cv_text(x + (w - text_w(&FONT_B, name)) / 2, y + 7, &FONT_B, name, lit ? K_GROUND : page ? col : K_TEXT);
    cv_text(x + (w - text_w(&FONT_T, KEY_NAME[pc])) / 2, y + h - 15, &FONT_T, KEY_NAME[pc], lit ? K_GROUND : K_MUTED);
}

static void draw_pads(int32_t y0, uint16_t col)
{
    static const uint8_t WHITE[7] = {0, 2, 4, 5, 7, 9, 11}, BLACK[5][2] = {{0, 1}, {1, 3}, {3, 6}, {4, 8}, {5, 10}};
    int lit = ui.hit_until ? ui.hit_pc : -1;
    int32_t i;
    cv_rect(8, 44 - y0, 224, 162, K_SURF);
    cv_text(14, 47 - y0, &FONT_T, "KEYS", K_MUTED);
    cv_text(226 - text_w(&FONT_T, "EVERY OCTAVE"), 47 - y0, &FONT_T, "EVERY OCTAVE", K_MUTED);
    for (i = 0; i < 5; i++)                          /* the black keys: on the line after their white one */
        draw_pad(13 + (BLACK[i][0] + 1) * 31 - 14, 61 - y0, 24, 57, BLACK[i][1], col, lit == BLACK[i][1]);
    for (i = 0; i < 7; i++)
        draw_pad(13 + i * 31, 123 - y0, 28, 79, WHITE[i], col, lit == WHITE[i]);
}

/* the page's rows y0 .. y0 + 92 of the screen (it is drawn in two strips): the tabs (the two pages' titles, this
 * one lit), and the sound's glyph large in its ring, each spoke named, the four on the knobs in the engine's colour
 * (DRUMS: its pads instead) */
static void draw_edit(int32_t y0)
{
    const tile_t *tl = &TILES[TILE_OF[eng_cur()]];
    int e = eng_cur();
    int32_t i, x = 8, cy = EDIT_CY - y0;
    uint8_t sh[8];
    char t[4];
    cv_begin(240, 93, K_GROUND);
    for (i = 0; i < 2; i++) {
        const char *s = kl_page_title(e, (int)i);
        int32_t w = text_w(&FONT_XS, s);
        cv_text(x, 23 - y0, &FONT_XS, s, i == ui.page ? tl->col : K_MUTED);
        if (i == ui.page)
            cv_rect(x, 38 - y0, w, 2, tl->col);
        x += w + 14;
    }
    t[0] = (char)('1' + ui.page);
    t[1] = '/';
    t[2] = '2';
    t[3] = 0;
    cv_text(232 - text_w(&FONT_XS, t), 23 - y0, &FONT_XS, t, K_MUTED);
    cv_rect(0, 40 - y0, 240, 1, K_LINE);
    if (e == KL_DRUMS) {
        draw_pads(y0, tl->col);
        cv_blit(0, (uint32_t)y0);
        return;
    }
    cv_text(8, 46 - y0, &FONT_T, "LIVE", K_MUTED);
    kl_shape(e, work[set.part][e], sh);
    ring_draw(EDIT_CX, cy, 60, K_LINE);
    glyph_draw(EDIT_CX, cy, 56, sh, tl->col, K_GROUND);
    for (i = 0; i < 8; i++) {                        /* a name beyond each spoke's end, away from the centre */
        kl_param_t d;
        int32_t lx = EDIT_CX + GDIR[i][0] * 66 / 256, ly = cy + GDIR[i][1] * 66 / 256, w;
        kl_param(e, (int)i, work[set.part][e], &d);
        w = text_w(&FONT_XS, d.label);
        if (GDIR[i][0] < 0)
            lx -= w;
        else if (GDIR[i][0] == 0)
            lx -= w / 2;
        if (GDIR[i][1] < 0)
            ly -= FONT_XS.h;
        else if (GDIR[i][1] == 0)
            ly -= FONT_XS.h / 2;
        cv_text(lx, ly, &FONT_XS, d.label, i / 4 == ui.page ? tl->col : K_MUTED);
    }
    cv_blit(0, (uint32_t)y0);
}

static const char *const TABS_LOOP[2] = {"LOOP", "STEP"};   /* the pages' tabs (draw_tabs, below) */
static const char *const TABS_MIX[2] = {"MIX", "SEND"};
static const char *const TABS_FX[2] = {"DELAY", "HALL"};
static void draw_tabs(int32_t y0, const char *const TAB[2], const char *right);

/* ---------------------------------------------------------------- the mixer --- */
#define FD_TOP 73                                    /* the faders and meters: from this row, this high */
#define FD_H 62

static int32_t meter_px(int32_t pk)                  /* a peak (24 bits) as a meter's height: -48 .. 0 dBFS in FD_H
                                                     * px, 6 dB a bit of the peak, its next three bits between */
{
    int32_t msb = 23, pos;
    if (pk <= 0)
        return 0;
    while (msb > 0 && !(pk >> msb))
        msb--;
    pos = (msb - 15) * 8 + (msb >= 3 ? (pk >> (msb - 3)) & 7 : 0);
    return clamp(pos, 0, 64) * FD_H / 64;
}

static void meters_read(uint32_t now)                /* the output's peaks since the last look, falling 2 px a look */
{
    int32_t c;
    for (c = 0; c < 2; c++) {
        int32_t h = meter_px(out_peak[c]);
        out_peak[c] = 0;
        ui.meter[c] = h > ui.meter[c] - 2 ? h : ui.meter[c] - 2;
    }
    if (lim_low < 64000)                             /* ~-0.2 dB or more: the limiter is working */
        ui.lim_until = now + 300u;
    lim_low = 1 << 16;
    ui.meter_at = now;
    ui.redraw_mix = 1;
}

/* a send's row in a strip at x, w wide, from row y: its name, a bar of v / 256 in col; the master's returns alike */
static void send_row(int32_t x, int32_t w, int32_t y, const char *name, int32_t v, uint16_t lab, uint16_t col)
{
    int32_t bx = x + 6 + text_w(&FONT_T, "HALL"), bw = x + w - 3 - bx;
    cv_text(x + 3, y, &FONT_T, name, lab);
    cv_rect(bx, y + 5, bw, 2, K_LINE);
    cv_rect(bx, y + 5, bw * clamp(v, 0, 256) / 256, 2, col);
}

/* the mixer's tabs and strips, the rows y0 .. y0 + 92 of the screen: three parts a page, the chosen part's (1-3 or
 * 4-6; two marks in the tab row light the page), and the master; the tab lights what its knobs turn (MIX the faders,
 * SEND the sends) */
static void draw_mix(int32_t y0)
{
    int32_t p, i, solo = 0, bot = FD_TOP + FD_H, first = set.part < 3 ? 0 : 3;
    cv_begin(240, 93, K_GROUND);
    draw_tabs(y0, TABS_MIX, "");
    cv_rect(222, 27 - y0, 7, 5, first ? K_LINE : K_TEXT);
    cv_rect(231, 27 - y0, 7, 5, first ? K_TEXT : K_LINE);
    for (p = 0; p < KL_NPART; p++)
        solo |= set.parts[p].solo;
    for (p = first; p < first + 3; p++) {            /* a strip a part: glyph, name, fader, LEVEL, PAN, sends, M S */
        const part_set_t *ps = &set.parts[p];
        const tile_t *tl = &TILES[TILE_OF[ps->engine]];
        int32_t x = 1 + (p - first) * 64, cx = x + 31, ly = ps->level < -40 ? bot : bot - (ps->level + 40) * FD_H / 46;
        int sel = p == set.part, dim = ps->mute || (solo && !ps->solo);
        uint16_t bg = sel ? K_SURF2 : K_SURF, col = dim ? K_MUTED : tl->col;
        uint8_t sh[8];
        char t[12] = "";
        cv_rect(x, 43 - y0, 62, 163, bg);
        if (sel)
            box_edge(x, 43 - y0, 62, 163, 2, tl->col);
        kl_shape(ps->engine, work[p][ps->engine], sh);
        glyph_draw(x + 11, 55 - y0, 7, sh, tl->col, bg);
        t[0] = (char)('1' + p);
        t[1] = ' ';
        t[2] = 0;
        text_cat(t, tl->name);
        text_fit(x + 21, 49 - y0, 39, &FONT_T, t, sel ? tl->col : K_TEXT);
        cv_rect(cx - 1, FD_TOP - y0, 3, FD_H, K_LINE);
        if (ps->level >= -40)
            cv_rect(cx - 1, ly - y0, 3, bot - ly, ui.tab ? K_MUTED : col);
        cv_rect(cx - 9, ly - 2 - y0, 18, 5, K_TEXT);
        mix_text(ps, 0, t, 1);
        cv_text(cx - text_w(&FONT_T, t) / 2, 138 - y0, &FONT_T, t, K_TEXT);
        cv_rect(cx - 23, 152 - y0, 46, 2, K_LINE);
        disc(cx + ps->pan * 23 / 50, 153 - y0, 3, col);
        mix_text(ps, 1, t, 1);
        cv_text(cx - text_w(&FONT_T, t) / 2, 156 - y0, &FONT_T, t, K_MUTED);
        for (i = 0; i < 2; i++)                      /* DLY and HALL */
            send_row(x, 62, 167 + 11 * i - y0, i ? "HALL" : "DLY", ps->send[i] * 256 / 100,
                     ui.tab && sel ? K_TEXT : K_MUTED, ui.tab ? col : K_MUTED);
        for (i = 0; i < 2; i++) {                    /* M and S */
            int on = i ? ps->solo : ps->mute;
            int32_t bx = cx - 22 + i * 24;
            const char *l = i ? "S" : "M";
            if (on)
                cv_rect(bx, 190 - y0, 20, 13, K_TEXT);
            else
                box_edge(bx, 190 - y0, 20, 13, 1, K_LINE);
            cv_text(bx + (20 - text_w(&FONT_T, l)) / 2, 190 - y0, &FONT_T, l, on ? K_GROUND : K_MUTED);
        }
    }
    {   /* MASTER: the pot, the output's meters, the effects' returns, LIM */
        int32_t x = 193, cx = x + 13, pct = (int32_t)(plat_master() * 100u / 4096u), my = bot - pct * FD_H / 100;
        int lim = plat_ms() < ui.lim_until;
        char v[8];
        cv_rect(x, 43 - y0, 46, 163, K_SURF);
        cv_text(x + (46 - text_w(&FONT_T, "MASTER")) / 2, 49 - y0, &FONT_T, "MASTER", K_TEXT);
        cv_rect(cx - 1, FD_TOP - y0, 3, FD_H, K_LINE);
        cv_rect(cx - 1, my - y0, 3, bot - my, K_TEXT);
        cv_rect(cx - 8, my - 2 - y0, 16, 5, K_TEXT);
        for (i = 0; i < 2; i++) {
            int32_t mx = x + 27 + i * 8;
            cv_rect(mx, FD_TOP - y0, 5, FD_H, K_LINE);
            cv_rect(mx, bot - ui.meter[i] - y0, 5, ui.meter[i], K_GREEN);
            cv_text(mx, 138 - y0, &FONT_T, i ? "R" : "L", K_MUTED);
        }
        v[0] = 0;
        int_text(pct, v);
        text_cat(v, "%");
        cv_text(x + (46 - text_w(&FONT_T, v)) / 2, 152 - y0, &FONT_T, v, K_TEXT);
        for (i = 0; i < 2; i++)                      /* the returns: the effects' LEVEL */
            send_row(x, 46, 167 + 11 * i - y0, i ? "HALL" : "DLY",
                     (set.fx[i][FK_LEVEL] - LEVEL_OFF) * 256 / (6 - LEVEL_OFF), K_MUTED, K_MUTED);
        if (lim)
            cv_rect(x + 8, 190 - y0, 30, 13, K_TEXT);
        else
            box_edge(x + 8, 190 - y0, 30, 13, 1, K_MUTED);
        cv_text(x + 8 + (30 - text_w(&FONT_T, "LIM")) / 2, 190 - y0, &FONT_T, "LIM", lim ? K_GROUND : K_MUTED);
    }
    cv_blit(0, (uint32_t)y0);
}

/* -------------------------------------------------------- the effects' page --- */
static uint16_t fx_shade(int32_t n, int32_t tone)    /* the n-th echo (0: the first), darker with a lower TONE */
{
    return mix565(K_TEXT, K_LINE, (uint32_t)clamp(256 - n * (220 - 2 * tone) / 6, 40, 256));
}

/* the effects' page, the rows y0 .. y0 + 92: its tabs; a picture of the tab's effect (the note sent in grey, then
 * DELAY's echoes, left up and right down, TIME apart and FDBK of the one before; or the gap of PRE and HALL's tail,
 * dying away with DECAY), both darker with TONE; under it what each part sends, 3 x 2 (the mixer's SEND; the chosen
 * part framed, a silent one grey with M) */
static void draw_fx(int32_t y0)
{
    const int8_t *f = set.fx[ui.tab];
    int32_t p, i, cy = 92 - y0;
    char t[24] = "", v[12];
    cv_begin(240, 93, K_GROUND);
    if (!ui.tab) {
        int_text(lp.tempo, t);
        text_cat(t, " BPM");
    }
    draw_tabs(y0, TABS_FX, t);
    cv_rect(1, 44 - y0, 238, 82, K_SURF);
    t[0] = 0;
    if (ui.tab) {
        text_cat(t, "PRE ");
        fx_text(FX_HALL, 0, v);
    } else {
        text_cat(t, dly_played());
        text_cat(t, " = ");
        int_text((int32_t)((dly_samples(0) * 10u + 220u) / 441u), v);   /* samples / 44.1: ms */
        text_cat(v, " MS");
    }
    text_cat(t, v);
    cv_text(7, 47 - y0, &FONT_XS, ui.tab ? "STEREO" : "PING-PONG", K_TEXT);
    cv_text(232 - text_w(&FONT_XS, t), 47 - y0, &FONT_XS, t, K_MUTED);
    cv_text(8, 66 - y0, &FONT_T, "L", K_MUTED);
    cv_text(8, 106 - y0, &FONT_T, "R", K_MUTED);
    cv_rect(20, cy, 213, 1, K_LINE);
    cv_rect(24, cy - 28, 3, 57, K_LINE);              /* the note sent in */
    if (!ui.tab) {                                   /* DELAY: the echoes, L, R, L ... */
        int32_t step = 8 + (int32_t)(dly_samples(0) * 17u / FX_DLY_N), h = 28 << 8, fb = f[FK_FDBK] * 251 / 100;
        for (i = 0; 24 + step * (i + 1) < 228 && h >= 1 << 8; i++, h = h * fb >> 8) {
            int32_t x = 24 + step * (i + 1), hh = clamp(h >> 8, 2, 28);
            cv_rect(x, i & 1 ? cy + 1 : cy - hh, 4, hh, fx_shade(i, f[FK_TONE]));
        }
    } else {                                         /* HALL: the gap of PRE, the tail (a little different L and R) */
        uint32_t r = 12345u;
        int32_t x0 = 31 + f[0] * 30 / 100, tau = 16 + f[1] * 12 / 10, env = 26 << 8;
        int32_t k = (256 << 8) / tau;                /* 1 / tau, Q16: env falls by env / tau a step of 2 px */
        for (i = 0; x0 + 2 * i < 232 && env >= 1 << 8; i++, env -= (env * k >> 16) * 2) {   /* 6656 * 4096 fits */
            int32_t c, a = i < 3 ? (i + 1) * 85 : 256;   /* the first lines grow in */
            uint16_t col = fx_shade(i / 6, f[2]);
            for (c = 0; c < 2; c++) {
                int32_t hh;
                r = r * 1103515245u + 12345u;
                hh = ((env >> 8) * a >> 8) * (140 + (int32_t)(r >> 25)) >> 8;
                if (hh > 0)
                    cv_rect(x0 + 2 * i, c ? cy + 1 : cy - hh, 1, hh, col);
            }
        }
    }
    for (p = 0; p < KL_NPART; p++) {                 /* the parts' sends */
        const part_set_t *ps = &set.parts[p];
        const tile_t *tl = &TILES[TILE_OF[ps->engine]];
        int32_t x = 1 + (p % 3) * 80, y = 129 + (p / 3) * 40 - y0, s = ps->send[ui.tab], solo = 0, q;
        int sel = p == set.part, dim;
        uint16_t bg = sel ? K_SURF2 : K_SURF, col;
        uint8_t sh[8];
        for (q = 0; q < KL_NPART; q++)
            solo |= set.parts[q].solo;
        dim = ps->mute || (solo && !ps->solo);
        col = dim ? K_MUTED : tl->col;
        cv_rect(x, y, 78, 37, bg);
        if (sel)
            box_edge(x, y, 78, 37, 2, tl->col);
        kl_shape(ps->engine, work[p][ps->engine], sh);
        glyph_draw(x + 11, y + 11, 6, sh, col, bg);
        t[0] = 0;
        part_name(p, t);
        text_fit(x + 20, y + 4, 55, &FONT_XS, t, sel ? tl->col : dim ? K_MUTED : K_TEXT);
        cv_rect(x + 6, y + 27, 48, 3, K_LINE);
        cv_rect(x + 6, y + 27, 48 * s / 100, 3, col);
        v[0] = 0;
        if (dim)
            text_cat(v, "M");
        else
            int_text(s, v);
        cv_text(x + 73 - text_w(&FONT_XS, v), y + 20, &FONT_XS, v, s && !dim ? K_TEXT : K_MUTED);
    }
    cv_blit(0, (uint32_t)y0);
}

/* ------------------------------------------------------- the looper's lanes --- */
#define LT_X 60                                      /* the lanes' time line: from x, this wide */
#define LT_W 176
#define LANE_Y 54                                    /* the first lane's row, each this high (and a row apart): six
                                                     * fit */
#define LANE_H 24

/* a page's two tabs (the looper's LOOP and STEP, the mixer's MIX and SEND, the effects' DELAY and HALL), this one
 * lit (as the edit pages'), and a note on the right */
static void draw_tabs(int32_t y0, const char *const TAB[2], const char *right)
{
    const tile_t *tl = &TILES[TILE_OF[eng_cur()]];
    int32_t i, x = 8;
    for (i = 0; i < 2; i++) {
        int32_t w = text_w(&FONT_XS, TAB[i]);
        cv_text(x, 23 - y0, &FONT_XS, TAB[i], i == ui.tab ? K_TEXT : K_MUTED);
        if (i == ui.tab)
            cv_rect(x, 38 - y0, w, 2, tl->col);
        x += w + 14;
    }
    cv_text(232 - text_w(&FONT_XS, right), 23 - y0, &FONT_XS, right, K_MUTED);
    cv_rect(0, 40 - y0, 240, 1, K_LINE);
}

static uint32_t loop_view(void)                      /* the ticks the lanes show: the length, or as far as the first
                                                     * take has come (a bar at least) */
{
    if (lp.bars)
        return lp.bars * (uint32_t)LP_BAR;
    if (lp.first && lp.state == LP_PLAY)
        return ((lp.pos16 >> 16) / LP_BAR + 1u) * LP_BAR;
    return LP_BAR;
}

/* a part's lane at row y of the strip: its glyph and name on one line (a red dot before them while it records), the
 * bar lines, its notes (higher up higher; a note over the loop's end goes on at its start); the chosen part's
 * framed, a silent one's grey */
static void draw_lane(int p, int32_t y, uint32_t len, int32_t bars)
{
    static int16_t on_x[128];
    const part_set_t *ps = &set.parts[p];
    const tile_t *tl = &TILES[TILE_OF[ps->engine]];
    const lp_ev_t *e = lp_ev[p];
    int sel = p == set.part, recp = lp.rec && lp.rpart == p, solo = 0, q;
    uint16_t bg = sel ? K_SURF2 : K_SURF, col;
    uint32_t n = lp.n[p], i;
    int32_t b, lo = 127, hi = 0, top = y + 3, h = LANE_H - 6;
    uint8_t sh[8];
    char t[12] = "";
    for (q = 0; q < KL_NPART; q++)
        solo |= set.parts[q].solo;
    col = ps->mute || (solo && !ps->solo) ? K_MUTED : tl->col;
    cv_rect(1, y, 238, LANE_H, bg);
    if (sel)
        box_edge(1, y, 238, LANE_H, 2, tl->col);
    kl_shape(ps->engine, work[p][ps->engine], sh);
    if (recp)
        disc(5, y + LANE_H / 2, 2, K_REC);
    glyph_draw(14, y + LANE_H / 2, 6, sh, tl->col, bg);
    part_name(p, t);
    text_fit(22, y + 6, LT_X - 24, &FONT_T, t, sel ? tl->col : K_TEXT);
    if (LT_W / (bars * 4) >= 6)                      /* the beats, where there is room */
        for (b = 1; b < bars * 4; b++)
            cv_rect(LT_X + b * LT_W / (bars * 4), y + 3, 1, LANE_H - 6, mix565(K_LINE, bg, 110));
    for (b = 1; b < bars; b++)
        cv_rect(LT_X + b * LT_W / bars, y + 3, 1, LANE_H - 6, K_LINE);
    for (i = 0; i < n && e[i].tick < len; i++) {
        lo = e[i].note < lo ? e[i].note : lo;
        hi = e[i].note > hi ? e[i].note : hi;
    }
    if (lo > hi) {
        if (!recp)
            cv_text(LT_X + (LT_W - text_w(&FONT_T, "EMPTY")) / 2, y + 6, &FONT_T, "EMPTY", K_MUTED);
        return;
    }
#define NOTE_Y(nt) (hi == lo ? top + h / 2 - 1 : top + h - 3 - ((nt) - lo) * (h - 3) / (hi - lo))
#define NOTE_RECT(x0, x1, nt) cv_rect((x0), NOTE_Y(nt), (x1) - (x0) > 2 ? (x1) - (x0) : 2, 3, col)
    for (i = 0; i < 128u; i++)
        on_x[i] = -1;
    for (i = 0; i < n && e[i].tick < len; i++) {
        int32_t x = LT_X + (int32_t)(e[i].tick * (uint32_t)LT_W / len), nt = e[i].note;
        if (e[i].vel & 0x7Fu) {                      /* on: a note held on is drawn up to here */
            if (on_x[nt] >= 0)
                NOTE_RECT(on_x[nt], x, nt);
            on_x[nt] = (int16_t)x;
        } else if (on_x[nt] >= 0) {
            NOTE_RECT(on_x[nt], x, nt);
            on_x[nt] = -1;
        } else {                                     /* begun before the loop's end, last time round */
            NOTE_RECT(LT_X, x, nt);
        }
    }
    for (i = 0; i < 128u; i++)
        if (on_x[i] >= 0)
            NOTE_RECT(on_x[i], LT_X + LT_W, (int32_t)i);
#undef NOTE_RECT
#undef NOTE_Y
}

/* the looper's page, the rows y0 .. y0 + 92 of the screen: the ruler (the bars' numbers, the one playing lit), a lane
 * a part, the play line */
static void draw_loop(int32_t y0)
{
    uint32_t len = loop_view(), pos = lp.pos16 >> 16;
    int32_t bars = (int32_t)(len / LP_BAR), b, p, cur = lp.state == LP_PLAY ? (int32_t)(pos / LP_BAR) : -1;
    char v[12];
    cv_begin(240, 93, K_GROUND);
    bars_text(lp.bars, v);
    draw_tabs(y0, TABS_LOOP, v);
    for (b = 0; b < bars; b++) {
        int32_t x = LT_X + b * LT_W / bars;
        char t[4];
        int_text(b + 1, t);
        if (b == cur) {
            cv_rect(x + 1, 42 - y0, text_w(&FONT_T, t) + 4, 11, K_TEXT);
            cv_text(x + 3, 42 - y0, &FONT_T, t, K_GROUND);
        } else if (bars <= 8 || b % 4 == 0) {
            cv_text(x + 3, 42 - y0, &FONT_T, t, K_MUTED);
        }
    }
    for (p = 0; p < KL_NPART; p++)
        draw_lane((int)p, LANE_Y + p * (LANE_H + 1) - y0, len, bars);
    if (lp.state == LP_PLAY)
        cv_rect(LT_X + (int32_t)(pos * (uint32_t)LT_W / len), LANE_Y - y0, 2, KL_NPART * (LANE_H + 1) - 1, K_TEXT);
    cv_blit(0, (uint32_t)y0);
}

/* the step sequencer's tab, the rows y0 .. y0 + 92 of the screen: the bar's 16 step numbers (the one playing lit), a
 * row a part (a step with a note filled; the chosen part's framed, its held steps outlined), and below the chosen
 * part's: the held step's note, velocity and length, or a new step's (DRUMS: the instrument's row and the kit) */
#define SX0 54                                       /* the steps: from x, 16 in this width */
#define SXW 182
static void draw_step_cell(int32_t x, int32_t y, int32_t h, int on, int play, int i, uint16_t col, uint16_t bg)
{
    uint16_t c = on ? (play ? K_TEXT : col) : mix565(K_TEXT, bg, play ? 70 : (i / 4) % 2 ? 12 : 24);   /* empty: a beat's
                                                                                                     * four alike */
    cv_rect(x + 1, y, SXW / 16 - 2, h, c);
}

static void draw_step(int32_t y0)
{
    const tile_t *ct = &TILES[TILE_OF[eng_cur()]];
    uint32_t play = step_play(), sel = step_sel();
    int drums = eng_cur() == KL_DRUMS, s0 = step_first_held();
    int32_t p, i;
    char t[24] = "", v[12];
    cv_begin(240, 93, K_GROUND);
    t[0] = 0;
    text_cat(t, "BAR ");
    int_text(ui.bar + 1, v);
    text_cat(t, v);
    text_cat(t, "/");
    int_text((int32_t)step_bars(), v);
    text_cat(t, v);
    draw_tabs(y0, TABS_LOOP, t);
    for (i = 0; i < 16; i++) {                       /* the step numbers: 1, 5, 9, 13, and the one playing */
        int32_t cx = SX0 + i * SXW / 16 + SXW / 32;
        int_text(i + 1, v);
        if (play & 1u << i) {
            cv_rect(cx - text_w(&FONT_T, v) / 2 - 2, 42 - y0, text_w(&FONT_T, v) + 4, 11, K_TEXT);
            cv_text(cx - text_w(&FONT_T, v) / 2, 42 - y0, &FONT_T, v, K_GROUND);
        } else if (i % 4 == 0) {
            cv_text(cx - text_w(&FONT_T, v) / 2, 42 - y0, &FONT_T, v, K_MUTED);
        }
    }
    for (p = 0; p < KL_NPART; p++) {                 /* a row a part */
        const part_set_t *ps = &set.parts[p];
        const tile_t *tl = &TILES[TILE_OF[ps->engine]];
        int32_t y = 54 + p * 16 - y0;                 /* six rows of 16 */
        int chosen = p == set.part;
        uint16_t bg = chosen ? K_SURF2 : K_GROUND, col = chosen ? tl->col : mix565(tl->col, K_GROUND, 190);
        uint32_t m = step_mask((int)p, LP_ANY);
        uint8_t sh[8];
        t[0] = 0;
        part_name((int)p, t);
        if (chosen) {
            cv_rect(1, y, 238, 15, bg);
            box_edge(1, y, 238, 15, 1, tl->col);
        }
        kl_shape(ps->engine, work[p][ps->engine], sh);
        glyph_draw(10, y + 7, 5, sh, tl->col, bg);
        text_fit(18, y + 2, 35, &FONT_T, t, chosen ? tl->col : K_TEXT);
        for (i = 0; i < 16; i++) {
            int32_t x = SX0 + i * SXW / 16;
            draw_step_cell(x, y + 2, 11, (int)(m >> i & 1u), (int)(play >> i & 1u), (int)i, col, chosen ? bg : K_SURF);
            if (chosen && (ui.held & 1u << i))
                box_edge(x, y, SXW / 16, 15, 1, K_TEXT);
        }
    }
    cv_rect(1, 152 - y0, 238, 55, K_SURF);           /* the chosen part's */
    if (drums) {                                     /* the instrument on K2: its row; the kit, those with steps
                                                     * dotted */
        uint32_t m = step_mask(set.part, sel), any = 0;
        inst_text((int)sel, v);
        t[0] = 0;
        text_cat(t, v);
        text_cat(t, " - ");
        part_name(set.part, t);
        text_fit(8, 154 - y0, 150, &FONT_XS, t, ct->col);
        cv_text(232 - text_w(&FONT_T, "KEYS = ITS STEPS"), 156 - y0, &FONT_T, "KEYS = ITS STEPS", K_MUTED);
        cv_text(8, 171 - y0, &FONT_T, v, ct->col);
        for (i = 0; i < 16; i++)
            draw_step_cell(SX0 + i * SXW / 16, 172 - y0, 10, (int)(m >> i & 1u), (int)(play >> i & 1u), (int)i,
                           ct->col, K_SURF);
        for (i = 0; i < 12; i++)                     /* which instruments the bar has */
            if (step_mask(set.part, (uint32_t)i))
                any |= 1u << i;
        for (i = 0; i < 12; i++) {
            int32_t x = 4 + i * 19;
            inst_text((int)i, v);
            if ((uint32_t)i == sel)
                cv_rect(x, 187 - y0, 18, 16, ct->col);
            else
                box_edge(x, 187 - y0, 18, 16, 1, K_LINE);
            cv_text(x + (18 - text_w(&FONT_T, v)) / 2, 189 - y0, &FONT_T, v,
                    (uint32_t)i == sel ? K_GROUND : any >> i & 1u ? K_TEXT : K_MUTED);
            if (any >> i & 1u)
                cv_rect(x + 14, 189 - y0, 2, 2, (uint32_t)i == sel ? K_GROUND : ct->col);
        }
    } else {                                         /* the held step's notes, or a new step's */
        int note = stp.note[set.part], vel = stp.vel[set.part], len = stp.len[set.part], held = 0;
        static const char *const LAB[3] = {"NOTE", "VEL", "LEN"};
        if (s0 >= 0)
            held = step_get(s0, &note, &vel, &len);
        t[0] = 0;
        if (held) {
            text_cat(t, "STEP ");
            two_digits(s0 + 1, v);
            text_cat(t, v);
        } else {
            text_cat(t, "NEW STEPS");
        }
        text_fit(8, 154 - y0, 150, &FONT_XS, t, ct->col);
        t[0] = 0;
        part_name(set.part, t);
        cv_text(232 - text_w(&FONT_T, held ? "HELD" : t), 156 - y0, &FONT_T, held ? "HELD" : t, K_MUTED);
        for (i = 0; i < 3; i++) {
            int32_t x = 8 + i * 75;
            if (i == 0)
                note_text(note, v);
            else if (i == 1)
                int_text(vel, v);
            else {
                v[0] = 0;
                text_cat(v, LEN_NAME[len]);
            }
            cv_text(x, 174 - y0, &FONT_T, LAB[i], K_MUTED);  /* the label, its value beside it */
            cv_text(x + text_w(&FONT_T, LAB[i]) + 4, 171 - y0, &FONT_B, v, K_TEXT);
        }
        cv_text(8, 193 - y0, &FONT_T, "WHITE KEYS: STEPS. HOLD ONE, TURN K2-K4", K_MUTED);
    }
    cv_blit(0, (uint32_t)y0);
}

static void draw(void)
{
    draw_header();
    if (ui.mix) {
        draw_mix(22);
        draw_mix(115);
    } else if (ui.fx) {
        draw_fx(22);
        draw_fx(115);
    } else if (ui.loop && ui.tab) {
        draw_step(22);
        draw_step(115);
    } else if (ui.loop) {
        draw_loop(22);
        draw_loop(115);
    } else if (ui.edit) {
        draw_edit(22);
        draw_edit(115);
    } else {
        draw_row(0);
        draw_row(1);
    }
    draw_footer();
}

void app_ui_init(void)
{
    touch_on(1);                                     /* velocity: the keys' readings on */
    lcd_fill(0, 0, 240, 240, K_GROUND);
    ui.redraw = 1;
}

void app_frame(void)
{
    static uint32_t last_pct;
    uint32_t now = plat_ms(), pct = plat_master() * 100u / 4096u;
    int note;
    uint32_t hits;
    input();
    speak();
    if (lp.tempo != fx_tempo)                        /* DELAY's time follows TEMPO */
        fx_apply();
    if ((hits = kl_hits(set.part, &note)) != ui.hits) {        /* a note played: DRUMS lights its pad a moment */
        ui.hits = hits;
        ui.hit_pc = (int8_t)(note % 12);
        ui.hit_until = now + 250u;
        ui.redraw_pads = ui.edit && eng_cur() == KL_DRUMS;
    } else if (ui.hit_until && now >= ui.hit_until) {
        ui.hit_until = 0;
        ui.redraw_pads = ui.edit && eng_cur() == KL_DRUMS;
    }
    if (pct != last_pct || (ui.msg_until && now >= ui.msg_until)) {
        last_pct = pct;
        if (now >= ui.msg_until)
            ui.msg_until = 0;
        ui.redraw = 1;
    }
    if (ui.mix && now - ui.meter_at >= 50u)          /* the mixer's meters, 20 times a second */
        meters_read(now);
    if (ui.ask && now >= ui.msg_until)               /* HOME + PLAY's question unanswered: nothing emptied */
        ui.ask = 0;
    if (ui.bar >= step_bars())                       /* the loop got shorter: STEP shows its last bar */
        ui.bar = (uint8_t)(step_bars() - 1u);
    {
        uint32_t sig = loop_sig();
        char it[8];
        loop_ind(it);
        if (sig != ui.lp_sig) {                      /* the looper did something: the whole screen */
            ui.lp_sig = sig;
            ui.redraw = 1;
        }
        if (lp.full != ui.full_seen) {               /* a note left out: the part is full */
            char t[24] = "";
            ui.full_seen = lp.full;
            part_name(lp.rpart, t);
            app_say("FULL", t);
        }
        if (!text_eq(it, ui.ind)) {                  /* another beat: the header */
            ui.ind[0] = 0;
            text_cat(ui.ind, it);
            ui.redraw_head = 1;
        }
    }
    if (ui.loop && lp.state != LP_STOP && now - ui.loop_at >= 40u) {   /* the play line, 25 times a second */
        ui.loop_at = now;
        ui.redraw_loop = 1;
    }
    if (ui.redraw) {
        ui.redraw = ui.redraw_pads = ui.redraw_mix = ui.redraw_loop = ui.redraw_head = 0;
        draw();
        return;
    }
    if (ui.redraw_head) {
        ui.redraw_head = 0;
        draw_header();
    }
    if (ui.redraw_loop && ui.tab) {
        ui.redraw_loop = 0;
        draw_step(22);
        draw_step(115);
    } else if (ui.redraw_loop) {
        ui.redraw_loop = 0;
        draw_loop(22);
        draw_loop(115);
    } else if (ui.redraw_mix) {                      /* the strips alone: the meters moved */
        ui.redraw_mix = 0;
        draw_mix(22);
        draw_mix(115);
    } else if (ui.redraw_pads) {                     /* the pads alone: a hit costs two strips, not the screen */
        ui.redraw_pads = 0;
        draw_edit(22);
        draw_edit(115);
    }
}

#ifdef PLAT_HOST
void app_host_reboot(void)
{
    uint32_t i;
    uint8_t *b = (uint8_t *)&ui;
    for (i = 0; i < sizeof ui; i++)
        b[i] = 0;
    set_dirty = 0;
}

int app_host_cmd(const char *cmd, const char *a, const char *b, const char *c, const char *d)
{
    (void)a, (void)b, (void)c, (void)d;
    if (!strcmp(cmd, "kls8")) {                      /* the settings into the store as KLS8 (four parts) */
        settings8_t old;
        uint32_t i, e;
        memset(&old, 0, sizeof old);
        old.magic = SET_MAGIC8;
        old.part = set.part;
        for (i = 0; i < OLD_NPART; i++)
            old.parts[i] = set.parts[i];
        for (e = 0; e < KL_NENG; e++)
            old.user[e] = set.user[e];
        for (i = 0; i < 8u; i++)
            old.fx[i / 4u][i % 4u] = set.fx[i / 4u][i % 4u];
        plat_store_save(OBJ_SETTINGS, &old, sizeof old);
        return 1;
    }
    if (!strcmp(cmd, "kls7")) {                      /* as KLS7 (v0.5's: four parts, no sends) */
        settings7_t old;
        uint32_t i, e;
        memset(&old, 0, sizeof old);
        old.magic = SET_MAGIC7;
        old.part = set.part;
        for (i = 0; i < OLD_NPART; i++) {
            const part_set_t *ps = &set.parts[i];
            old.parts[i].engine = ps->engine;
            for (e = 0; e < KL_NENG; e++)
                old.parts[i].preset[e] = ps->preset[e];
            old.parts[i].octave = ps->octave;
            old.parts[i].level = ps->level;
            old.parts[i].pan = ps->pan;
            old.parts[i].mute = ps->mute;
            old.parts[i].solo = ps->solo;
        }
        for (e = 0; e < KL_NENG; e++)
            old.user[e] = set.user[e];
        plat_store_save(OBJ_SETTINGS, &old, sizeof old);
        return 1;
    }
    return 0;
}

static int expect_k(const char *what, const char *name)   /* "v4" for name "v": 4; else -1 */
{
    size_t n = strlen(name);
    return !strncmp(what, name, n) && what[n] >= '0' && what[n] <= '7' && !what[n + 1] ? what[n] - '0' : -1;
}

int app_expect(const char *what, int *got)
{
    int k;
    if (!strcmp(what, "engine"))                     /* the chosen part's (so are preset, octave, edited, v, dsp) */
        *got = eng_cur();
    else if (!strcmp(what, "part"))                  /* 1..4 */
        *got = set.part + 1;
    else if (!strcmp(what, "preset"))
        *got = part_cur()->preset[eng_cur()];
    else if (!strcmp(what, "octave"))
        *got = part_cur()->octave;
    else if (!strcmp(what, "voices"))                /* every part's */
        *got = kl_voices(-1);
    else if (!strcmp(what, "pvoices"))               /* the chosen part's */
        *got = kl_voices(set.part);
    else if (!strcmp(what, "cost"))                  /* what they cost, tenths of an FM6 voice */
        *got = kl_cost_now();
    else if (!strcmp(what, "mix"))                   /* the mixer is open */
        *got = ui.mix;
    else if (!strcmp(what, "fx"))                    /* the effects' page is open */
        *got = ui.fx;
    else if (!strcmp(what, "dlyd"))                  /* DELAY's time, samples, as it plays at TEMPO */
        *got = (int)dly_samples(0);
    else if ((k = expect_k(what, "fxawake")) >= 0)   /* 0 DELAY, 1 HALL: it runs */
        *got = fx_awake(k);
    else if ((k = expect_k(what, "send")) >= 0)      /* the chosen part's: 0 DELAY, 1 HALL */
        *got = k < 2 ? part_cur()->send[k] : -1;
    else if ((k = expect_k(what, "fx")) >= 0)        /* the effects' knobs: 0..3 DELAY's, 4..7 HALL's */
        *got = set.fx[k / 4][k % 4];
    else if (!strcmp(what, "level"))                 /* the chosen part's strip */
        *got = part_cur()->level;
    else if (!strcmp(what, "pan"))
        *got = part_cur()->pan;
    else if (!strcmp(what, "mute"))
        *got = part_cur()->mute;
    else if (!strcmp(what, "solo"))
        *got = part_cur()->solo;
    else if (!strcmp(what, "outl") || !strcmp(what, "outr")) {   /* the output's peak since the last ask, 24 bits */
        int c = what[3] == 'r';
        *got = test_peak[c];
        test_peak[c] = 0;
    } else if (!strcmp(what, "lim")) {               /* 1: the limiter held the level down since the last ask */
        *got = test_lim < 64000;
        test_lim = 1 << 16;
    }
    else if (!strcmp(what, "dirty"))
        *got = set_dirty;
    else if (!strcmp(what, "edit"))
        *got = ui.edit;
    else if (!strcmp(what, "page"))
        *got = ui.page + 1;
    else if (!strcmp(what, "edited"))
        *got = edited[set.part][eng_cur()];
    else if (!strcmp(what, "user"))
        *got = set.user[eng_cur()].saved;
    else if (!strcmp(what, "note"))                  /* a MONO sound: the note its voice plays, -1 none */
        *got = kl_note_now(set.part);
    else if (!strcmp(what, "pad"))                   /* DRUMS: the lit pad's semitone, -1 none */
        *got = ui.hit_until ? ui.hit_pc : -1;
    else if (!strcmp(what, "loop"))                  /* the looper's page is open */
        *got = ui.loop;
    else if (!strcmp(what, "lstate"))                /* 0 stopped, 1 counting in, 2 playing */
        *got = lp.state;
    else if (!strcmp(what, "rec"))                   /* recording, into part rpart (1..4) */
        *got = lp.rec;
    else if (!strcmp(what, "rpart"))
        *got = lp.rpart + 1;
    else if (!strcmp(what, "bars"))                  /* the loop's length, 0: AUTO (none yet) */
        *got = lp.bars;
    else if (!strcmp(what, "lbar"))                  /* the bar playing, 1.., 0 stopped */
        *got = lp.state == LP_PLAY ? (int)((lp.pos16 >> 16) / LP_BAR + 1u) : 0;
    else if (!strcmp(what, "tempo"))
        *got = lp.tempo;
    else if (!strcmp(what, "quant"))                 /* 0 OFF, 1 1/32, 2 1/16, 3 1/8, 4 1/4 */
        *got = lp.quant;
    else if (!strcmp(what, "click"))                 /* 0 OFF, 1 REC, 2 ON */
        *got = lp.click;
    else if (!strcmp(what, "events"))                /* the chosen part's notes on and off in the loop */
        *got = lp.n[set.part];
    else if (!strcmp(what, "lsound"))                /* the chosen part's notes the looper has sounding */
        *got = (lp.on[set.part][0] | lp.on[set.part][1] | lp.on[set.part][2] | lp.on[set.part][3]) != 0;
    else if (!strcmp(what, "hits"))                  /* the chosen part's notes started so far (keys, MIDI, looper) */
        *got = (int)kl_hits(set.part, &k);
    else if (!strcmp(what, "ask"))                   /* HOME + PLAY's question stands */
        *got = ui.ask;
    else if (!strcmp(what, "tab"))                   /* the page's: 1 LOOP, MIX, DELAY; 2 STEP, SEND, HALL */
        *got = ui.tab + 1;
    else if (!strcmp(what, "sbar"))                  /* STEP: the bar shown, 1.. */
        *got = ui.bar + 1;
    else if (!strcmp(what, "smask"))                 /* the chosen part's steps with a note in the bar shown (DRUMS:
                                                     * the instrument's), bit 0 step 1 */
        *got = (int)step_mask(set.part, step_sel());
    else if (!strcmp(what, "sall"))                  /* the same, every note */
        *got = (int)step_mask(set.part, LP_ANY);
    else if (!strcmp(what, "slights"))               /* the white keys lit, bit 0 step 1 */
        *got = (int)(step_mask(set.part, step_sel()) ^ step_play());
    else if (!strcmp(what, "snote"))                 /* a new step's note (DRUMS: the instrument's semitone) */
        *got = eng_cur() == KL_DRUMS ? stp.inst[set.part] : stp.note[set.part];
    else if (!strcmp(what, "svel"))
        *got = stp.vel[set.part];
    else if (!strcmp(what, "slen"))                  /* LEN: 0 1/32, 1 1/16, 2 1/8 .. 8 1/1 */
        *got = stp.len[set.part];
    else if ((k = expect_k(what, "lt")) >= 0)        /* the chosen part's event k: its tick (-1 none) */
        *got = (uint32_t)k < lp.n[set.part] ? lp_ev[set.part][k].tick : -1;
    else if ((k = expect_k(what, "lv")) >= 0)        /* its velocity (0: a note's end) */
        *got = (uint32_t)k < lp.n[set.part] ? lp_ev[set.part][k].vel & 0x7F : -1;
    else if ((k = expect_k(what, "ln")) >= 0)        /* its note */
        *got = (uint32_t)k < lp.n[set.part] ? lp_ev[set.part][k].note : -1;
    else if ((k = expect_k(what, "v")) >= 0)
        *got = work[set.part][eng_cur()][k];
    else if ((k = expect_k(what, "dsp")) >= 0)
        *got = kl_value_now(set.part, k);
    else if ((k = expect_k(what, "glyph")) >= 0) {
        uint8_t sh[8];
        kl_shape(eng_cur(), work[set.part][eng_cur()], sh);
        *got = sh[k];
    } else
        return 0;
    return 1;
}
#endif
