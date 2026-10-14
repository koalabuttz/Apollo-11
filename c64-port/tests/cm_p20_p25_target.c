/* SPDX-License-Identifier: MIT */
/* The program the cm-p20-p25 cases run: Comanche055 P20-P25 routines alone,
 * entered at NEWOPS (CM only). */
#include "agc_transfer.h"
#include "cm_p20_p25.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_p20_p25_basic,1,&cm_p20_p25_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
