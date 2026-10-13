/* SPDX-License-Identifier: MIT */
/* The program the lm-rcsmon cases run: Luminary099 RCS_FAILURE_MONITOR's routines
 * alone (basic code). */
#include "agc_transfer.h"
#include "lm_rcsmon.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_rcsmon_basic,1,&lm_rcsmon_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
