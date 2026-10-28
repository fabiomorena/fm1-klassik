/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's send effects (compiled apart at -O2): DELAY, a ping-pong delay of up to 0.5 s a side, and HALL, Rings'
 * reverb (hall/hall.c, MIT) behind PRE, a delay of up to 100 ms of its own. The app mixes the parts' sends into a mono
 * input for each and adds their stereo outputs to the master; the buffers are the app's (its pool).
 * Floats inside, the delay lines in 16 bits; outside the app's scale, where a part at its clamp (2^19) is 1.0.
 * The delay: the input goes into the left line, each line's output, through TONE's low pass and FDBK, into the other
 * one: the first echo left, the next right, each FDBK of the one before. A new time fades from the old one in 23 ms. */
#include "klassik_dsp.h"
#include "hall/hall.h"

void *memset(void *d, int c, unsigned n);       /* the FM-1 has no libc: base/bsp/chip/libc.c has this one */

#define FX_FS 44100.0f
#define IN_SCALE (1.0f / 524288.0f)             /* the app's 2^19 is 1.0 */
#define OUT_SCALE 524288.0f
#define FADE 1024                               /* a new delay time: the old one out, the new one in, in 23 ms */
#define QUIET 16                                /* an output this small (a 16-bit step of 1.0) is silence */
#define HALL_GAIN 1.0f                          /* the hall's return at LEVEL 0 dB */

_Static_assert(FX_HALL_N == HL_SIZE, "the hall's buffer is Rings' FxEngine's");

static float clip1(float v) { return v > 1.0f ? 1.0f : v < -1.0f ? -1.0f : v; }
static int16_t to16(float v) { return (int16_t)(clip1(v) * 32767.0f); }
static float from16(int16_t v) { return (float)v * (1.0f / 32768.0f); }

static float fx_exp2(float x)                   /* 2^x, |x| < 30: the fraction by its series (1e-4 off), the whole
                                                 * part by halving or doubling (the main loop's, at a knob's turn) */
{
    float r = 1.0f;
    while (x < 0.0f) {
        x += 1.0f;
        r *= 0.5f;
    }
    while (x >= 1.0f) {
        x -= 1.0f;
        r *= 2.0f;
    }
    return r * (1.0f + x * (0.6931472f + x * (0.2402265f + x * (0.0555041f + x * 0.0096181f))));
}

static struct {
    int16_t *buf[2];                            /* L, R: FX_DLY_N samples each */
    uint32_t w;                                 /* where the next sample goes */
    uint32_t d, d_old, fade;                    /* the time playing (samples), the one fading out, the fade left */
    volatile uint32_t d_tgt;                    /* the main loop's */
    volatile float fb, a;                       /* FDBK, TONE's low pass coefficient: the main loop's */
    float lp[2];
    uint32_t quiet;                             /* samples its output has been silent */
    uint8_t awake;
} dl;

static struct {
    int16_t *pre;                               /* FX_PRE_N samples */
    uint32_t w;
    volatile uint32_t d;                        /* PRE, samples: the main loop's */
    volatile float time, lp;                    /* Rings' set_time and set_lp: the main loop's */
    hl_reverb_t r;
    uint32_t quiet;
    uint8_t awake;
} hl;

void fx_init(int16_t *dly, int16_t *pre, uint16_t *hall)
{
    memset(&dl, 0, sizeof dl);
    memset(&hl, 0, sizeof hl);
    dl.buf[0] = dly;
    dl.buf[1] = dly + FX_DLY_N;
    memset(dly, 0, 2u * FX_DLY_N * sizeof *dly);
    dl.d = dl.d_tgt = FX_DLY_N;
    hl.pre = pre;
    memset(pre, 0, FX_PRE_N * sizeof *pre);
    hl_init(&hl.r, hall, FX_FS);
    hl.r.amount = 1.0f;                         /* a send effect: the reverb alone */
    hl.r.input_gain = 0.2f;                     /* as Rings */
    hl.r.diffusion = 0.625f;
    fx_delay_set(FX_DLY_N, 0, 100);
    fx_hall_set(0, 50, 50);
}

/* TONE: a one-pole low pass at 500 Hz * 32^(tone / 100), 500 Hz .. 16 kHz: a = 1 - e^(-2 pi fc / fs) */
void fx_delay_set(uint32_t d, int fdbk, int tone)
{
    float fc = 500.0f * fx_exp2((float)tone * 0.05f);
    dl.a = 1.0f - fx_exp2(-6.2831853f * fc / FX_FS * 1.4426950f);
    dl.fb = (float)fdbk * 0.0098f;              /* 100 %: 0.98, the echoes die away */
    dl.d_tgt = d < 1u ? 1u : d > FX_DLY_N ? FX_DLY_N : d;
}

void fx_hall_set(uint32_t pre, int decay, int tone)   /* DECAY and TONE as Rings maps its damping and brightness */
{
    hl.time = 0.35f + 0.0063f * (float)decay;
    hl.lp = 0.3f + 0.006f * (float)tone;
    hl.d = pre > FX_PRE_N ? FX_PRE_N : pre;
}

