/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc (Luminary099 pages 1159-1204, Comanche055 pages
 * 1262-1308). The faithful C is c64-port/src/lm_conic_compiled.c (with
 * lm_conic_checkctr.c, which runs CHECKCTR at its RTB) and cm_conics.c.
 *
 * The subroutines that "SOLVE VARIOUS PROBLEMS INVOLVING THE TRAJECTORY
 * PRODUCED BY A CENTRAL INVERSE-SQUARE FORCE ACTING ON A POINT MASS": a
 * conic about the earth or the moon (the central body's constants are its
 * entry in MUTABLE, picked by X1). KEPLER moves a state along its conic by
 * a time; LAMBERT finds the velocity that takes one position to another in
 * a time; TIME-THETA and TIME-RADIUS the time to a true anomaly or to a
 * radius; APSIDES the apsides. They share DELTIME (Kepler's equation:
 * the time a value of the universal variable X gives), ITERATOR (the
 * search for the root) and the lower routines PARAM, GEOM and GETX. Pages
 * 1159-1173 describe them all (inputs, outputs, scaling, timing); the code
 * starts on page 1174. Nearly all of it is interpretive code, the same in
 * both missions but for a few words (the CM stores and branches with one
 * STCALL where the LM has a STORE and a GOTO).
 *
 * Written in parts (README.md):
 *   part 1: KEPLER (kepler.c: KEPLERN to KEPCONVG, LM pp. 1174-1179, CM
 *     pp. 1277-1282), DELTIME (deltime.c: LM pp. 1180-1181, CM pp.
 *     1283-1284) and CHECKCTR (checkctr.c: LM p. 1183, CM p. 1286).
 * The binding (src/binding/conics_routines.c) runs the faithful port's
 * routines at the keys no part has written yet. */
#ifndef READABLE_CONICS_H
#define READABLE_CONICS_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* TC at, to a routine out of the file: Q gets the return, and the routine
 * runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* Part 1: KEPLER, and DELTIME's three stretches of interpretive code
 * (its EXIT, the store between its POLYs, and the rest after the second
 * POLY). */
extern const struct interpretive_code KEPLER_CODE, DELTIME_CODE, DELTIME_S_CODE, DELTIME_C_CODE;

/* DELTIME's basic code: the two series by POLY. */
void deltime_after_exit(void);        /* DELTIME +1: TC POLY, S(XI)'s series */
void deltime_after_poly(void);        /* DELTIME +027: TC INTPRET */
void deltime_after_second_exit(void); /* DELTIME +033: TC POLY, C(XI)'s series */
void deltime_after_second_poly(void); /* DELTIME +061: TC INTPRET */

/* CHECKCTR: ITERCTR counted down, to MPAC, then DANZIG. KEPLER and
 * ITERATOR reach it by RTB. */
void checkctr(void);

#endif
