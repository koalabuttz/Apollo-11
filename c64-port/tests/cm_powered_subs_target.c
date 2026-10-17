/* SPDX-License-Identifier: MIT */
/* The program the cm-powered-subs cases run: Comanche055
 * POWERED_FLIGHT_SUBROUTINES' routines alone, basic ones at Z and
 * interpretive ones at NEWOPS. The mission's DANZIG marker comes first
 * among the interpretive tables, so CD*TR*GS's CADR words after TC
 * USPRCADR (EDOP BIT8, the interpreter's EXIT) resolve at LOC
 * (include/agc_transfer.h). */
#include "agc_transfer.h"
#include "cm_powered_subs.h"
const agc_native_program *irc_program(uint8_t cm);
/* (filled on every call, so the target has no branch: the tables are not
 * constant expressions) */
static agc_routine_table tables[2];
static agc_native_program cm={&cm_powered_subs_basic,1,tables,2,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;
 tables[0]=agc_cm_danzig_marker;tables[1]=cm_powered_subs_interpretive;
 return &cm;}
