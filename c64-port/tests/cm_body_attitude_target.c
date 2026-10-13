/* SPDX-License-Identifier: MIT */
/* The program the cm-body-attitude cases run: Comanche055 CM_BODY_ATTITUDE's
 * routines alone, basic ones at Z and interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "cm_body_attitude.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_body_attitude_basic,1,&cm_body_attitude_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