int fx_awake(int which) { return which ? hl.awake : dl.awake; }

static uint32_t behind(uint32_t w, uint32_t d, uint32_t n) { return w >= d ? w - d : w + n - d; }

static void delay_render(const int32_t *in, int32_t *out, uint32_t n)
{
    int16_t *bl = dl.buf[0], *br = dl.buf[1];
    float fb = dl.fb, a = dl.a, lpl = dl.lp[0], lpr = dl.lp[1];
    uint32_t i, w = dl.w, peak = 0;
    if (dl.d_tgt != dl.d && !dl.fade) {         /* a new time: fade over to it */
        dl.d_old = dl.d;
        dl.d = dl.d_tgt;
        dl.fade = FADE;
    }
    for (i = 0; i < n; i++) {
        uint32_t r = behind(w, dl.d, FX_DLY_N), al, ar;
        float yl = from16(bl[r]), yr = from16(br[r]);
        int32_t ol, or_;
        if (dl.fade) {
            uint32_t ro = behind(w, dl.d_old, FX_DLY_N);
            float t = (float)dl.fade * (1.0f / FADE);
            yl += (from16(bl[ro]) - yl) * t;
            yr += (from16(br[ro]) - yr) * t;
            dl.fade--;
        }
        lpl += a * (yl - lpl);
        lpr += a * (yr - lpr);
        bl[w] = to16((float)in[i] * IN_SCALE + fb * lpr);   /* the input and the right's echo into the left */
        br[w] = to16(fb * lpl);                 /* the left's echo into the right */
        if (++w == FX_DLY_N)
            w = 0;
        ol = (int32_t)(yl * OUT_SCALE);
        or_ = (int32_t)(yr * OUT_SCALE);
        out[2 * i] = ol;
        out[2 * i + 1] = or_;
        al = (uint32_t)(ol < 0 ? -ol : ol);
        ar = (uint32_t)(or_ < 0 ? -or_ : or_);
        peak |= al | ar;
    }
    dl.w = w;
    dl.lp[0] = lpl;
    dl.lp[1] = lpr;
    dl.quiet = peak < QUIET ? dl.quiet + n : 0;
}

static void hall_render(const int32_t *in, int32_t *out, uint32_t n)
{
    float l[32], r[32];
    uint32_t b, i, peak = 0;
    hl.r.reverb_time = hl.time;
    hl.r.lp = hl.lp;
    for (b = 0; b < n; b += 32u) {              /* 32 at a time: the reverb's buffers on the stack */
        uint32_t m = n - b < 32u ? n - b : 32u, d = hl.d;
        for (i = 0; i < m; i++) {               /* PRE */
            float x = (float)in[b + i] * IN_SCALE;
            if (d) {
                uint32_t rd = behind(hl.w, d, FX_PRE_N);
                l[i] = from16(hl.pre[rd]);
                hl.pre[hl.w] = to16(x);
                if (++hl.w == FX_PRE_N)
                    hl.w = 0;
            } else {
                l[i] = x;
            }
            r[i] = 0.0f;
        }
        hl_process(&hl.r, l, r, m);
        for (i = 0; i < m; i++) {
            int32_t ol = (int32_t)(l[i] * (HALL_GAIN * OUT_SCALE)), or_ = (int32_t)(r[i] * (HALL_GAIN * OUT_SCALE));
            out[2 * (b + i)] = ol;
            out[2 * (b + i) + 1] = or_;
            peak |= (uint32_t)(ol < 0 ? -ol : ol) | (uint32_t)(or_ < 0 ? -or_ : or_);
        }
    }
    hl.quiet = peak < QUIET ? hl.quiet + n : 0;
}

/* an effect runs while a part sends to it (on), and after until its output has been silent longer than anything in
 * its lines takes to come out (the delay: a whole line; the hall: its buffer, more than a round of its loops): then
 * every line is silent too, and it sleeps (no cost) */
void fx_render(const int32_t *dly_in, const int32_t *hall_in, int32_t *dly_out, int32_t *hall_out, uint32_t n,
               int dly_on, int hall_on)
{
    uint32_t i;
    if (dly_on)
        dl.awake = 1;
    if (dl.awake) {
        delay_render(dly_in, dly_out, n);
        if (!dly_on && dl.quiet > FX_DLY_N)
            dl.awake = 0;
    } else {
        for (i = 0; i < 2u * n; i++)
            dly_out[i] = 0;
    }
    if (hall_on)
        hl.awake = 1;
    if (hl.awake) {
        hall_render(hall_in, hall_out, n);
        if (!hall_on && hl.quiet > FX_HALL_N + FX_PRE_N)
            hl.awake = 0;
    } else {
        for (i = 0; i < 2u * n; i++)
            hall_out[i] = 0;
    }
}
