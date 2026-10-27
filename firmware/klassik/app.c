/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena
 * From fm1-base's template, Copyright (C) 2026 Charles Vestal (MPL-2.0) */
/* KLASSIK v0.3: the rack and its edit pages. Six engines, FM6 (msfa's six operators), three of Felucca-Engines
 * (ANALOG, PHASE, WHEEL), DRUMS (four kits on Plaits' drum models) and ACID (Open303's TB-303), play from the keys
 * and from MIDI; K1 picks the engine (in the rack's order), PRESET its sound, OCT- / OCT+ the octave. On ACID a key
 * pressed while ENV is held is accented (ACC in the header). EDIT opens the engine's two pages of four parameters
 * on K1..K4 (EDIT or OCT+ the other page, OCT- back to the rack); an edit plays at once (FM6's MOD, DTUN, ATK, DEC,
 * SUS and REL and ACID's DECAY and ACCENT from the next note). HOME held, a knob turned goes back to its sound's
 * value (the preset's, or the USER sound's), marked on its bar while it is elsewhere.
 * SAVE keeps the engine, the sounds and the octave across a power cycle, an edited sound as its engine's USER sound.
 * The screen: the header (the engine, the sound's glyph and name, the octave, the battery), then the six engine
 * tiles or an edit page (its tabs, the sound's glyph large, its spokes named; DRUMS:
 * the octave's keys as pads, each with its instrument), and a row of what the knobs hold.
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
#define SET_MAGIC 0x4B4C5335u                        /* "KLS5": another magic (DRUMS' KLS4, v0.2's KLS3, v0.1's KLS2,
                                                     * v0's KLS1) loads defaults */
typedef struct {
    int8_t v[8];                                     /* the eight parameters */
    uint8_t base;                                    /* the preset it was made from: its envelopes and MONO */
    uint8_t saved;                                   /* 1 = there is one */
} user_t;
typedef struct {
    uint32_t magic;
    uint8_t engine;                                  /* KL_* */
    uint8_t preset[KL_NENG];                         /* each engine's last sound; kl_presets(e) is its USER sound */
    int8_t octave;                                   /* -2 .. +2 */
    uint8_t rsv[3];
    user_t user[KL_NENG];                            /* each engine's own sound (SAVE after an edit) */
} settings_t;
_Static_assert(sizeof(settings_t) <= PLAT_STORE_MAX, "the settings are one store object");
static settings_t set, set_kept;                     /* as it is, and as the store has it (or the defaults at boot) */
static uint8_t set_dirty;

/* each engine's sound as it plays: its preset's values or its USER sound's (orig), and whether EDIT has any of them
 * elsewhere (not saved) */
static int8_t work[KL_NENG][8], orig[KL_NENG][8];
static uint8_t edited[KL_NENG];

static void settings_defaults(void)
{
    uint32_t i;
    uint8_t *b = (uint8_t *)&set;
    for (i = 0; i < sizeof set; i++)                 /* no USER sounds, every engine at its first preset */
        b[i] = 0;
    set.magic = SET_MAGIC;
    set.engine = KL_ANALOG;
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
    uint32_t i;
    if (set.magic != SET_MAGIC || set.engine >= KL_NENG || set.octave < -2 || set.octave > 2)
        return 0;
    for (i = 0; i < KL_NENG; i++) {
        const user_t *u = &set.user[i];
        if (u->saved > 1 || set.preset[i] >= kl_presets((int)i) + u->saved)
            return 0;
        if (u->saved && (u->base >= kl_presets((int)i) || !values_ok((int)i, u->v)))
            return 0;
    }
    return 1;
}

static int is_user(int e) { return set.preset[e] >= kl_presets(e); }
static int sound_base(int e) { return is_user(e) ? set.user[e].base : set.preset[e]; }   /* its preset */
static int sound_count(int e) { return kl_presets(e) + set.user[e].saved; }   /* PRESET's choices */
static const char *sound_name(int e) { return is_user(e) ? "USER" : kl_preset_name(e, set.preset[e]); }

static void sound_load(int e)                        /* the engine's sound as chosen: an edit is forgotten */
{
    uint32_t k;
    if (is_user(e))
        for (k = 0; k < 8u; k++)
            work[e][k] = set.user[e].v[k];
    else
        kl_preset_values(e, set.preset[e], work[e]);
    for (k = 0; k < 8u; k++)
        orig[e][k] = work[e][k];
    edited[e] = 0;
}

