/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena
 * From fm1-base's template, Copyright (C) 2026 Charles Vestal (MPL-2.0) */
/* KLASSIK v0: the rack. Three engines of Felucca-Engines (ANALOG, PHASE, WHEEL) play from the keys and from MIDI;
 * K1 picks the engine, PRESET its sound, OCT- / OCT+ the octave, SAVE keeps them across a power cycle.
 * The screen: the header (the engine, the sound's glyph and name, the octave, the battery), six engine tiles
 * (three playing, three to come) and a row of what the knobs hold.
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
    {"FM6", "6-OP FM", -1, RGB(92, 200, 255)},
    {"VA", "ANALOG", KL_ANALOG, RGB(255, 180, 84)},
    {"PD", "PHASE DIST", KL_PHASE, RGB(255, 122, 182)},
    {"ORGAN", "DRAWBAR", KL_WHEEL, RGB(126, 224, 138)},
    {"STRINGS", "ENSEMBLE", -1, RGB(185, 162, 255)},
    {"ACID", "MONO BASS", -1, RGB(227, 244, 94)},
};
static const uint8_t TILE_OF[KL_NENG] = {1, 2, 3};   /* engine -> its tile */

/* -------------------------------------------------------------- settings --- */
#define SET_MAGIC 0x4B4C5331u                        /* "KLS1": another magic loads defaults */
typedef struct {
    uint32_t magic;
    uint8_t engine;                                  /* KL_* */
    uint8_t preset[KL_NENG];                         /* each engine's last sound */
    int8_t octave;                                   /* -2 .. +2 */
    uint8_t rsv[3];
} settings_t;
static settings_t set;
static uint8_t set_dirty;

static void settings_defaults(void)
{
    uint32_t i;
    set.magic = SET_MAGIC;
    set.engine = KL_ANALOG;
    for (i = 0; i < KL_NENG; i++)
        set.preset[i] = 0;
    set.octave = 0;
    for (i = 0; i < sizeof set.rsv; i++)
        set.rsv[i] = 0;
}

static int settings_ok(void)
{
    uint32_t i;
    if (set.magic != SET_MAGIC || set.engine >= KL_NENG || set.octave < -2 || set.octave > 2)
        return 0;
    for (i = 0; i < KL_NENG; i++)
        if (set.preset[i] >= kl_presets((int)i))
            return 0;
    return 1;
}

static void sound_select(void) { kl_select(set.engine, set.preset[set.engine]); }

/* ------------------------------------------------------------------ audio --- */
static int32_t mono[APP_BLOCK];
static int32_t gain_q8;                              /* MASTER's gain, gliding: no step at a block's edge */
static volatile uint32_t midi_seen;

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
        s = ((int64_t)mono[i] * (gain_q8 >> 8)) >> 7;   /* x32 at full MASTER, toward 24-bit */
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
    if (plat_store_load(OBJ_SETTINGS, &set, sizeof set) != (int)sizeof set || !settings_ok())
        settings_defaults();
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
    uint8_t redraw;
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
    char t[96], o[4];
    if (ann_on() && ann_where()) {
        octave_text(o);
        t[0] = 0;
        text_cat(t, APP_NAME ". ENGINE ");
        text_cat(t, TILES[TILE_OF[set.engine]].name);
        text_cat(t, ". SOUND ");
        text_cat(t, kl_preset_name(set.engine, set.preset[set.engine]));
        text_cat(t, ". OCTAVE ");
        text_cat(t, o);
        ann_say(t, 0, ANN_NOW);
    }
    ann_tick(plat_ms());
}

static void engine_step(int32_t d)
{
    int32_t e = ((int32_t)set.engine + d) % KL_NENG;
    char t[48];
    if (e < 0)
        e += KL_NENG;
    set.engine = (uint8_t)e;
    set_dirty = 1;
    sound_select();
    ui.redraw = 1;
    t[0] = 0;
    text_cat(t, "ENGINE ");
    text_cat(t, TILES[TILE_OF[e]].name);
    text_cat(t, ". ");
    text_cat(t, kl_preset_name(e, set.preset[e]));
    announce(t);
}

