/* SPDX-License-Identifier: MIT */
/* The program the cm-p34-p35 cases run: Comanche055 P34-35_P74-75 routines alone,
 * entered at NEWOPS (CM only). */
#include "agc_transfer.h"
#include "cm_p34_p35.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_p34_p35_basic,1,&cm_p34_p35_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
