/* SPDX-License-Identifier: MIT */
#ifndef EPHEMERIS_SHIFT_H
#define EPHEMERIS_SHIFT_H
#include "overlay_runtime.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Native scalar SL5/SL8/SL16 dependency at original GENSHIFT entry. Ordinary
 * canonical RAM, DP MODE+0, coherent bank0, A=LOW7 and exact positive no-round
 * shift descriptor required. All source debris is retained through DANZIG.
 * Returns zero without changing the image for unsupported input. */
uint8_t ephemeris_scalar_left(apollo_overlay_state *,enum apollo_fixed_mission);
#endif
