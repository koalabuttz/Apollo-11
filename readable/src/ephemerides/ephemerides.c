/* SPDX-License-Identifier: MIT */
/* LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc (Luminary099 pages 984-987,
 * Comanche055 pages 785-788): its interpretive code in the listing's
 * notation (src/interpreter/notation.h), which the readable interpreter
 * runs (src/binding/ephemerides_routines.c). The faithful C is
 * c64-port/src/lm_ephemeris.c and cm_ephemeris.c. */
#include "ephemerides.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* LSPOS -- LM pp. 984-987.
 * "COMPUTES UNIT POSITION VECTOR OF THE SUN AND MOON IN THE BASIC
 * REFERENCE SYSTEM", from angles that grow at a mean rate through the
 * year, each less a periodic correction ("FUDGE FACTORS TO MINIMIZE THE
 * DEVIATION"). The constants are in fixed memory, "UPDATED BY YEAR": the
 * mean angles at midnight June 30th and their rates (RATESP), the
 * corrections' amplitudes, phases and periods (VAL67) and the matrix of the
 * obliquity and the moon's inclination (KONMAT, whose third word is a
 * double precision zero). LUNPOS is LSPOS. The work area's 16D to 24D
 * hold the angles (STMP), the correction (GTMP) and the time (TIMEP).
 *
 * LSPOS: the time, as the AGC clock's (TIME2 and TIME1) plus TEPHEM,
 * "TIME FROM MIGNIGHT 1 JULY PRECEDING THE LAUNCH", divided by CSTODAY
 * into days, to TIMEP; X1 and X2 zero; FREEFLAG clear. Then three angles,
 * X2 stepping by -2 through STMP, X1 by -6 through VAL67:
 *
 *   POSITA  GTMP the sum of the corrections: at POSITB one amplitude
 *           times the sine of the time over a period plus a phase, for
 *           the moon's longitude twice over (BOFSET: once with FREEFLAG
 *           clear, which sets it, once more), for the sun's once.
 *   POSITD  the angle: its value at midnight June 30th, plus its rate
 *           times the time, less GTMP, to STMP,2 (LOM, LOS, LON). With
 *           X2 as it was plus 2, BZE goes to POSITE (no correction, for
 *           the moon's node) after the sun's, BPL back to POSITA for the
 *           sun's after the moon's, and on to POSITF after the third.
 *   POSITF  the moon: COS(LOM), SIN(LOM) and SIN(LOM-LON) into a vector
 *           (VDEF), through KONMAT, unit, to VMOON; the sun: COS(LOS),
 *           SIN(LOS) and zero, through KONMAT, unit, to VSUN. RVQ.
 *
 * LUNVEL is an RVQ alone, "TO FOOL INTEGRATION". */
static const word lspos[] = {
    /* LSPOS */ ORDERS(SETPD, SR),
    ADDRESS_PLUS_ONE_D(0),
    SR_COUNT(14), /* "TP" */
    ORDERS(TAD, DDV),
    ADDRESS_PLUS_ONE(TEPHEM),  /* "TIME OF LAUNCH" */
    ADDRESS_PLUS_ONE(CSTODAY), /* "24 HOURS-8640000 CENTI-SECS/DAY B-33" */
    STORE_D(24),               /* TIMEP: "T IN DAYS" */
    ORDERS(AXT_1, AXT_2),
    NUMBER(0),
    NUMBER(0),
    ORDER(CLEAR),
    SWITCH(CLEAR, FREEFLAG),
    /* POSITA */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_AT(KONMAT, 2), /* "ZEROS" */
    STORE_D(22),                    /* GTMP */
    /* POSITB */ ORDERS(DLOAD, DMP_STAR),
    ADDRESS_PLUS_ONE_D(24),           /* "T" */
    ADDRESS_PLUS_ONE_AT_X1(VAL67, 4), /* "1/27 OR 1/32 OR 1/365" */
    ORDERS(SL, DAD_STAR),
    SL_COUNT(8),
    ADDRESS_PLUS_ONE_AT_X1(VAL67, 2), /* "AARG" */
    ORDERS(SIN, DMP_STAR),            /* "SIN(T/27+PHI) OR T/32 OR T/365" */
    ADDRESS_PLUS_ONE_AT_X1(VAL67, 0),
    ORDERS(DAD, INCR_1), /* "PLUS" */
    ADDRESS_PLUS_ONE_D(22),
    MINUS_NUMBER(6),
    STORE_D(22),
    ORDER(BOFSET),
    SWITCH(BOFSET, FREEFLAG),
    ADDRESS(POSITB),
    /* POSITD */ ORDERS(DLOAD, DMP_STAR),
    ADDRESS_PLUS_ONE_D(24),            /* "T" */
    ADDRESS_PLUS_ONE_AT_X2(RATESP, 0), /* "LOMR,LOSR,LONR" */
    ORDERS(SL, DAD_STAR),
    SL_COUNT(5),
    ADDRESS_PLUS_ONE_AT_X2(RATESP, 6), /* "LOMO,LOSO,LONO" */
    ORDER(DSU),
    ADDRESS_PLUS_ONE_D(22),
    STORE_X2_D(16), /* STMP,2: "LOM,LOS,LON" */
    ORDERS(SLOAD, INCR_2),
    ADDRESS_PLUS_ONE(X2),
    MINUS_NUMBER(2),
    ORDERS(DAD, BZE),
    ADDRESS_PLUS_ONE(RCB_13), /* "PLUS 2" */
    ADDRESS(POSITE),          /* "2ND" */
    ORDER(BPL),
    ADDRESS(POSITA),                 /* "1ST" */
    /* POSITF */ ORDERS(DLOAD, DSU), /* "3RD" */
    ADDRESS_PLUS_ONE_D(16),          /* "LOM" */
    ADDRESS_PLUS_ONE_D(20),          /* "LON" */
    ORDERS(SIN, PDDL),               /* "SIN(LOM-LON)" */
    ADDRESS_PLUS_ONE_D(16),
    ORDERS(SIN, PDDL), /* "SIN LOM" */
    ADDRESS_PLUS_ONE_D(16),
    ORDERS(COS, VDEF), /* "COS LOM" */
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(KONMAT), /* "K1,K2,K3,K4," */
    STORE(VMOON),
    ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE_AT(KONMAT, 2), /* "ZERO" */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(SIN, PDDL), /* "SIN LOS" */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(COS, VDEF), /* "COS LOS" */
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(KONMAT),
    STORE(VSUN),
    ORDER(RVQ),
    /* POSITE */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_AT(KONMAT, 2), /* "ZEROS" */
    STORE_D(22),
    ORDER(GOTO),
    ADDRESS(POSITD),
    /* LUNVEL */ ORDER(RVQ), /* "TO FOOL INTEGRATION" */
};

const struct interpretive_code EPHEMERIDES_CODE = {015u, K_LSPOS, COUNT(lspos), lspos};

#else

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
