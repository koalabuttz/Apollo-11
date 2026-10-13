/* SPDX-License-Identifier: MIT */
/* The program the cm-p32-p33 cases run: Comanche055 P32-P33_P72-P73 routines alone,
 * entered at NEWOPS (CM only). */
#include "agc_transfer.h"
#include "cm_p32_p33.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_p32_p33_basic,1,&cm_p32_p33_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
