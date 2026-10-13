/* SPDX-License-Identifier: MIT */
/* The program the cm-stable-orbit cases run: Comanche055 CM_STABLE_ORBIT's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "cm_stable_orbit.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_stable_orbit_basic,1,&cm_stable_orbit_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
