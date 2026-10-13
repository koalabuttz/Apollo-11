/* SPDX-License-Identifier: MIT */
/* The program the lm-p32-p35 cases run: Luminary099 P32-P35_P72-P75 routines alone,
 * entered at NEWOPS (LM only). */
#include "agc_transfer.h"
#include "lm_p32_p35.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_p32_p35_basic,1,&lm_p32_p35_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
