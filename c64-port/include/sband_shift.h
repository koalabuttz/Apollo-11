/* SPDX-License-Identifier: MIT */
#ifndef SBAND_SHIFT_H
#define SBAND_SHIFT_H
#include "fixed_data.h"
#include "overlay_runtime.h"
#include "agc_word.h"
/* Explicit native short vector left shift, counts1..8. Original SHORTV entry
 * PC02121, bank0, MODE-1, CYR020000+count-1, ordinary image/Q/raw16 A/L and
 * coherent banks/ZERO required. Stops before mission DANZIG; no dispatch.
 * Caller owns the canonical image and excludes producers. External latches
 * are untouched. Unsupported context preserves the complete image. */
uint8_t sband_shift_left(apollo_overlay_state *,enum apollo_fixed_mission,uint8_t count);
#endif
