#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2009 Robin Schmidt (www.rs-met.com) (Open303: rosic_MipMappedWaveTable.cpp, rosic_Open303.cpp,
#   rosic_OnePoleFilter.cpp, rosic_BiquadFilter.cpp, rosic_LeakyIntegrator.cpp, rosic_AnalogEnvelope.cpp,
#   rosic_DecayEnvelope.cpp, rosic_EllipticQuarterBandFilter.h)
# Changed 2026 by fabiomorena: what Open303 computes at start-up in double, as a generator that writes it as a C header
#   at build time (the FM-1 has no double): the wave tables as int16, the higher ones shorter; the anti-alias filter
#   as second-order sections; the fixed filters' and envelopes' coefficients.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of
# the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
"""The tables of KLASSIK's ACID (dsp/acid/acid.c), as Open303 makes them at start-up.

  gen_acid_tables.py [--fs RATE] acid_tables.h

Open303 (https://github.com/RobinSchmidt/Open303, Source/DSPCode/) renders its two waveforms at start-up with double
maths and keeps twelve band-limited copies of each (MipMappedWaveTable: an FFT, the top octave of harmonics cut for
each next one, back). Here the same, once, for the flash:
  ACID_SAW, ACID_SQR   the 303 saw and the 303 square (a saw through tanh, half a period later), tables 0..9 (the
                       highest a note under 20 kHz reaches; Open303 makes 12), int16 of ACID_TAB_GAIN; tables 0..4
                       2048 long as Open303's, each one after half as long (it holds half the harmonics: still 32
                       samples or more a period of the highest), each with one sample after its end (the first)
  ACID_ELL             the 12th-order elliptic anti-alias filter (EllipticQuarterBandFilter, direct form II in
                       Open303) as six second-order sections, for single precision
  the rest             the coefficients of the filters and envelopes Open303 never changes, at this rate
"""
import argparse
import cmath
import math
import sys

OS = 4                           # Open303::oversampling
N = 2048                         # MipMappedWaveTable::tableLength
NTAB = 10                        # tables kept (Open303: 12; 10 and 11 need 22 kHz and more)
FULL = 5                         # tables 0..4 full length, then each half the one before


# ------------------------------------------------------------- Open303's helpers (GlobalFunctions.h) ---
def db2amp(db):
    return math.exp(db * 0.11512925464970228420089957273422)


def amp2db(amp):
    return 8.6858896380650365530225783783321 * math.log(amp)


def round_to_int(x):             # rosic_NumberManipulations.h, the portable branch
    fl = math.floor(x)
    return int(fl) + 1 if x - fl >= 0.5 else int(fl)


