/* SPDX-License-Identifier: MIT */
#ifndef EPHEMERIS_TIME_SCALE_H
#define EPHEMERIS_TIME_SCALE_H
#include "ephemeris_shift.h"
/* Original scalar SR14 at GENSHIFT bank0; A=LOW7, ADDRWD020616,
 * scalar MODE+0, coherent EB/BB and ordinary canonical words. */
uint8_t ephemeris_time_downscale(apollo_overlay_state *,enum apollo_fixed_mission);
#endif
