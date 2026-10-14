/* SPDX-License-Identifier: MIT */
/* The program the lm-p70-p71 cases run: Luminary's P70-P71 routines
 * alone, entered at NEWOPS or Z. */
#include "agc_transfer.h"
#include "lm_p70_p71.h"
const agc_native_program *irc_program(uint8_t cm);
/* in_interrupt: CONTABRT's RESUME ends an interrupt ('in_isr'). */
static agc_hardware hw;
static const agc_native_program lm={&lm_p70_p71_basic,1,&lm_p70_p71_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,&hw};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
