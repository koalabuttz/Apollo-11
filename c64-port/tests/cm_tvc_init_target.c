/* SPDX-License-Identifier: MIT */
/* The program the cm-tvc-init cases run: Comanche055 TVCINITIALIZE's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "cm_tvc_init.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_tvc_init_basic,1,&cm_tvc_init_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
