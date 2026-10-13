/* SPDX-License-Identifier: MIT */
/* The program the cm-sxtmark cases run: Comanche055 SXTMARK's routines
 * alone, basic ones at Z. */
#include "agc_transfer.h"
#include "cm_sxtmark.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_sxtmark_basic,1,&cm_sxtmark_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
