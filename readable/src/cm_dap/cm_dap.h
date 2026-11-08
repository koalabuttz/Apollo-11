/* SPDX-License-Identifier: MIT */
/* The CM's RCS digital autopilot, readable C of Comanche055 (CM).
 *
 * MYSUBS.agc (pp. 999-1001), mysubs.c: two arithmetic subroutines of the
 * DAP ("COUNT 21/DAPMS"), on the double word KMPAC, KMPAC +1:
 *     SMALLMP  a single word times KMPAC: KMPAC A, double length.
 *     DPADD    A,L added into KMPAC, the sum taken round the circle when
 *              it overflows (KMPAC is an angle).
 * Each is called by TC and returns by TC Q. The faithful C is
 * c64-port/src/cm_mysubs.c.
 *
 * The file also gives SPCOS and SPSIN (SINGLE_PRECISION_SUBROUTINES.agc,
 * src/single_precision) the DAP's names for them: SPCOS1 and SPCOS2 EQUALS
 * SPCOS, SPSIN1 and SPSIN2 EQUALS SPSIN. */
#ifndef READABLE_CM_DAP_H
#define READABLE_CM_DAP_H

#include "../agc/agc.h"

void smallmp(void);
void dpadd(void);

#endif
