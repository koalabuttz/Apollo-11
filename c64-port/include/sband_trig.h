/* SPDX-License-Identifier: MIT */
#ifndef SBAND_TRIG_H
#define SBAND_TRIG_H
#include "fixed_data.h"
#include "agc_word.h"
#include "overlay_runtime.h"
/* Original canonical SINE/COSINE body, DP MODE0, explicit mission entry PC and
 * source bank0; ends before DANZIG. Ordinary RAM/Q/rawA/L, coherent banks/ZERO.
 * Caller excludes producers and owns the image; no scheduler poll or decoder.
 * SUPER/IRQ remain external and untouched. Invalid context is unchanged. */
uint8_t sband_trig(apollo_overlay_state*,enum apollo_fixed_mission,uint8_t cosine);
#endif
