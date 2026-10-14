/* SPDX-License-Identifier: MIT */
/* The program the lm-stable-orbit cases run: Luminary099 STABLE_ORBIT routines alone,
 * entered at NEWOPS (LM only). */
#include "agc_transfer.h"
#include "lm_stable_orbit.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_stable_orbit_basic,1,&lm_stable_orbit_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &lm;}
