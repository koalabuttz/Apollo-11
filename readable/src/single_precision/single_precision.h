/* SPDX-License-Identifier: MIT */
/* Single precision sine and cosine (SINGLE_PRECISION_SUBROUTINES.agc), for
 * basic code that needs one quickly: the digital autopilots, the entry
 * DAP, the radar and the displays. The interpreter has its own, in double
 * precision (SIN and COS in INTERPRETER.agc).
 *
 * The angle comes in A, scaled at pi: a half turn is 1/2, so A runs over
 * one turn from -1 to +1. The sine or cosine comes back in A, scaled at 1,
 * by TC Q. Both routines are called with TC SPSIN or TC SPCOS. */
#ifndef READABLE_SINGLE_PRECISION_H
#define READABLE_SINGLE_PRECISION_H

#include "../agc/agc.h"

void spcos(void);
void spsin(void);

#endif
