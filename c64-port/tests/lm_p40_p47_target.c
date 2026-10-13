/* SPDX-License-Identifier: MIT */
/* The program the lm-p40-p47 cases run: Luminary's P40-P47 routines
 * alone, entered at NEWOPS or Z. */
#include "agc_transfer.h"
#include "lm_p40_p47.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_p40_p47_basic,1,&lm_p40_p47_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
