/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055): the
 * basic routines an interpretive RTB runs ("return to basic": RTB names a
 * routine of these, the interpreter's SWCALL goes there, and the routine
 * ends back at DANZIG). Only CDULOGIC (for AOTMARK's sightings and the
 * geometry) and 2V1STO2S (for the LM's radar angles) are here so far; the
 * rest of the file is the faithful port's. */
#ifndef READABLE_RTB_OP_CODES_H
#define READABLE_RTB_OP_CODES_H

#include "../agc/agc.h"

void cdulogic(void);
void two_v1sto2s(void);

#endif