static void edited_update(int e)                     /* edited while a value is not its sound's */
{
    uint32_t k;
    edited[e] = 0;
    for (k = 0; k < 8u; k++)
        edited[e] |= work[e][k] != orig[e][k];
}

static void dirty_update(void)                       /* SAVE lit while the settings or a sound differ from the store */
{
    uint32_t i;
    const uint8_t *a = (const uint8_t *)&set, *b = (const uint8_t *)&set_kept;
    set_dirty = 0;
    for (i = 0; i < sizeof set; i++)
        set_dirty |= a[i] != b[i];
    for (i = 0; i < KL_NENG; i++)
        set_dirty |= edited[i];
}

static void sound_select(void) { kl_select(set.engine, sound_base(set.engine), work[set.engine]); }

/* ------------------------------------------------------------------ audio --- */
static int32_t mono[APP_BLOCK];
static int32_t gain_q8;                              /* MASTER's gain, gliding: no step at a block's edge */
static volatile uint32_t midi_seen;

/* (a * b) >> 7 in 64 bits, out of line: an inlined 64-bit multiply beside loads now and then leaves a register a bit
 * off on the FM-1 (base/bsp/docs/CPU.md) */
static __attribute__((noinline)) int64_t mul_shr7(int32_t a, int32_t b) { return ((int64_t)a * b) >> 7; }

static void midi_in(void)                            /* audio interrupt: the packets that came in */
{
    uint32_t pkt;
    int port;
    while (plat_midi_in(&pkt, &port)) {
        uint32_t st = (pkt >> 8) & 0xFFu, d1 = (pkt >> 16) & 0x7Fu, d2 = (pkt >> 24) & 0x7Fu;
        if ((st & 0xF0u) == 0x90u && d2)
            kl_key_now(1, (int)d1, (int)d2);
        else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u)
            kl_key_now(0, (int)d1, 0);
        if (st != 0xF8u && st != 0xFEu)
            midi_seen++;
    }
}

void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    uint32_t i;
    int32_t target = (int32_t)master_q12 << 8;
    if (n > APP_BLOCK)
        n = APP_BLOCK;
    midi_in();
    for (i = 0; i < n; i++)
        mono[i] = 0;
    kl_render(mono, n);
    for (i = 0; i < n; i++) {
        int64_t s;
        gain_q8 += (target - gain_q8) >> 8;          /* ~6 ms */
        s = mul_shr7(mono[i], gain_q8 >> 8);         /* x32 at full MASTER, toward 24-bit */
        if (s > 8388607)
            s = 8388607;
        else if (s < -8388607)
            s = -8388607;
        lr[2 * i] = (int32_t)s;
        lr[2 * i + 1] = (int32_t)s;
    }
}

void app_panic(void) { kl_silence(); }

void app_init(void)
{
    int e;
    if (plat_store_load(OBJ_SETTINGS, &set, sizeof set) != (int)sizeof set || !settings_ok())
        settings_defaults();
    set_kept = set;
    for (e = 0; e < KL_NENG; e++)
        sound_load(e);
    sound_select();
}

/* --------------------------------------------------------------------- UI --- */
static const char *const BTN_NAME[NB] = {"FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ",
                                         "PLAY", "REC", "OCT-", "OCT+"};
static struct {
    char msg[2][24];                                 /* a short message over the header */
    uint32_t msg_until;
    uint32_t keys, btn;
    int8_t knote[NKEYS];                             /* the note each held key started (the octave may change) */
    uint8_t edit, page;                              /* an edit page is open, which (0: EDIT 1, 1: EDIT 2) */
    uint8_t redraw, redraw_pads;                     /* the whole screen; DRUMS' pads only (a hit lit or gone) */
    uint32_t hits, hit_until;                        /* the DSP's note count as last seen; the hit pad lit until */
    int8_t hit_pc;                                   /* the last note's semitone (C = 0) */
} ui;

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

static void octave_text(char *t)
{
    t[0] = set.octave < 0 ? '-' : '+';
    t[1] = (char)('0' + (set.octave < 0 ? -set.octave : set.octave));
    t[2] = 0;
}

