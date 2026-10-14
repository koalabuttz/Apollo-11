/* SPDX-License-Identifier: MIT */
/* The program the cm-p76-file cases run: Comanche055
 * P76 (target delta-V)' routines alone, basic ones at Z and
 * interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "cm_p76_file.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_p76_file_basic,1,&cm_p76_file_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
