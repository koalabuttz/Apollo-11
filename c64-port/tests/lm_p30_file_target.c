/* SPDX-License-Identifier: MIT */
/* The program the lm-p30-file cases run: Luminary099
 * P30 (external delta-V targeting)' routines alone, basic ones at Z and
 * interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "lm_p30_file.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_p30_file_basic,1,&lm_p30_file_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
