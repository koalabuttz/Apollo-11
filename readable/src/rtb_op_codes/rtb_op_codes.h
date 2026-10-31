/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055): the
 * basic routines an interpretive RTB runs ("return to basic": RTB names a
 * routine of these, the interpreter's SWCALL goes there, and the routine
 * ends back at DANZIG). Only CDULOGIC is here so far, for AOTMARK's
 * sightings; the rest of the file is the faithful port's. */
#ifndef READABLE_RTB_OP_CODES_H
#define READABLE_RTB_OP_CODES_H

#include "../agc/agc.h"

void cdulogic(void);

#endif
