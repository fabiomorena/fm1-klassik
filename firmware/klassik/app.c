/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena
 * From fm1-base's template, Copyright (C) 2026 Charles Vestal (MPL-2.0) */
/* KLASSIK v0.5 (parts, mixer, looper): the rack and its edit pages. Six engines, FM6 (msfa's six operators), three of
 * Felucca-Engines (ANALOG, PHASE, WHEEL), DRUMS (four kits on Plaits' drum models) and ACID (Open303's TB-303), on
 * four parts that play at once, each its own sound and octave: SEL, or SELECT turned, goes to the next part (SELECT
 * to the left: the one before), and the keys, K1 (the engine, in the rack's order), PRESET (its sound), OCT- / OCT+
 * (its octave) and EDIT are the chosen part's; MIDI
 * channel 1..4 plays part 1..4 (others the chosen part), the keys send on their part's channel. On ACID a key
 * pressed while ENV is held is accented (ACC in the header). EDIT opens the engine's two pages of four parameters
 * on K1..K4 (EDIT or OCT+ the other page, OCT- back to the rack); an edit plays at once (FM6's MOD, DTUN, ATK, DEC,
 * SUS and REL and ACID's DECAY and ACCENT from the next note). HOME held, a knob turned goes back to its sound's
 * value (the preset's, or the USER sound's), marked on its bar while it is elsewhere.
 * SAVE keeps the parts (the one chosen, each one's engine, sounds and octave) across a power cycle, an edited sound
 * as its engine's USER sound (every part's).
 * GLO opens the mixer: a strip a part (LEVEL, PAN, MUTE, SOLO on K1..K4 for the chosen part) and the master's
 * meters; the output is stereo, through a limiter.
 * The looper records notes (not sound) on one loop for all four parts: PLAY starts and stops it, REC records the
 * chosen part on top of what it has (stopped: after a bar's count-in), the first take sets the length; HOME + REC
 * takes the last take back, HOME + PLAY empties a part. SEQ opens its page: a lane a part, TEMPO, LENGTH, QUANT and
 * CLICK on K1..K4; the header shows it on every page.
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
#define SET_MAGIC 0x4B4C5337u                        /* "KLS7": another magic (the parts' KLS6, v0.3's KLS5, DRUMS'
                                                     * KLS4, v0.2's KLS3, v0.1's KLS2, v0's KLS1) loads defaults */
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
} part_set_t;
typedef struct {
    uint32_t magic;
    uint8_t part;                                    /* the part chosen: the keys, K1, PRESET, OCT and EDIT are its */
    uint8_t rsv[3];
    part_set_t parts[KL_NPART];
    user_t user[KL_NENG];                            /* each engine's own sound (SAVE after an edit), every part's */
} settings_t;
_Static_assert(sizeof(settings_t) <= PLAT_STORE_MAX, "the settings are one store object");
static settings_t set, set_kept;                     /* as it is, and as the store has it (or the defaults at boot) */
static uint8_t set_dirty;
static const uint8_t PART_ENGINE[KL_NPART] = {KL_ANALOG, KL_DRUMS, KL_ACID, KL_FM6};   /* the parts at first */
#define LEVEL_OFF (-41)                              /* the mixer's LEVEL below -40 dB: off */

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
            ps->pan < -50 || ps->pan > 50 || ps->mute > 1 || ps->solo > 1)
            return 0;
        for (i = 0; i < KL_NENG; i++)
            if (ps->preset[i] >= kl_presets((int)i) + set.user[i].saved)
                return 0;
    }
    for (i = 0; i < KL_NENG; i++) {
        const user_t *u = &set.user[i];
        if (u->saved > 1 || (u->saved && (u->base >= kl_presets((int)i) || !values_ok((int)i, u->v))))
            return 0;
    }
    return 1;
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

/* the gains from LEVEL, PAN, MUTE and SOLO: a soloed part mutes every part not soloed; PAN keeps the near side and
 * turns the far one down (the middle: both at LEVEL, as without a mixer) */
