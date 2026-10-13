/* SPDX-License-Identifier: MIT */
/* The program the findcduw cases run: Luminary099 FINDCDUW--GUIDAP_INTERFACE
 * routines alone, entered at NEWOPS. */
#include "agc_transfer.h"
#include "lm_findcduw.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_findcduw_basic,1,&lm_findcduw_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
