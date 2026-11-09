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
 *     pp. 1337-1340);
 *   part 2: OBLATE to COSPHIE (oblate.c: LM pp. 1234-1237, CM pp.
 *     1341-1344), and the LM's QUALITY1 and QUALITY2 (quality.c: LM pp.
 *     1247-1248);
 *   part 3: DIFEQTAB to DIFEQ0, with the sphere of influence checks
 *     (timestep.c: LM pp. 1237-1238, CM pp. 1344-1345), and DOSWITCH to
 *     MINIRECT (rectify.c: LM pp. 1239-1240, CM pp. 1345-1347);
 *   part 4: DIFEQ+0 to WMATEND (difeq.c: LM pp. 1241-1244, CM pp.
 *     1348-1351), DOW.. and DOW..1 and the LM's SETITCTR (dow.c: LM pp.
 *     1244-1246, CM pp. 1351-1353), and the constants (constants.c: LM pp.
 *     1246-1247, CM pp. 1353-1354).
 * The binding (src/binding/orbital_integration_routines.c) gives every key
 * of the faithful port's tables its readable routine. */
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

/* Part 2: OBLATE to COSPHIE; the LM's QUALITY1 and QUALITY2. */
extern const struct interpretive_code OBLATE_CODE;
#if LUMINARY099
extern const struct interpretive_code QUALITY1_CODE;
#endif

/* Part 3: DIFEQTAB to DIFEQ0 with the sphere checks; DOSWITCH, ORIGCHNG,
 * RECTIFY and MINIRECT. */
extern const struct interpretive_code TIMESTEP_CODE, DOSWITCH_CODE;

/* Part 4: DIFEQ+0 to ENDSTATE's EXIT; after its TC INTPRET to WMATEND's
 * EXIT; after WMATEND's TC INTPRET to DOW..1 (and the LM's SETITCTR); the
 * constants of ORBITAL1 and ORBITAL2. */
extern const struct interpretive_code DIFEQ_CODE, AMOVED_CODE, DOW_CODE;
extern const struct interpretive_code ORBITAL1_CONSTANTS, ORBITAL2_CONSTANTS;

/* The basic code: after INT-ABRT's EXIT (part 1), ENDSTATE's and
 * WMATEND's (part 4). */
void int_abrt_after_exit(void);     /* INT-ABRT +1: TC POODOO */
void endstate_after_exit(void);     /* ENDSTATE +014: TC PHASCHNG */
void endstate_after_phaschng(void); /* ENDSTATE +016: TC UPFLAG */
void endstate_after_upflag(void);   /* ENDSTATE +020: TC INTPRET */
void wmatend_after_exit(void);      /* WMATEND +7: TC ALARM */
void wmatend_after_alarm(void);     /* WMATEND +011: TC INTPRET */

#endif
