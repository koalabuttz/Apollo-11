/* SPDX-License-Identifier: MIT */
/* The program the cm-t4rupt-optics cases run: the Comanche055 T4RUPT
 * optics monitor and drive routines alone, at Z. */
#include "agc_transfer.h"
#include "cm_t4rupt_optics.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_t4rupt_optics_basic,1,&cm_t4rupt_optics_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
