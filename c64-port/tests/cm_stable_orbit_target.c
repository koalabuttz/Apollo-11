/* SPDX-License-Identifier: MIT */
/* The program the cm-stable-orbit cases run: Comanche055 CM_STABLE_ORBIT's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. The
 * mission's DANZIG marker comes first among the interpretive tables, so a
 * pending right-hand order (P39/P79B's RTB LOADTIME, DAD on its right) resolves at LOC
 * (include/agc_transfer.h). */
#include "agc_transfer.h"
#include "cm_stable_orbit.h"
const agc_native_program *irc_program(uint8_t cm);
/* (filled at the first call: the tables are not constant expressions) */
static agc_routine_table tables[2];
static agc_native_program cm={&cm_stable_orbit_basic,1,tables,2,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;
 if(!tables[1].entries){tables[0]=agc_cm_danzig_marker;tables[1]=cm_stable_orbit_interpretive;}
 return &cm;}
