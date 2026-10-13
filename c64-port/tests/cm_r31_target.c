/* SPDX-License-Identifier: MIT */
/* The program the cm-r31 cases run: Comanche055 R31 routines alone,
 * entered at NEWOPS (CM only). */
#include "agc_transfer.h"
#include "cm_r31.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_r31_basic,1,&cm_r31_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
