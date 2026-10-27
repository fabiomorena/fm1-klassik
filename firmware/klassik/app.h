/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 fabiomorena
 * From fm1-base's template, Copyright (C) 2026 Charles Vestal (MPL-2.0) */
/* KLASSIK: the app's half of the contract with the platform (base/device/unity.c, base/host/plat_host.c). */
#pragma once
#include <stdint.h>
#include "plat.h"

#define APP_NAME "KLASSIK"
#define APP_SPLASH_RGB RGB(255, 180, 84)
#define APP_BLOCK 256u                   /* frames per audio block: 5.8 ms at 44.1 kHz, 8 engine blocks of 32 */
#define APP_TOUCH                        /* velocity and pressure from the keys (lib/touch.h) */

/* the store: the settings, and the panel calibration (the platform's), at the template's offsets */
enum { OBJ_SETTINGS, OBJ_PANEL, OBJ_NOBJ };
#define APP_STORE_MAP {0xFC000u, 0xDC000u}

/* the contract: the platform calls these */
void app_init(void);
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *a, const char *b);

/* the host simulator's checks: engine, preset, octave, voices, dirty; edit (a page open), page (1, 2), edited,
 * user (the engine has a USER sound); of the sound playing, v0..v7 (its values), dsp0..dsp7 (as the render plays
 * them), glyph0..glyph7 (its spokes, 0..255); the mixer's and the looper's (app_expect in app.c) */
#define APP_HOST_EXPECT
int app_expect(const char *what, int *got);
/* the simulator's reboot: the app forgets what the RAM would (the page open, a message) */
#define APP_HOST_REBOOT
void app_host_reboot(void);