static void mix_update(void)
{
    int p, solo = 0;
    for (p = 0; p < KL_NPART; p++)
        solo |= set.parts[p].solo;
    for (p = 0; p < KL_NPART; p++) {
        const part_set_t *ps = &set.parts[p];
        int32_t g = ps->level < -40 || ps->mute || (solo && !ps->solo) ? 0 : LEVEL_Q11[ps->level + 40];
        mix_tgt[p][0] = g * (ps->pan > 0 ? 50 - ps->pan : 50) / 50;
        mix_tgt[p][1] = g * (ps->pan < 0 ? 50 + ps->pan : 50) / 50;
    }
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
enum { LC_PLAY, LC_STOP, LC_REC, LC_UNREC, LC_PART, LC_UNDO, LC_CLEAR, LC_BARS };   /* the main loop's asks */
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
static uint8_t lc_cmd[LC_N], lc_arg[LC_N];
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

static void lp_ask(int cmd, int arg)                 /* main loop: an ask for the render, in order */
{
    uint32_t w = lc_w;
    if (w - lc_r >= LC_N)
        return;
    lc_cmd[w % LC_N] = (uint8_t)cmd;
    lc_arg[w % LC_N] = (uint8_t)arg;
    lc_w = w + 1u;
}

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

/* an event into its place in a part: after those at its tick; ahead of the play line it is passed over once (it has
 * sounded live), behind it the next to play stays the same */
static void lp_insert(int p, uint32_t tick, int note, int vel)
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
    if (lp.state == LP_COUNT || (lp.state == LP_PLAY && (tick << 16) >= lp.pos16))
        e[i].vel |= 0x80u;
    else if (i <= lp.cur[p])
        lp.cur[p]++;
    lp.n[p] = (uint16_t)(n + 1u);
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
    lp_insert(p, (uint32_t)t, note, on ? clamp(vel, 1, 127) : 0);
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

static void lp_asks(void)                            /* render: the main loop's asks, then the keys it played */
{
    while (lc_r != lc_w) {
        uint32_t j = lc_r % LC_N;
        int a = lc_arg[j];
        switch (lc_cmd[j]) {
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

/* ------------------------------------------------------------------ audio --- */
static int32_t part_buf[KL_NPART][APP_BLOCK];
static int32_t gain_q8;                              /* MASTER's gain, gliding: no step at a block's edge */
static int32_t mix_q19[KL_NPART][2];                 /* the parts' gains as they glide, Q19 (Q11 << 8) */
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

static void midi_in(void)                            /* audio interrupt: the packets that came in, channel 1..4 to part
                                                     * 1..4, the others to the part chosen (and to the looper) */
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

_Static_assert(KL_NPART == 4, "app_render's out[] has a buffer for each part");
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
        int32_t *const out[KL_NPART] = {part_buf[0] + b, part_buf[1] + b, part_buf[2] + b, part_buf[3] + b};
        lp_step(ck_buf + b);
        kl_render(out, 32u);
    }
    for (i = 0; i < n; i++) {
        int32_t c, sum[2] = {0, 0}, y[2], pk, lim;
        for (p = 0; p < KL_NPART; p++) {             /* each part at its gains: L and R */
            int32_t x = clamp(part_buf[p][i], -524287, 524287);   /* 2^19 * the most gain, 4086, fits 32 bits */
            for (c = 0; c < 2; c++) {
                mix_q19[p][c] += ((mix_tgt[p][c] << 8) - mix_q19[p][c]) >> 8;   /* ~6 ms */
                sum[c] += x * (mix_q19[p][c] >> 8) >> 11;
            }
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

void app_init(void)
{
    int e, p;
    if (plat_store_load(OBJ_SETTINGS, &set, sizeof set) != (int)sizeof set || !settings_ok())
        settings_defaults();
    set_kept = set;
    for (p = 0; p < KL_NPART; p++) {
        for (e = 0; e < KL_NENG; e++)
            sound_load(p, e);
        sound_select(p);
    }
    mix_update();
    loop_init();
}

/* --------------------------------------------------------------------- UI --- */
static const char *const BTN_NAME[NB] = {"FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ",
                                         "PLAY", "REC", "OCT-", "OCT+"};
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

/* a mixer knob's value as the screen shows it: K1 LEVEL "-6 dB" / "OFF", K2 PAN "C" / "L10" / "R25", K3 MUTE and
 * K4 SOLO "ON" / "OFF"; short: as under a strip ("-6", "OFF") */
static void mix_text(const part_set_t *ps, int32_t k, char *b, int short_form)
{
    char t[8];
    b[0] = 0;
    if (k == 0 && ps->level < -40) {
        text_cat(b, "OFF");
    } else if (k == 0) {
        if (ps->level > 0)
            text_cat(b, "+");
        int_text(ps->level, t);
        text_cat(b, t);
        if (!short_form)
            text_cat(b, " dB");
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

static const char *const MIX_LABEL[4] = {"LEVEL", "PAN", "MUTE", "SOLO"};

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
        if (ui.mix) {                                /* the mixer: the chosen part's strip */
            text_cat(t, ". MIX");
            for (k = 0; k < 4; k++) {
                char v[12];
                text_cat(t, ". ");
                text_cat(t, MIX_LABEL[k]);
                text_cat(t, " ");
                mix_text(part_cur(), k, v, 0);
                text_cat(t, v);
            }
        }
        if (ui.loop) {                               /* the looper: what it does, what its knobs hold */
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
    ui.mix = ui.loop = 0;
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

static void mix_said(int32_t k)                      /* a screen reader hears the knob: "LEVEL -6 dB" */
{
    char t[32], v[12];
    t[0] = 0;
    text_cat(t, MIX_LABEL[k]);
    text_cat(t, " ");
    mix_text(part_cur(), k, v, 0);
    text_cat(t, v);
    announce(t);
}

static void mix_show(int on)                         /* GLO: the mixer open (an edit page closes), or shut */
{
    ui.mix = (uint8_t)(on != 0);
    ui.edit = ui.loop = 0;
    ui.redraw = 1;
    announce(on ? "MIX" : "RACK");
}

static void mix_changed(int32_t k)
{
    mix_update();
    dirty_update();
    ui.redraw = 1;
    mix_said(k);
}

/* K1..K4 on the mixer, the chosen part's: LEVEL a dB a detent (below -40 off), PAN a step of 50 a side, MUTE and
 * SOLO on to the right, off to the left */
static void mix_step(int32_t k, int32_t d)
{
    part_set_t *ps = part_cur();
    if (k == 0)
        ps->level = (int8_t)clamp(ps->level + d, LEVEL_OFF, 6);
    else if (k == 1)
        ps->pan = (int8_t)clamp(ps->pan + d, -50, 50);
    else if (k == 2)
        ps->mute = d > 0;
    else
        ps->solo = d > 0;
    mix_changed(k);
}

static void mix_reset(int32_t k)                     /* HOME + K1..K4 on the mixer: 0 dB, the middle, off */
{
    part_set_t *ps = part_cur();
    if (k == 0)
        ps->level = 0;
    else if (k == 1)
        ps->pan = 0;
    else if (k == 2)
        ps->mute = 0;
    else
        ps->solo = 0;
    mix_changed(k);
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

static void loop_show(int on)                        /* SEQ: the looper's page open (another closes), or shut */
{
    ui.loop = (uint8_t)(on != 0);
    ui.edit = ui.mix = 0;
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
    uint32_t h = lp.state | (uint32_t)lp.rec << 2 | (uint32_t)lp.rpart << 3 | (uint32_t)lp.bars << 5 |
                 (uint32_t)lp.first << 10 | (uint32_t)lp.undone << 11;
    int p;
    for (p = 0; p < KL_NPART; p++)
        h = h * 31u + lp.n[p];
    return h;
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
        if ((keys & bit) && !(ui.keys & bit)) {
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
        else if ((k == B_OCTDN && ui.loop) || k == B_SEQ)
            loop_show(!ui.loop);                     /* the looper's page, open and shut */
        else if ((k == B_OCTDN && ui.mix) || k == B_GLO)
            mix_show(!ui.mix);                       /* the mixer, open and shut */
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
        else if (k == B_HOME && (ui.edit || ui.mix || ui.loop))
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
        else if (ui.loop && k >= EN_K1 && (btn & 1u << B_HOME))
            loop_reset((int32_t)(k - EN_K1));
        else if (ui.loop && k >= EN_K1)
            loop_step((int32_t)(k - EN_K1), e);
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
                  (lp.state != LP_STOP ? 1u << B_PLAY : 0u) | (lp.rec ? 1u << B_REC : 0u),
              keys);
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
    } else if (ui.mix || ui.loop) {                  /* the mixer, the looper: all four parts */
        const char *c = ui.mix ? "MIX" : "LOOP";
        char st[24] = "";
        int32_t w = text_w(&FONT_XS, c) + 8;
        if (ui.mix)
            text_cat(st, "PARTS 1-4");
        else
            loop_state_text(st);
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
        if (set.parts[p].engine != tl->eng)
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
 * elsewhere, MUTE and SOLO not (OFF or ON says it) */
static void footer_mix(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    const part_set_t *ps = part_cur();
    int32_t i;
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

static void draw_footer(void)                        /* what the controls hold, each with a bar of where it is */
{
    const tile_t *tl = &TILES[TILE_OF[eng_cur()]];
    char lab[4][16], v[4][12];
    int32_t frac[4], mark[4], i;
    if (ui.mix)
        footer_mix(lab, v, frac, mark);
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
static const char *const KEY_NAME[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
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

/* ---------------------------------------------------------------- the mixer --- */
static int32_t meter_px(int32_t pk)                  /* a peak (24 bits) as a meter's height: -48 .. 0 dBFS in 88 px,
                                                     * 6 dB a bit of the peak, its next three bits between */
{
    int32_t msb = 23, pos;
    if (pk <= 0)
        return 0;
    while (msb > 0 && !(pk >> msb))
        msb--;
    pos = (msb - 15) * 8 + (msb >= 3 ? (pk >> (msb - 3)) & 7 : 0);
    return clamp(pos, 0, 64) * 88 / 64;
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

static void draw_mix(int32_t y0)                     /* the mixer's strips, the rows y0 .. y0 + 92 of the screen */
{
    int32_t p, i, solo = 0;
    cv_begin(240, 93, K_GROUND);
    for (p = 0; p < KL_NPART; p++)
        solo |= set.parts[p].solo;
    for (p = 0; p < KL_NPART; p++) {                 /* a strip a part: glyph, name, fader, LEVEL, PAN, M S */
        const part_set_t *ps = &set.parts[p];
        const tile_t *tl = &TILES[TILE_OF[ps->engine]];
        int32_t x = 1 + p * 48, cx = x + 23, ly = ps->level < -40 ? 146 : 146 - (ps->level + 40) * 88 / 46;
        int sel = p == set.part, dim = ps->mute || (solo && !ps->solo);
        uint16_t bg = sel ? K_SURF2 : K_SURF;
        uint8_t sh[8];
        char t[12] = "";
        cv_rect(x, 24 - y0, 46, 182, bg);
        if (sel) {
            cv_rect(x, 24 - y0, 46, 2, tl->col);
            cv_rect(x, 204 - y0, 46, 2, tl->col);
            cv_rect(x, 24 - y0, 2, 182, tl->col);
            cv_rect(x + 44, 24 - y0, 2, 182, tl->col);
        }
        kl_shape(ps->engine, work[p][ps->engine], sh);
        glyph_draw(cx, 34 - y0, 7, sh, tl->col, bg);
        t[0] = (char)('1' + p);
        t[1] = ' ';
        t[2] = 0;
        text_cat(t, tl->name);
        text_fit(cx - clamp(text_w(&FONT_T, t), 0, 42) / 2, 43 - y0, 42, &FONT_T, t, sel ? tl->col : K_TEXT);
        cv_rect(cx - 1, 58 - y0, 3, 88, K_LINE);
        if (ps->level >= -40)
            cv_rect(cx - 1, ly - y0, 3, 146 - ly, dim ? K_MUTED : tl->col);
        cv_rect(cx - 9, ly - 2 - y0, 18, 5, K_TEXT);
        mix_text(ps, 0, t, 1);
        cv_text(cx - text_w(&FONT_T, t) / 2, 149 - y0, &FONT_T, t, K_TEXT);
        cv_rect(cx - 17, 165 - y0, 34, 2, K_LINE);
        disc(cx + ps->pan * 17 / 50, 166 - y0, 3, dim ? K_MUTED : tl->col);
        mix_text(ps, 1, t, 1);
        cv_text(cx - text_w(&FONT_T, t) / 2, 170 - y0, &FONT_T, t, K_MUTED);
        for (i = 0; i < 2; i++) {                    /* M and S */
            int on = i ? ps->solo : ps->mute;
            int32_t bx = cx - 16 + i * 18;
            const char *l = i ? "S" : "M";
            if (on)
                cv_rect(bx, 186 - y0, 14, 13, K_TEXT);
            else
                box_edge(bx, 186 - y0, 14, 13, 1, K_LINE);
            cv_text(bx + (14 - text_w(&FONT_T, l)) / 2, 186 - y0, &FONT_T, l, on ? K_GROUND : K_MUTED);
        }
    }
    {   /* MASTER: the pot, the output's meters, LIM */
        int32_t x = 193, cx = x + 13, pct = (int32_t)(plat_master() * 100u / 4096u), my = 146 - pct * 88 / 100;
        int lim = plat_ms() < ui.lim_until;
        char v[8];
        cv_rect(x, 24 - y0, 46, 182, K_SURF);
        cv_text(x + (46 - text_w(&FONT_T, "MASTER")) / 2, 30 - y0, &FONT_T, "MASTER", K_TEXT);
        cv_rect(cx - 1, 58 - y0, 3, 88, K_LINE);
        cv_rect(cx - 1, my - y0, 3, 146 - my, K_TEXT);
        cv_rect(cx - 8, my - 2 - y0, 16, 5, K_TEXT);
        for (i = 0; i < 2; i++) {
            int32_t mx = x + 27 + i * 8;
            cv_rect(mx, 58 - y0, 5, 88, K_LINE);
            cv_rect(mx, 146 - ui.meter[i] - y0, 5, ui.meter[i], K_GREEN);
            cv_text(mx, 149 - y0, &FONT_T, i ? "R" : "L", K_MUTED);
        }
        v[0] = 0;
        int_text(pct, v);
        text_cat(v, "%");
        cv_text(x + (46 - text_w(&FONT_T, v)) / 2, 165 - y0, &FONT_T, v, K_TEXT);
        if (lim)
            cv_rect(x + 8, 184 - y0, 30, 15, K_TEXT);
        else
            box_edge(x + 8, 184 - y0, 30, 15, 1, K_MUTED);
        cv_text(x + 8 + (30 - text_w(&FONT_T, "LIM")) / 2, 185 - y0, &FONT_T, "LIM", lim ? K_GROUND : K_MUTED);
    }
    cv_blit(0, (uint32_t)y0);
}

/* ------------------------------------------------------- the looper's lanes --- */
#define LT_X 50                                      /* the lanes' time line: from x, this wide */
#define LT_W 186

static uint32_t loop_view(void)                      /* the ticks the lanes show: the length, or as far as the first
                                                     * take has come (a bar at least) */
{
    if (lp.bars)
        return lp.bars * (uint32_t)LP_BAR;
    if (lp.first && lp.state == LP_PLAY)
        return ((lp.pos16 >> 16) / LP_BAR + 1u) * LP_BAR;
    return LP_BAR;
}

/* a part's lane at row y of the strip: its glyph and name (REC while it records), the bar lines, its notes (higher up
 * higher; a note over the loop's end goes on at its start); the chosen part's framed, a silent one's grey */
static void draw_lane(int p, int32_t y, uint32_t len, int32_t bars)
{
    static int16_t on_x[128];
    const part_set_t *ps = &set.parts[p];
    const tile_t *tl = &TILES[TILE_OF[ps->engine]];
    const lp_ev_t *e = lp_ev[p];
    int sel = p == set.part, recp = lp.rec && lp.rpart == p, solo = 0, q;
    uint16_t bg = sel ? K_SURF2 : K_SURF, col;
    uint32_t n = lp.n[p], i;
    int32_t b, lo = 127, hi = 0, top = y + 5, h = 32;
    uint8_t sh[8];
    char t[12] = "";
    for (q = 0; q < KL_NPART; q++)
        solo |= set.parts[q].solo;
    col = ps->mute || (solo && !ps->solo) ? K_MUTED : tl->col;
    cv_rect(1, y, 238, 42, bg);
    if (sel)
        box_edge(1, y, 238, 42, 2, tl->col);
    kl_shape(ps->engine, work[p][ps->engine], sh);
    glyph_draw(13, y + 13, 8, sh, tl->col, bg);
    part_name(p, t);
    text_fit(5, y + 27, 44, &FONT_T, t, sel ? tl->col : K_TEXT);
    if (recp) {
        cv_rect(26, y + 7, text_w(&FONT_T, "REC") + 6, 12, K_REC);
        cv_text(29, y + 8, &FONT_T, "REC", K_GROUND);
    }
    if (LT_W / (bars * 4) >= 6)                      /* the beats, where there is room */
        for (b = 1; b < bars * 4; b++)
            cv_rect(LT_X + b * LT_W / (bars * 4), y + 3, 1, 36, mix565(K_LINE, bg, 110));
    for (b = 1; b < bars; b++)
        cv_rect(LT_X + b * LT_W / bars, y + 3, 1, 36, K_LINE);
    for (i = 0; i < n && e[i].tick < len; i++) {
        lo = e[i].note < lo ? e[i].note : lo;
        hi = e[i].note > hi ? e[i].note : hi;
    }
    if (lo > hi) {
        if (!recp)
            cv_text(LT_X + (LT_W - text_w(&FONT_T, "EMPTY")) / 2, y + 16, &FONT_T, "EMPTY", K_MUTED);
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
    cv_begin(240, 93, K_GROUND);
    for (b = 0; b < bars; b++) {
        int32_t x = LT_X + b * LT_W / bars;
        char t[4];
        int_text(b + 1, t);
        if (b == cur) {
            cv_rect(x + 1, 23 - y0, text_w(&FONT_T, t) + 4, 11, K_TEXT);
            cv_text(x + 3, 23 - y0, &FONT_T, t, K_GROUND);
        } else if (bars <= 8 || b % 4 == 0) {
            cv_text(x + 3, 23 - y0, &FONT_T, t, K_MUTED);
        }
    }
    for (p = 0; p < KL_NPART; p++)
        draw_lane((int)p, 34 + p * 44 - y0, len, bars);
    if (lp.state == LP_PLAY)
        cv_rect(LT_X + (int32_t)(pos * (uint32_t)LT_W / len), 34 - y0, 2, 174, K_TEXT);
    cv_blit(0, (uint32_t)y0);
}

static void draw(void)
{
    draw_header();
    if (ui.mix) {
        draw_mix(22);
        draw_mix(115);
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
    if (ui.redraw_loop) {
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
