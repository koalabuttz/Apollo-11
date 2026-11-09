/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc (Luminary099 pages 1227-1248, Comanche055
 * pages 1334-1355). The faithful C is c64-port/src/lm_orbital_integration.c
 * and cm_orbital_integration.c.
 *
 * The integration itself, which INTEGRATION_INITIALIZATION
 * (src/integration_init/) sets up and ends. A state vector is carried as a
 * conic (RCV and VCV, TC the time along it) plus a small deviation from it
 * (TDELTAV, TNUV), Encke's method: each step moves the conic on to a time
 * TAU. by Kepler's equation (KEPPREP, then KEPLERN in CONIC_SUBROUTINES),
 * and integrates the deviation, Nystrom's method (DIFEQ+0, DIFEQ+1,
 * DIFEQ+2), from the accelerations that are not the conic's: the other
 * bodies' (ACCOMP, GAMCOMP) and the earth's or the moon's oblateness
 * (OBLATE). When the deviation grows too big, it is put into the conic
 * (RECTIFY). With DIM0FLAG set, the W-matrix is integrated with it.
 *
 * Written in parts (README.md):
 *   part 1: KEPPREP and FBR3 (kepprep.c: LM pp. 1227-1229, CM pp.
 *     1334-1336) and ACCOMP to INT-ABRT (accomp.c: LM pp. 1230-1233, CM
 *     pp. 1337-1340).
 * The binding (src/binding/orbital_integration_routines.c) gives the keys
 * of the faithful port's tables written so far their readable routines. */
#ifndef READABLE_ORBITAL_INTEGRATION_H
#define READABLE_ORBITAL_INTEGRATION_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* A fixed label's bank, and its address in the bank, from its I_ value
 * (bank * 02000 + its place in the bank). */
#ifndef CODE_BANK
#define CODE_BANK(i) ((unsigned)((i) >> 10))
#define CODE_Z(i)    ((word)((i) % 02000u + 02000u))
#endif

/* Part 1: KEPPREP and FBR3; ACCOMP, GETRPSV, GAMCOMP and GOBAQUE, to
 * INT-ABRT's EXIT. */
extern const struct interpretive_code KEPPREP_CODE, ACCOMP_CODE;

/* Part 1's basic code: after INT-ABRT's EXIT. */
void int_abrt_after_exit(void); /* INT-ABRT +1: TC POODOO */

#endif
