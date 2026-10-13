/* SPDX-License-Identifier: MIT */
/* The program the cm-servicer cases run: Comanche055 SERVICER207's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "cm_servicer.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_servicer_basic,1,&cm_servicer_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
