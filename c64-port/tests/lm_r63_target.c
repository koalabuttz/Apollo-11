/* SPDX-License-Identifier: MIT */
/* The program the lm-r63 cases run: Luminary's R63 routines
 * alone, entered at NEWOPS or Z. */
#include "agc_transfer.h"
#include "lm_r63.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_r63_basic,1,&lm_r63_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
