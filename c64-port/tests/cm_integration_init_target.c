/* SPDX-License-Identifier: MIT */
/* The program the cm-integration-init cases run: Comanche055 CM_INTEGRATION_INIT's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. The
 * mission's DANZIG marker comes first among the interpretive tables, so a
 * pending right-hand order after an RTB resolves at LOC
 * (include/agc_transfer.h). */
#include "agc_transfer.h"
#include "cm_integration_init.h"
const agc_native_program *irc_program(uint8_t cm);
/* (filled at the first call: the tables are not constant expressions) */
static agc_routine_table tables[2];
static agc_native_program cm={&cm_integration_init_basic,1,tables,2,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;
 if(!tables[1].entries){tables[0]=agc_cm_danzig_marker;tables[1]=cm_integration_init_interpretive;}
 return &cm;}