static void preset_step(int32_t d)
{
    int32_t n = kl_presets(set.engine), p = ((int32_t)set.preset[set.engine] + d) % n;
    if (p < 0)
        p += n;
    set.preset[set.engine] = (uint8_t)p;
    set_dirty = 1;
    sound_select();
    ui.redraw = 1;
    announce(kl_preset_name(set.engine, p));
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
        if (k == B_OCTDN)
            octave_step(-1);
        else if (k == B_OCTUP)
            octave_step(1);
        else if (k == B_SAVE) {
            if (plat_store_save(OBJ_SETTINGS, &set, sizeof set) == 0) {
                set_dirty = 0;
                app_say("SAVED", "");
            } else {
                app_say("SAVE", "FAILED");
            }
        } else {
            app_say(BTN_NAME[k], "SOON");            /* the pages behind the other buttons come later */
        }
    }
    if ((e = plat_enc(EN_K1)) != 0)
        engine_step(e);
    if ((e = plat_enc(EN_PRESET)) != 0)
        preset_step(e);
    for (k = 0; k < NE; k++)                         /* the others: read, so they do not pile up */
        if (k != EN_K1 && k != EN_PRESET)
            (void)plat_enc((int)k);
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
        int32_t w = text_w(&FONT_XS, tl->name) + 8;
        cv_rect(6, 4, w, 15, tl->col);
        cv_text(10, 4, &FONT_XS, tl->name, K_GROUND);
        kl_shape(set.engine, set.preset[set.engine], sh);
        glyph_draw(6 + w + 11, 11, 8, sh, tl->col, K_HEAD);
        text_fit(6 + w + 23, 3, 150 - w, &FONT_B, kl_preset_name(set.engine, set.preset[set.engine]), K_TEXT);
    }
    octave_text(t);
    text_cat(o, t);
    cv_text(206 - text_w(&FONT_XS, o), 4, &FONT_XS, o, K_MUTED);
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
            int32_t p = set.preset[tl->eng];
            kl_shape(tl->eng, p, sh);
            glyph_draw(x + 23, y + 21, 18, sh, tl->col, bg);
            cv_text(x + 6, y + 40, &FONT_B, tl->name, sel ? tl->col : K_TEXT);
            cv_text(x + 6, y + 57, &FONT_T, tl->sub, K_MUTED);
            text_fit(x + 6, y + 71, w - 10, &FONT_T, kl_preset_name(tl->eng, p), K_TEXT);
        } else {
            cv_rect(x + 6, y + 20, 30, 3, mix565(tl->col, bg, 110));
            cv_text(x + 6, y + 40, &FONT_B, tl->name, K_MUTED);
            cv_text(x + 6, y + 57, &FONT_T, tl->sub, K_MUTED);
            cv_text(x + 6, y + 71, &FONT_T, "SOON", mix565(tl->col, bg, 200));
        }
    }
    cv_blit(0, 22 + row * 93);
}

static void draw_footer(void)                        /* what the controls hold, each with a bar of where it is */
{
    const tile_t *tl = &TILES[TILE_OF[set.engine]];
    char v[4][12], t[8];
    int32_t frac[4], i;
    int32_t n = kl_presets(set.engine), p = set.preset[set.engine];
    uint32_t pct = plat_master() * 100u / 4096u;
    static const char *const LAB[4] = {"K1 ENGINE", "PRESET", "OCTAVE", "MASTER"};
    v[0][0] = 0;
    text_cat(v[0], tl->name);
    frac[0] = set.engine * 256 / (KL_NENG - 1);
    two_digits(p + 1, v[1]);
    text_cat(v[1], "/");
    two_digits(n, t);
    text_cat(v[1], t);
    frac[1] = n > 1 ? p * 256 / (n - 1) : 256;
    octave_text(v[2]);
    frac[2] = (set.octave + 2) * 64;
    int_text((int32_t)pct, v[3]);
    text_cat(v[3], "%");
    frac[3] = (int32_t)pct * 256 / 100;
    cv_begin(240, 32, K_HEAD);
    cv_rect(0, 0, 240, 1, K_LINE);
    for (i = 0; i < 4; i++) {
        int32_t x = i * 60;
        if (i)
            cv_rect(x, 1, 1, 31, K_LINE);
        cv_text(x + 6, 3, &FONT_T, LAB[i], K_MUTED);
        cv_text(x + 6, 13, &FONT_XS, v[i], K_TEXT);
        cv_rect(x + 6, 28, 48, 2, K_LINE);
        cv_rect(x + 6, 28, 48 * clamp(frac[i], 0, 256) / 256, 2, tl->col);
    }
    cv_blit(0, 208);
}

static void draw(void)
{
    draw_header();
    draw_row(0);
    draw_row(1);
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
    input();
    speak();
    if (pct != last_pct || (ui.msg_until && now >= ui.msg_until)) {
        last_pct = pct;
        if (now >= ui.msg_until)
            ui.msg_until = 0;
        ui.redraw = 1;
    }
    if (ui.redraw) {
        ui.redraw = 0;
        draw();
    }
}

#ifdef PLAT_HOST
int app_expect(const char *what, int *got)
{
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
    else
        return 0;
    return 1;
}
#endif
