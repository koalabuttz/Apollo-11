/* SPDX-License-Identifier: MIT */
/* MEASUREMENT_INCORPORATION.agc (Luminary099 pages 1149-1158,
 * Comanche055 pages 1252-1261). The faithful C is c64-port/src/
 * lm_measurement_inc.c and cm_measurement_inc.c.
 *
 * The navigation filter's last step: a measurement (a radar range or
 * angle, a star or landmark sighting) taken into the state vector.
 *
 *   INCORP1   "PERFORMS THE SIX DIMENSIONAL STATE VECTOR DEVIATION FOR
 *             POSITION AND VELOCITY OR THE NINE-DIMENSIONAL DEVIATION" (with
 *             the radar or landmark bias): from the error transition matrix
 *             W and the measurement's BVECTOR, the vector ZI, the scalar
 *             GAMMA, the weighting vector OMEGA and the deviation DELTAX.
 *   INCORP2   "INCORPORATES THE COMPUTED STATE VECTOR DEVIATIONS INTO THE
 *             ESTIMATED STATE VECTOR", the LM's or the CSM's (VEHUPFLG),
 *             and updates W, a restart-protected job in three phases.
 *   NEWZCOMP  scales ZI and NORMGAM so the largest of its three vectors
 *             keeps its precision.
 *
 * The two missions' files are the same code; the CM's NEWZCOMP is split
 * over two banks (NEWZCMP1). Most of it is interpretive code: the tables
 * below, in the listing's notation, which the readable interpreter runs.
 * INCORP2's basic code, the phases' bookkeeping between them, is in
 * incorp2.c. */
#ifndef READABLE_MEASUREMENT_INC_H
#define READABLE_MEASUREMENT_INC_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

/* The interpretive code, part by part: INCORP1 (to INCOR3's end), INCORP2
 * to its EXIT, INCORP2's two interpretive stretches after its TC INTPRETs
 * (FAZA1's and FAZB1's), DOCSM and DOCSM1 with the file's constants, and
 * NEWZCOMP (the CM's in two parts). */
#if LUMINARY099
#define MEASUREMENT_INC_PARTS 6u
#else
#define MEASUREMENT_INC_PARTS 7u
#endif
extern const struct interpretive_code MEASUREMENT_INC_CODE[MEASUREMENT_INC_PARTS];

/* INCORP2's basic code -- LM pp. 1153-1157, CM pp. 1256-1260. */
void incorp2_after_exit(void);  /* the W and Z indices, then FAZA */
void faza(void);                /* FAZA: the phase change */
void faza_after_phaschng(void); /* FAZA +2: UPFLAG REINTFLG */
void faza1(void);               /* FAZA1: the first phase's indices, INTPRET */
void fazb1(void);               /* FAZB1: the second phase's, INTPRET */
void fazab5_after_exit(void);   /* FAZAB5 +4: out by POSTJUMP to INTWAKE */

#endif
