/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc (Luminary099 pp. 1259-1267): basic and
 * interpretive helpers of the powered flight programs. Only QUICTRIG (for
 * FINDCDUW's RTB, src/findcduw/) is here so far; the rest of the file is
 * the faithful port's (c64-port/src/lm_powered_quick.c and others). */
#ifndef READABLE_POWERED_FLIGHT_H
#define READABLE_POWERED_FLIGHT_H

#include "../agc/agc.h"

#if LUMINARY099
void quictrig(void); /* QUICTRIG */
#endif

#endif
