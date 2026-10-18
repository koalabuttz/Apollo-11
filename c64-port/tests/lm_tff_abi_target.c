/* SPDX-License-Identifier: MIT */
/* The program the lm-tff-abi cases run: Luminary099 TIME_OF_FREE_FALL's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. The
 * mission's DANZIG marker comes first among the interpretive tables, so a
 * pending right-hand order (TFFXTEST's RTB DMP T(X)) resolves at LOC
 * (include/agc_transfer.h). */
#include "agc_transfer.h"
#include "lm_tff_abi.h"
const agc_native_program *irc_program(uint8_t cm);
/* (filled at the first call: the tables are not constant expressions) */
static agc_routine_table tables[2];
static agc_native_program lm={&lm_tff_abi_basic,1,tables,2,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;
 if(!tables[1].entries){tables[0]=agc_lm_danzig_marker;tables[1]=lm_tff_abi_interpretive;}
 return &lm;}