static void sound_title(int e, char *t)              /* its name, with "*" while an edit is not saved */
{
    t[0] = 0;
    text_cat(t, sound_name(e));
    if (edited[e])
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
    kl_param(set.engine, k, work[set.engine], &d);
    value_text(&d, work[set.engine][k], v);
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

/* a screen reader that has just started listening (or asks) hears the whole screen */
static void speak(void)
{
    char t[192], o[4];
    int32_t k;
    if (ann_on() && ann_where()) {
        octave_text(o);
        t[0] = 0;
        text_cat(t, APP_NAME ". ENGINE ");
        text_cat(t, TILES[TILE_OF[set.engine]].name);
        text_cat(t, ". SOUND ");
        text_cat(t, sound_name(set.engine));
        if (edited[set.engine])
            text_cat(t, ", EDITED");
        text_cat(t, ". OCTAVE ");
        text_cat(t, o);
        if (ui.edit) {                               /* the page and what its knobs hold */
            text_cat(t, ". EDIT ");
            text_cat(t, kl_page_title(set.engine, ui.page));
            for (k = 0; k < 4; k++) {
                text_cat(t, ". ");
                param_cat(t, ui.page * 4 + k);
            }
        }
        ann_say(t, 0, ANN_NOW);
    }
    ann_tick(plat_ms());
}

static void engine_step(int32_t d)
{
    int32_t e = (engine_pos(set.engine) + d) % KL_NENG;
    char t[48];
    if (e < 0)
        e += KL_NENG;
    e = engine_at(e);
    set.engine = (uint8_t)e;
    set_dirty = 1;
    sound_select();
    ui.redraw = 1;
    t[0] = 0;
    text_cat(t, "ENGINE ");
    text_cat(t, TILES[TILE_OF[e]].name);
    text_cat(t, ". ");
    text_cat(t, sound_name(e));
    announce(t);
}

static void preset_step(int32_t d)
{
    int e = set.engine;
    int32_t n = sound_count(e), p = ((int32_t)set.preset[e] + d) % n;
    if (p < 0)
        p += n;
    set.preset[e] = (uint8_t)p;
    sound_load(e);
    set_dirty = 1;
    sound_select();
    ui.redraw = 1;
    announce(sound_name(e));
}

static void page_show(uint32_t page)                 /* EDIT 1 (0) or EDIT 2 (1) of the engine playing */
{
    char t[24] = "EDIT ";
    ui.edit = 1;
    ui.page = (uint8_t)page;
    ui.redraw = 1;
    text_cat(t, kl_page_title(set.engine, (int)page));
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
    int e = set.engine;
    kl_param_t p;
    int32_t v;
    char t[32];
    kl_param(e, k, work[e], &p);
    v = clamp(work[e][k] + d, p.min, p.max);
    if (v == work[e][k])
        return;
    work[e][k] = (int8_t)v;
    edited_update(e);
    dirty_update();
    sound_select();
    ui.redraw = 1;
    t[0] = 0;
    param_cat(t, k);
    announce(t);
}

static void param_reset(int32_t k)                   /* HOME + K1..K4 on an edit page: back to its sound's value */
{
    int e = set.engine;
    char t[32];
    if (work[e][k] == orig[e][k])
        return;
    work[e][k] = orig[e][k];
    edited_update(e);
    dirty_update();
    sound_select();
    ui.redraw = 1;
    t[0] = 0;
    param_cat(t, k);
    announce(t);
}

static void save(void)                               /* each edited sound becomes its engine's USER sound */
{
    uint32_t e, k, users = 0;
    for (e = 0; e < KL_NENG; e++) {
        user_t *u = &set.user[e];
        if (!edited[e])
            continue;
        for (k = 0; k < 8u; k++)
            u->v[k] = orig[e][k] = work[e][k];
        u->base = (uint8_t)sound_base((int)e);
        u->saved = 1;
        set.preset[e] = (uint8_t)kl_presets((int)e);
        edited[e] = 0;
        users++;
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
    int32_t v = clamp(set.octave + d, -2, 2);
    if (v == set.octave)
        return;
    set.octave = (int8_t)v;
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
            int32_t note = clamp(53 + (int32_t)k + 12 * set.octave, 0, 127);
            uint32_t v = touch_vel(k);
            if (set.engine == KL_ACID)               /* its accent is ENV held, as Open303's: velocity 100 or more */
                v = btn & 1u << B_ENV ? 127u : 64u;
            ui.knote[k] = (int8_t)note;
            kl_key(1, note, (int)v);
            plat_midi_send(0x09u | 0x90u << 8 | (uint32_t)note << 16 | v << 24, TO_USB);
        } else if (!(keys & bit) && (ui.keys & bit)) {
            int32_t note = ui.knote[k];
            kl_key(0, note, 0);
            plat_midi_send(0x08u | 0x80u << 8 | (uint32_t)note << 16, TO_USB);
        }
    }
    for (k = 0; k < NB; k++) {
        uint32_t bit = 1u << k;
        if (!(btn & bit) || (ui.btn & bit))
            continue;
        if (k == B_OCTDN && ui.edit)
            edit_close();                            /* back, as on Felucca's pages */
        else if (k == B_OCTDN)
            octave_step(-1);
        else if (k == B_EDIT || (k == B_OCTUP && ui.edit))
            page_show(ui.edit ? ui.page ^ 1u : 0u);  /* the edit pages, then the other one */
        else if (k == B_OCTUP)
            octave_step(1);
        else if (k == B_SAVE)
            save();
        else if (k == B_ENV && set.engine == KL_ACID)
            announce("ACCENT");                      /* held: the keys pressed now are accented */
        else if (k == B_HOME && ui.edit)
            app_say("RESET", "TURN K1-K4");          /* held: a knob turned goes back to its sound's value */
        else
            app_say(BTN_NAME[k], "SOON");            /* the pages behind the other buttons come later */
    }
    for (k = 0; k < NE; k++) {                       /* every one read, so none piles up */
        if ((e = plat_enc((int)k)) == 0)
            continue;
        if (k == EN_PRESET)
            preset_step(e);
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
    plat_leds(btn | (set_dirty ? 1u << B_SAVE : 0u), keys);
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

static int acc_on(void) { return set.engine == KL_ACID && (ui.btn & 1u << B_ENV); }

static void draw_header(void)
{
    const tile_t *tl = &TILES[TILE_OF[set.engine]];
    char o[8] = "OCT ", t[4];
    cv_begin(240, 22, K_HEAD);
    cv_rect(0, 21, 240, 1, K_LINE);
    if (plat_ms() < ui.msg_until) {
        int32_t x = cv_text(8, 3, &FONT_B, ui.msg[0], K_TEXT);
        cv_text(x + 6, 3, &FONT_B, ui.msg[1], tl->col);
    } else {
        uint8_t sh[8];
        char name[24];
        int32_t w = text_w(&FONT_XS, tl->name) + 8;
        cv_rect(6, 4, w, 15, tl->col);
        cv_text(10, 4, &FONT_XS, tl->name, K_GROUND);
        kl_shape(set.engine, work[set.engine], sh);  /* the sound as it plays: an edit shows at once */
        glyph_draw(6 + w + 11, 11, 8, sh, tl->col, K_HEAD);
        sound_title(set.engine, name);
        text_fit(6 + w + 23, 3, (acc_on() ? 124 : 150) - w, &FONT_B, name, K_TEXT);
    }
    octave_text(t);
    text_cat(o, t);
    cv_text(206 - text_w(&FONT_XS, o), 4, &FONT_XS, o, K_MUTED);
    if (acc_on()) {                                  /* ACID's accent held */
        int32_t x = 202 - text_w(&FONT_XS, o) - text_w(&FONT_T, "ACC") - 6;
        box_edge(x, 5, text_w(&FONT_T, "ACC") + 6, 13, 1, tl->col);
        cv_text(x + 3, 6, &FONT_T, "ACC", tl->col);
    }
    draw_battery(214, 6);
    cv_blit(0, 0);
}

static void draw_row(int32_t row)                    /* three tiles, 93 rows high */
{
    int32_t c;
    cv_begin(240, 93, K_GROUND);
    for (c = 0; c < 3; c++) {
        const tile_t *tl = &TILES[row * 3 + c];
        int32_t x = 6 + c * 77, y = 2, w = 73, h = 89;
        int sel = tl->eng >= 0 && tl->eng == set.engine;
        uint16_t bg = sel ? K_SURF2 : K_SURF;
        cv_rect(x, y, w, h, bg);
        if (sel)
            box_edge(x, y, w, h, 2, tl->col);
        if (tl->eng >= 0) {
            uint8_t sh[8];
            char name[24];
            kl_shape(tl->eng, work[tl->eng], sh);
            glyph_draw(x + 23, y + 21, 18, sh, tl->col, bg);
            cv_text(x + 6, y + 40, &FONT_B, tl->name, sel ? tl->col : K_TEXT);
            cv_text(x + 6, y + 57, &FONT_T, tl->sub, K_MUTED);
            sound_title(tl->eng, name);
            text_fit(x + 6, y + 71, w - 10, &FONT_T, name, K_TEXT);
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
    int e = set.engine;
    int32_t n = sound_count(e), p = set.preset[e];
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
    if (is_user(e)) {
        text_cat(v[1], "USER");
    } else {
        two_digits(p + 1, v[1]);
        text_cat(v[1], "/");
        two_digits(kl_presets(e), t);
        text_cat(v[1], t);
    }
    frac[1] = n > 1 ? p * 256 / (n - 1) : 256;
    octave_text(v[2]);
    frac[2] = (set.octave + 2) * 64;
    int_text((int32_t)pct, v[3]);
    text_cat(v[3], "%");
    frac[3] = (int32_t)pct * 256 / 100;
}

/* K1..K4: the page's parameters, each marked where its sound has it while it is elsewhere */
static void footer_edit(char lab[4][16], char v[4][12], int32_t frac[4], int32_t mark[4])
{
    int e = set.engine;
    int32_t i;
    for (i = 0; i < 4; i++) {
        int32_t k = ui.page * 4 + i, x = work[e][k], o = orig[e][k];
        kl_param_t d;
        kl_param(e, k, work[e], &d);
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

static void draw_footer(void)                        /* what the controls hold, each with a bar of where it is */
{
    const tile_t *tl = &TILES[TILE_OF[set.engine]];
    char lab[4][16], v[4][12];
    int32_t frac[4], mark[4], i;
    if (ui.edit)
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
    const char *name = kl_drum_name(sound_base(KL_DRUMS), pc);
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
    const tile_t *tl = &TILES[TILE_OF[set.engine]];
    int e = set.engine;
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
    kl_shape(e, work[e], sh);
    ring_draw(EDIT_CX, cy, 60, K_LINE);
    glyph_draw(EDIT_CX, cy, 56, sh, tl->col, K_GROUND);
    for (i = 0; i < 8; i++) {                        /* a name beyond each spoke's end, away from the centre */
        kl_param_t d;
        int32_t lx = EDIT_CX + GDIR[i][0] * 66 / 256, ly = cy + GDIR[i][1] * 66 / 256, w;
        kl_param(e, (int)i, work[e], &d);
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

static void draw(void)
{
    draw_header();
    if (ui.edit) {
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
    if ((hits = kl_hits(&note)) != ui.hits) {        /* a note played: DRUMS lights its pad a moment */
        ui.hits = hits;
        ui.hit_pc = (int8_t)(note % 12);
        ui.hit_until = now + 250u;
        ui.redraw_pads = ui.edit && set.engine == KL_DRUMS;
    } else if (ui.hit_until && now >= ui.hit_until) {
        ui.hit_until = 0;
        ui.redraw_pads = ui.edit && set.engine == KL_DRUMS;
    }
    if (pct != last_pct || (ui.msg_until && now >= ui.msg_until)) {
        last_pct = pct;
        if (now >= ui.msg_until)
            ui.msg_until = 0;
        ui.redraw = 1;
    }
    if (ui.redraw) {
        ui.redraw = ui.redraw_pads = 0;
        draw();
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
    if (!strcmp(what, "engine"))
        *got = set.engine;
    else if (!strcmp(what, "preset"))
        *got = set.preset[set.engine];
    else if (!strcmp(what, "octave"))
        *got = set.octave;
    else if (!strcmp(what, "voices"))
        *got = kl_voices();
    else if (!strcmp(what, "dirty"))
        *got = set_dirty;
    else if (!strcmp(what, "edit"))
        *got = ui.edit;
    else if (!strcmp(what, "page"))
        *got = ui.page + 1;
    else if (!strcmp(what, "edited"))
        *got = edited[set.engine];
    else if (!strcmp(what, "user"))
        *got = set.user[set.engine].saved;
    else if (!strcmp(what, "note"))                  /* ACID: the note its voice plays, -1 none */
        *got = kl_note_now();
    else if (!strcmp(what, "pad"))                   /* DRUMS: the lit pad's semitone, -1 none */
        *got = ui.hit_until ? ui.hit_pc : -1;
    else if ((k = expect_k(what, "v")) >= 0)
        *got = work[set.engine][k];
    else if ((k = expect_k(what, "dsp")) >= 0)
        *got = kl_value_now(k);
    else if ((k = expect_k(what, "glyph")) >= 0) {
        uint8_t sh[8];
        kl_shape(set.engine, work[set.engine], sh);
        *got = sh[k];
    } else
        return 0;
    return 1;
}
#endif
