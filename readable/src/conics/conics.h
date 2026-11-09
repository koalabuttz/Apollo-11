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
 *     1283-1284) and CHECKCTR (checkctr.c: LM p. 1183, CM p. 1286);
 *   part 2: ITERATOR (iterator.c: LM pp. 1182-1183, CM pp. 1285-1286) and
 *     NEWSTATE with LAMENTER (newstate.c: LM p. 1184, CM p. 1287);
 *   part 3: TIME-THETA (timethet.c: LM p. 1186, CM p. 1289), PARAM and
 *     GEOM (param.c: LM pp. 1186-1188, CM pp. 1289-1291) and GETX (getx.c:
 *     LM pp. 1189-1192, CM pp. 1292-1295);
 *   part 4: LAMBERT (lambert.c: LM pp. 1193-1197, CM pp. 1296-1300);
 *   part 5: TIME-RADIUS (timerad.c: LM pp. 1198-1199, CM pp.
 *     1301-1302), APSIDES and ABTCONIC (apsides.c: LM pp. 1200-1201, CM
 *     pp. 1303-1304) and the constants in bank 4 (constants.c: LM p.
 *     1185, CM p. 1288).
 * With part 5 the file is whole: the binding
 * (src/binding/conics_routines.c) runs readable code at every key. */
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

/* Part 2: ITERATOR, and NEWSTATE with LAMENTER. */
extern const struct interpretive_code ITERATOR_CODE, NEWSTATE_CODE;

/* Part 3: TIME-THETA, PARAM with GEOM (in bank 4), and GETX's two
 * stretches (to its EXIT, and after its POLY). */
extern const struct interpretive_code TIMETHET_CODE, PARAM_CODE, GETX_CODE, GETX_SERIES_CODE;

/* Part 4: LAMBERT. */
extern const struct interpretive_code LAMBERT_CODE;

/* Part 5: TIME-RADIUS, APSIDES with ABTCONIC, and the constants. */
extern const struct interpretive_code TIMERAD_CODE, APSIDES_CODE, CONIC_CONSTANTS;

/* DELTIME's basic code: the two series by POLY. */
void deltime_after_exit(void);        /* DELTIME +1: TC POLY, S(XI)'s series */
void deltime_after_poly(void);        /* DELTIME +027: TC INTPRET */
void deltime_after_second_exit(void); /* DELTIME +033: TC POLY, C(XI)'s series */
void deltime_after_second_poly(void); /* DELTIME +061: TC INTPRET */

/* GETX's basic code: its series by POLY. */
void getx_after_exit(void); /* POLYCOEF +010: TC POLY */
void getx_after_poly(void); /* POLYCOEF +030: TC INTPRET */

/* ABTCONIC's basic code: alarm 607. */
void abtconic_after_exit(void); /* ABTCONIC +1: TC POODOO */

/* CHECKCTR: ITERCTR counted down, to MPAC, then DANZIG. KEPLER and
 * ITERATOR reach it by RTB. */
void checkctr(void);

#endif
