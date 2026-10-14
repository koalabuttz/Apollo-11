/* SPDX-License-Identifier: MIT */
/* The program the lm-kalman cases run: Luminary099 KALMAN_FILTER's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS, with the
 * channels they read in the program's agc_hardware. */
#include "agc_transfer.h"
#include "lm_kalman.h"
const agc_native_program *irc_program(uint8_t cm);
static agc_hardware hw;
static const agc_native_program lm={&lm_kalman_basic,1,&lm_kalman_interpretive,0,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,&hw};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
