/* SPDX-License-Identifier: MIT */
/* The program the lm-llge cases run: Luminary's LUNAR_LANDING_GUIDANCE_EQUATIONS routines
 * alone, entered at NEWOPS or Z, with the channel latches WOR CHAN13 writes. */
#include "agc_transfer.h"
#include "lm_llge.h"
const agc_native_program *irc_program(uint8_t cm);
static agc_hardware hw;
static const agc_native_program lm={&lm_llge_basic,1,&lm_llge_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,&hw};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
