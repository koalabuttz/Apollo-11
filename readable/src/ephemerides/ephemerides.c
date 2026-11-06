/* SPDX-License-Identifier: MIT */
/* LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc (Comanche055 pages
 * 785-788): its interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/ephemerides_routines.c). The faithful C is
 * c64-port/src/cm_ephemeris.c. */
#include "ephemerides.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if COMANCHE055

/* LUNAR AND SOLAR EPHEMERIDES SUBROUTINES -- CM pp. 785-788.
 *
 * The caller loads the time, ground elapsed time in centiseconds ("CS x
 * B-28 LOADED IN MPAC"), and CALLs one of the four entries. The ground's
 * prelaunch load holds the rest: TIMEMO, "TIME AT THE CENTER OF THE RANGE
 * OVER WHICH THE LUNAR POSITION POLYNOMIAL IS VALID"; VECOEM, the
 * polynomial's ten vector coefficients, "LOADED IN DESCENDING SEQUENCE";
 * RESO and VESO, the sun's position and velocity at TIMEMO; and OMEGAES,
 * the "ANGULAR VELOCITY OF THE VECTOR RESO AT TIMEMO".
 *
 * Each entry puts the routine it wants in X1 and goes to LSTIME, which
 * turns the time into time from TIMEMO and goes on through X1:
 *
 *   LSPOS  X1 = RES, X2 = RESA: the sun's position, then on to REM for
 *          the moon's. "THE POSITION VECTOR OF THE SUN IS STORED IN 2D OF
 *          THE VAC AREA. THE POSITION VECTOR OF THE MOON IS STORED IN
 *          MPAC."
 *   LUNPOS X1 = REM: the moon's position.
 *   LUNVEL X1 = VEM: the moon's velocity.
 *   SOLPOS X1 = RES, and X2 the caller's return (STQ X2), where RES goes
 *          when it is done.
 *
 * The moon's routines leave by RVQ, to the caller's CALL. */
static const word ephemerides[] = {
    /* LSPOS */ ORDER(AXT_2), /* "COMPUTES POSITION VECTORS OF BOTH THE" */
    ADDRESS(RESA),            /* (X2: where RES goes when done) */
    ORDERS(AXT_1, GOTO),
    ADDRESS(RES),
    ADDRESS(LSTIME),
    /* LUNPOS */ ORDERS(AXT_1, GOTO), /* "COMPUTES THE POSITION VECTOR OF THE MOON" */
    ADDRESS(REM),
    ADDRESS(LSTIME),
    /* LUNVEL */ ORDERS(AXT_1, GOTO), /* "COMPUTES THE VELOCITY VECTOR OF THE MOON" */
    ADDRESS(VEM),
    ADDRESS(LSTIME),
    /* SOLPOS */ ORDERS(STQ, AXT_1), /* "COMPUTES THE POSITION VECTOR OF THE SUN" */
    ADDRESS(X2),                     /* (the caller's return, for RES's GOTO X2) */
    ADDRESS(RES),

    /* LSTIME: the time from TIMEMO. The time, shifted from B-28 to B-42
     * (as TEPHEM and TIMEMO are), plus TEPHEM, less TIMEMO (a sum
     * complemented, TIMEMO added, the sum complemented again), shifted left
     * 16. S1 = 6, the step of the coefficient loops (a vector's
     * six words). Then on to the routine in X1. */
    /* LSTIME */ ORDERS(SETPD, SR),
    ADDRESS_PLUS_ONE_D(0),
    SR_COUNT(14),
    ORDERS(TAD, DCOMP),
    ADDRESS_PLUS_ONE(TEPHEM),
    ORDERS(TAD, DCOMP),
    ADDRESS_PLUS_ONE(TIMEMO),
    ORDERS(SL, SSP),
    SL_COUNT(16),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDER(GOTO),
    ADDRESS(X1),

    /* RES: the sun's position. RESO turned about the earth by OMEGAES
     * times the time, in the plane of RESO and VESO: with N the unit
     * normal of that plane, UNIT(UNIT(RESO) * VESO),
     *   RES = RESO COS(A) + (N * RESO) SIN(A), A = OMEGAES T,
     * the push-down list holding T, A, RESO COS(A), SIN(A) and RESO along
     * the way (the PD- notes). Then on through X2. */
    /* RES */ ORDERS(PUSH, DMP), /* PD- 2 */
    ADDRESS_PLUS_ONE(OMEGAES),
    ORDERS(PUSH, COS),  /* PD- 4 */
    ORDERS(VXSC, PDDL), /* PD- 8 */
    ADDRESS_PLUS_ONE(RESO),
    ORDERS(SIN, PDVL), /* PD-10 */
    ADDRESS_PLUS_ONE(RESO),
    ORDERS(PUSH, UNIT), /* PD-16 */
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(VESO),
    ORDERS(VXV, VSL1),  /* PD-10 */
    ORDERS(VXSC, VAD),  /* PD-02 */
    ORDERS(VSL1, GOTO), /* "RES IN METERS x B-38 IN MPAC." */
    ADDRESS(X2),

    /* RESA: LSPOS's way on. The sun's position to 2D; the time back,
     * off the push-down list (the DL has no address). */
    /* RESA */ STODL_D(2), /* "RES IN METERS x B-38 IN 2D OF VAC." */

    /* REM: the moon's position, the polynomial by Horner's rule. The
     * highest coefficient first; then nine times over, times the time and
     * the next coefficient added (VECOEM +60D less X1: X1 from 54D down
     * by S1, TIX looping while it stays above zero). */
    /* REM */ ORDERS(AXT_1, PDVL), /* PD- 2 */
    NUMBER(54),
    ADDRESS_PLUS_ONE(VECOEM),
    /* REMA */ ORDERS(VXSC, VAD_STAR),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_AT_X1(VECOEM, 60),
    ORDERS(TIX_1, VSL2), /* "REM IN METERS x B-29 IN MPAC." */
    ADDRESS(REMA),
    ORDER(RVQ),

    /* VEM: the moon's velocity, the polynomial's derivative by Horner's
     * rule. 9 times the highest coefficient first; then eight times over,
     * times the time, and N - 1 times the next coefficient added, N (in
     * 2D) counting down from 9. */
    /* VEM */ ORDERS(AXT_1, PDDL), /* PD- 2 */
    NUMBER(48),
    ADDRESS_PLUS_ONE(NINEB4),
    ORDERS(PUSH, VXSC), /* PD- 4 */
    ADDRESS_PLUS_ONE(VECOEM),
    /* VEMA */ ORDER(VXSC),
    ADDRESS_PLUS_ONE_D(0),
    STODL_D(4),        /* PD- 2 */
    ORDERS(DSU, PUSH), /* PD- 4 */
    ADDRESS_PLUS_ONE(ONEB4),
    ORDERS(VXSC_STAR, VAD),
    ADDRESS_PLUS_ONE_AT_X1(VECOEM, 54),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(TIX_1, VSL2), /* "VEM IN METERS/CS x B-7 IN MPAC." */
    ADDRESS(VEMA),
    ORDER(RVQ),

    /* The constants VEM counts with: 9 and 1, B-4. */
    /* NINEB4 */ NUMBER(022000), /* 2DEC 9.0 B-4 */
    NUMBER(0),
    /* ONEB4 */ NUMBER(02000), /* 2DEC 1.0 B-4 */
    NUMBER(0),
};

const struct interpretive_code EPHEMERIDES_CODE = {026u, K_LSPOS, COUNT(ephemerides), ephemerides};

#endif
