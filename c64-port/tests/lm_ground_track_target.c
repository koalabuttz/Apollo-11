/* SPDX-License-Identifier: MIT */
/* The program the lm-ground-track cases run: Luminary099
 * GROUND_TRACKING_DETERMINATION_PROGRAM' routines alone, basic ones at Z and
 * interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "lm_ground_track.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_ground_track_basic,1,&lm_ground_track_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
