/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena */
/* KLASSIK's own presets for Felucca-Engines' ANALOG, PHASE and WHEEL, after the engines' own (engines.c preset_at):
 * their numbers stay. Felucca's preset_t: the eight parameters, ATK DEC SUS REL, ENV > FILTER, MONO; its effect sends
 * and pattern are Felucca's, KLASSIK reads none. Included by engines.c after the engines. */

static const preset_t VA_MORE[] = {         /* WAVE DTN MIX NOIS CUT RES DRV KTR */
    {.name = "SYNC LEAD", .e = {5, 40, 100, 0, 95, 25, 20, 64}, .env = {2, 70, 100, 40}, .fenv = 15, .mono = 1},
    {.name = "FAT BASS", .e = {6, 6, 90, 0, 45, 40, 30, 32}, .env = {0, 60, 70, 25}, .fenv = 35, .mono = 1},
    {.name = "POLY KEYS", .e = {1, 10, 64, 0, 60, 20, 0, 90}, .env = {0, 72, 45, 50}, .fenv = 28},
};

static const preset_t PD_MORE[] = {         /* WAVE WAVE2 DCW ENV DTN LINE SUB FOLD */
    {.name = "BASS", .e = {0, 0, 20, 70, 0, 0, 60, 0}, .env = {0, 55, 40, 20}, .mono = 1},
    {.name = "E.PIANO", .e = {3, 0, 25, 60, 0, 0, 0, 0}, .env = {0, 85, 20, 60}},
    {.name = "SWEEP PAD", .e = {7, 0, 25, 55, 8, 0, 15, 0}, .env = {75, 100, 110, 90}},
    {.name = "RESO LEAD", .e = {5, 0, 75, 30, 7, 0, 0, 25}, .env = {2, 70, 100, 40}, .mono = 1},
    {.name = "FOLD BASS", .e = {0, 0, 5, 40, 0, 0, 40, 70}, .env = {0, 60, 50, 25}, .mono = 1},
    {.name = "CHIME", .e = {3, 6, 70, 50, 20, 1, 0, 30}, .env = {0, 100, 0, 100}},
};

static const preset_t ORGAN_MORE[] = {      /* REG SUB BODY TOP PERC CLICK DRV ROTR */
    {.name = "BLUES", .e = {5, 0, 0, -2, 4, 40, 35, 2}, .env = {0, 64, 127, 40}},
    {.name = "BALLAD", .e = {3, 3, 0, -4, 0, 15, 0, 1}, .env = {8, 64, 127, 60}},
    {.name = "MELLOW", .e = {2, 0, 0, 0, 5, 20, 0, 1}, .env = {0, 64, 127, 45}},
    {.name = "CHAPEL", .e = {12, 4, 0, -3, 0, 0, 0, 0}, .env = {25, 64, 127, 80}},
    {.name = "REED", .e = {10, 0, 0, 2, 0, 35, 45, 1}, .env = {0, 64, 127, 40}},
    {.name = "CLARINET", .e = {9, -2, 0, 0, 0, 20, 0, 1}, .env = {10, 64, 127, 40}},
    {.name = "BRIGHT", .e = {13, 0, 0, 2, 2, 60, 10, 2}, .env = {0, 64, 127, 40}},
};

_Static_assert(NELEM(ANALOG_PRESETS) + NELEM(VA_MORE) <= KL_MAXPRESET, "VA: at most KL_MAXPRESET presets");
_Static_assert(NELEM(PHASE_PRESETS) + NELEM(PD_MORE) <= KL_MAXPRESET, "PD: at most KL_MAXPRESET presets");
_Static_assert(NELEM(WHEEL_PRESETS) + NELEM(ORGAN_MORE) <= KL_MAXPRESET, "ORGAN: at most KL_MAXPRESET presets");
