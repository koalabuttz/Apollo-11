/* SPDX-License-Identifier: MIT */
/* The program the lm-p20-p25 cases run: Luminary099 P20-P25 routines alone,
 * entered at NEWOPS (LM only). The mission's DANZIG marker comes first among
 * the interpretive tables, so a pending right-hand order (R61LEM1's RTB CALL
 * READCDUD, LSR22.3's RTB PUSH NORMUNX1) resolves at LOC
 * (include/agc_transfer.h). */
#include "agc_transfer.h"
#include "lm_p20_p25.h"
const agc_native_program *irc_program(uint8_t cm);
static agc_hardware hw;
/* (filled at the first call: the tables are not constant expressions) */
static agc_routine_table tables[2];
static agc_native_program lm={&lm_p20_p25_basic,1,tables,2,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,&hw};
const agc_native_program *irc_program(uint8_t m){(void)m;
 if(!tables[1].entries){tables[0]=agc_lm_danzig_marker;tables[1]=lm_p20_p25_interpretive;}
 return &lm;}
