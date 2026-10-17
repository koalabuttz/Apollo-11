/* SPDX-License-Identifier: MIT */
/* The program the lm-llge cases run: Luminary's LUNAR_LANDING_GUIDANCE_EQUATIONS routines
 * alone, entered at NEWOPS or Z, with the channel latches WOR CHAN13 writes.
 * The mission's DANZIG marker comes first among the interpretive tables, so
 * the EXIT USPRCADR leaves pending in EDOP resolves at LOC, the CADR word
 * (include/agc_transfer.h). */
#include "agc_transfer.h"
#include "lm_llge.h"
const agc_native_program *irc_program(uint8_t cm);
static agc_hardware hw;
/* (filled at each call: the tables are not constant expressions) */
static agc_routine_table tables[2];
static agc_native_program lm={&lm_llge_basic,1,tables,2,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,&hw};
const agc_native_program *irc_program(uint8_t m){(void)m;
 tables[0]=agc_lm_danzig_marker;tables[1]=lm_llge_interpretive;
 return &lm;}