# --------------------------------------------------------------------------------- the wave tables ---
def fft(x, inverse=False):
    """an in-order radix-2 complex FFT; the inverse without the 1/n"""
    n = len(x)
    a = list(x)
    j = 0
    for i in range(1, n):        # bit reversal
        bit = n >> 1
        while j & bit:
            j ^= bit
            bit >>= 1
        j |= bit
        if i < j:
            a[i], a[j] = a[j], a[i]
    size = 2
    sign = 1.0 if inverse else -1.0
    while size <= n:
        w = cmath.exp(complex(0.0, sign * 2.0 * math.pi / size))
        for start in range(0, n, size):
            wk = 1.0 + 0.0j
            for k in range(size // 2):
                u = a[start + k]
                v = a[start + k + size // 2] * wk
                a[start + k] = u + v
                a[start + k + size // 2] = u - v
                wk *= w
        size <<= 1
    return a


def saw303():                    # fillWithSaw303 (and the saw fillWithSquare303 starts from)
    n1 = min(max(round_to_int(0.5 * (N - 1)), 1), N - 1)
    n2 = N - n1
    s1, s2 = 1.0 / (n1 - 1), 1.0 / n2
    return [s1 * n for n in range(n1)] + [-1.0 + s2 * (n - n1) for n in range(n1, N)]


def square303():                 # fillWithSquare303, the constructor's shaper (36.9 dB, 4.37) and shift (180)
    k, off, shift = db2amp(36.9), 4.37, 180.0
    p = [-math.tanh(k * x + off) for x in saw303()]
    na = round_to_int(N * shift / 360.0)
    return p[N - na:] + p[:N - na]   # circularShift by +na


def mipmap(proto):
    """generateMipMap: table 0 the prototype; table t its harmonics 1 .. N / 2^(t+1) - 1 (no DC, no Nyquist)"""
    spec = fft(proto)
    tabs = [list(proto)]
    for t in range(1, NTAB):
        keep = N >> (t + 1)      # Open303 zeroes the bins from tableLength / 2^t of its re/im array
        s = [0j] * N
        for k in range(1, keep):
            s[k] = spec[k]
            s[N - k] = spec[N - k]
        tabs.append([v.real / N for v in fft(s, inverse=True)])
    return tabs


def tab_len(t):
    return N if t < FULL else N >> (t - FULL + 1)


def packed(tabs, gain):
    out, ofs = [], []
    for t, tab in enumerate(tabs):
        step = N // tab_len(t)
        short = tab[::step]
        ofs.append(len(out))
        out += [max(-32767, min(32767, round_to_int(v / gain))) for v in short + short[:1]]
    return out, ofs


# ----------------------------------------------------------- the anti-alias filter, in sections ---
ELL_A = [1.0, -9.1891604652189471, 40.177553696870497, -110.11636661771178, 210.18506612078195,
         -293.84744771903240, 308.16345558359234, -244.06786780384243, 144.81877911392738, -62.770692151724198,
         18.867762095902137, -3.5327094230551848, 0.31183189275203149]
ELL_B = [0.00013671732099945628, -0.00055538501265606384, 0.0013681887636296387, -0.0022158566490711852,
         0.0028320091007278322, -0.0029776933151090413, 0.0030283628243514991, -0.0029776933151090413,
         0.0028320091007278331, -0.0022158566490711861, 0.0013681887636296393, -0.00055538501265606384,
         0.00013671732099945636]


def roots(c):
    """the roots of c[0] z^n + c[1] z^(n-1) + ... (Durand-Kerner, then Newton on the full polynomial)"""
    n = len(c) - 1
    m = [v / c[0] for v in c]

    def p(z):
        r = 0j
        for v in m:
            r = r * z + v
        return r

    def dp(z):
        r = 0j
        for i, v in enumerate(m[:-1]):
            r = r * z + v * (n - i)
        return r

    z = [complex(0.4, 0.9) ** i for i in range(n)]
    for _ in range(2000):
        new = []
        for i in range(n):
            d = 1.0 + 0j
            for j in range(n):
                if i != j:
                    d *= z[i] - z[j]
            new.append(z[i] - p(z[i]) / d)
        if max(abs(a - b) for a, b in zip(new, z)) < 1e-15:
            z = new
            break
        z = new
    for _ in range(20):
        z = [r - p(r) / dp(r) if dp(r) != 0 else r for r in z]
    return z


def pairs(rs):
    """complex roots as conjugate pairs, the one with positive imaginary part of each"""
    up = [r for r in rs if r.imag > 1e-9]
    if len(up) * 2 != len(rs):
        raise SystemExit("gen_acid_tables: a real root in the elliptic filter (not expected)")
    return up


def ell_sections():
    poles = sorted(pairs(roots(ELL_A)), key=abs)        # the last section the poles nearest the circle
    zeros = pairs(roots(ELL_B))
    secs = []
    for p in poles:
        q = min(zeros, key=lambda z: abs(z - p))       # each pole pair with the zero pair nearest it
        zeros.remove(q)
        b = [1.0, -2.0 * q.real, abs(q) ** 2]
        a = [1.0, -2.0 * p.real, abs(p) ** 2]
        g = sum(a) / sum(b)                             # each section 1 at DC
        secs.append([b[0] * g, b[1] * g, b[2] * g, a[1], a[2]])
    gain = sum(ELL_B) / sum(ELL_A)                      # the whole filter's DC gain
    return secs, gain


def check_sections(secs, gain):
    worst = 0.0
    for i in range(1, 200):
        w = math.pi * i / 200
        z1 = cmath.exp(complex(0.0, -w))
        ref = sum(c * z1 ** k for k, c in enumerate(ELL_B)) / sum(c * z1 ** k for k, c in enumerate(ELL_A))
        h = gain
        for b0, b1, b2, a1, a2 in secs:
            h *= (b0 + b1 * z1 + b2 * z1 * z1) / (1.0 + a1 * z1 + a2 * z1 * z1)
        worst = max(worst, abs(h - ref) / max(abs(ref), 1e-6))
    if worst > 1e-6:
        raise SystemExit(f"gen_acid_tables: the sections are {worst:g} off the elliptic filter")


# ----------------------------------------------------------- Open303's fixed coefficients ---
def onepole_hp(fc, fs):          # OnePoleFilter::calcCoeffs, HIGHPASS: y = b0 x + b1 x1 + a1 y1
    x = math.exp(-2.0 * math.pi * fc / fs)
    return 0.5 * (1 + x), -0.5 * (1 + x), x


def onepole_ap(fc, fs):          # ALLPASS
    t = math.tan(math.pi * fc / fs)
    x = (t - 1.0) / (t + 1.0)
    return x, 1.0, -x


def biquad_notch(f, bw, fs):     # BiquadFilter::calcCoeffs, BANDREJECT: y = b0 x + b1 x1 + b2 x2 + a1 y1 + a2 y2
    w = 2 * math.pi * f / fs
    s, c = math.sin(w), math.cos(w)
    alpha = s * math.sinh(0.5 * math.log(2.0) * bw * w / s)
    scale = 1.0 / (1.0 + alpha)
    return scale, -2.0 * c * scale, scale, 2.0 * c * scale, (alpha - 1.0) * scale


def biquad_lp12(f, gain_db, fs):  # LOWPASS12
    w = 2 * math.pi * f / fs
    s, c = math.sin(w), math.cos(w)
    q = db2amp(gain_db)
    alpha = s / (2.0 * q)
    scale = 1.0 / (1.0 + alpha)
    b1 = (1.0 - c) * scale
    return 0.5 * b1, b1, 0.5 * b1, 2.0 * c * scale, (alpha - 1.0) * scale


def rc_coeff(tau_ms, fs):        # LeakyIntegrator::calculateCoefficient
    return math.exp(-1.0 / (fs * 0.001 * tau_ms)) if tau_ms > 0.0 else 0.0


def env_coeff(t_ms, fs):         # AnalogEnvelope::setDecay / setRelease (tauScale, timeScale 1)
    return 1.0 - math.exp(-1.0 / (fs * 0.001 * t_ms)) if t_ms > 0.0 else 1.0


def f32(v):
    s = f"{v:.9g}"
    return s + ("f" if ("." in s or "e" in s) else ".0f")


def arr16(name, vals, per=16):
    rows = [", ".join(str(v) for v in vals[i:i + per]) for i in range(0, len(vals), per)]
    return f"static const int16_t {name}[{len(vals)}] = {{\n    " + ",\n    ".join(rows) + "\n};\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--fs", type=int, default=44100, help="the sample rate (the engines' FS)")
    ap.add_argument("--dump", help="also write the tables as float64, 2 x NTAB x 2048 (a check against Open303)")
    ap.add_argument("out")
    a = ap.parse_args()
    fs, fso = float(a.fs), float(a.fs * OS)
    saw, sqr = mipmap(saw303()), mipmap(square303())
    if a.dump:
        import struct
        with open(a.dump, "wb") as f:
            for tabs in (saw, sqr):
                for tab in tabs:
                    f.write(struct.pack(f"<{N}d", *tab))
    peak = max(abs(v) for tabs in (saw, sqr) for tab in tabs for v in tab)
    gain = peak / 32767.0
    saw16, ofs = packed(saw, gain)
    sqr16, _ = packed(sqr, gain)
    secs, ell_gain = ell_sections()
    check_sections(secs, ell_gain)
    hp1 = onepole_hp(44.486, fso)                # Open303(): highpass1, before the filter (oversampled)
    fbhp = onepole_hp(150.0, fso)                # filter.setFeedbackHighpassCutoff(150)
    hp2 = onepole_hp(24.167, fs)                 # highpass2, after
    allp = onepole_ap(14.008, fs)
    notch = biquad_notch(7.5164, 4.7, fs)
    declick = biquad_lp12(200.0, amp2db(math.sqrt(0.5)), fs)
    h = ["/* SPDX-License-Identifier: MIT", " * Copyright (c) 2009 Robin Schmidt (www.rs-met.com)",
         f" * Generated by firmware/klassik/tools/gen_acid_tables.py --fs {a.fs}: Open303's tables (see there). */",
         "#pragma once", "#include <stdint.h>", "",
         f"#define ACID_GEN_FS {a.fs}", f"#define ACID_OS {OS}", f"#define ACID_TAB_N {N}",
         f"#define ACID_NTAB {NTAB}", f"#define ACID_TAB_GAIN {f32(gain)}", "",
         "static const uint16_t ACID_TAB_OFS[ACID_NTAB] = {" + ", ".join(str(o) for o in ofs) + "};",
         "static const uint8_t ACID_TAB_SHIFT[ACID_NTAB] = {" +
         ", ".join(str(max(0, t - FULL + 1)) for t in range(NTAB)) + "};   /* 2048 >> shift long */", "",
         "/* the anti-alias filter: b0 b1 b2 a1 a2 (y = b0 x + s1, s1 = b1 x - a1 y + s2, s2 = b2 x - a2 y), then the gain */",
         "static const float ACID_ELL[6][5] = {"]
    h += ["    {" + ", ".join(f32(v) for v in s) + "}," for s in secs]
    h += ["};", f"#define ACID_ELL_GAIN {f32(ell_gain)}", "",
          "/* the fixed filters (b0 b1 a1: y = b0 x + b1 x1 + a1 y1; b0 b1 b2 a1 a2: y = ... + a1 y1 + a2 y2) */",
          "static const float ACID_HP1[3] = {" + ", ".join(f32(v) for v in hp1) + "};      /* 44.486 Hz, oversampled */",
          "static const float ACID_FBHP[3] = {" + ", ".join(f32(v) for v in fbhp) + "};     /* 150 Hz, oversampled */",
          "static const float ACID_HP2[3] = {" + ", ".join(f32(v) for v in hp2) + "};      /* 24.167 Hz */",
          "static const float ACID_ALLPASS[3] = {" + ", ".join(f32(v) for v in allp) + "};  /* 14.008 Hz */",
          "static const float ACID_NOTCH[5] = {" + ", ".join(f32(v) for v in notch) + "};   /* 7.5164 Hz, 4.7 octaves */",
          "static const float ACID_DECLICK[5] = {" + ", ".join(f32(v) for v in declick) + "}; /* 200 Hz low-pass */",
          "",
          "/* the envelopes' fixed coefficients */",
          f"#define ACID_RC2 {f32(rc_coeff(15.0, fs))}                 /* rc2: 15 ms */",
          f"#define ACID_AMP_DECAY {f32(env_coeff(1230.0, fs))}       /* ampEnv: decay 1230 ms */",
          f"#define ACID_AMP_RELEASE {f32(env_coeff(1.0, fs))}         /* release 1 ms */",
          f"#define ACID_AMP_RELEASE_ACC {f32(env_coeff(50.0, fs))}    /* release 50 ms (an accent) */",
          "",
          arr16("ACID_SAW", saw16), arr16("ACID_SQR", sqr16)]
    with open(a.out, "w") as f:
        f.write("\n".join(h))
    return 0


if __name__ == "__main__":
    sys.exit(main())
