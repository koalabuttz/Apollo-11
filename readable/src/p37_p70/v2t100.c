/* SPDX-License-Identifier: MIT */
/* P37_P70.agc of the CM, part 4 (Comanche055 pp. 914-923, banks 31 and
 * 32): the "INTEGRATION CALLING SUBROUTINE" (RTENCK1, RTENCK2 and
 * RTENCK3), and the "V2(T1) COMPUTATION SUBROUTINE" (V2T100), which finds
 * the velocity just after the burn: one that "MEETS THE INPUT VELOCITY
 * CHANGE DESIRED (RTEDVD) IN A MINIMUM TIME", or if none is asked for
 * one that "MINIMIZES THE IMPULSE (DV)". */
#include "p37_p70.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- RTENCK1, RTENCK2 and RTENCK3 -- pp. 914-916 ---- */

/* Three entries, each called by STQ: the return in RTENCKEX, and
 * INTSTALL first. RTENCK1, a conic (INTYPFLG set), and RTENCK3, a
 * precision integration (RTENCK3A: INTYPFLG clear), both from R(T1)/ and
 * V2(T1)/ at T1 to T2 (RTENCK3B). RTENCK2, a precision integration from
 * R(T2)/ and V(T2)/ at T2, on by the time on the push-down list (DAD).
 * RTENCK3D: INTEGRVS to TDEC1, about the earth (MOONFLAG clear). The end
 * state as R(T2)/ (pushed as well), V(T2)/ (left in MPAC) and T2; X1 from
 * CONICX1, as the conics take it; and back through RTENCKEX. */
static const word rtenck_words[] = {
    /* RTENCK1 */ ORDERS(STQ, CALL),
    ADDRESS(RTENCKEX),
    ADDRESS(INTSTALL),
    ORDERS(VLOAD, SET),
    ADDRESS_PLUS_ONE(R_T1_),
    SWITCH(SET, INTYPFLG),
    ORDER(GOTO),
    ADDRESS(RTENCK3B),
    /* RTENCK2 */ ORDERS(STQ, CALL),
    ADDRESS(RTENCKEX),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, VLOAD),
    SWITCH(CLEAR, INTYPFLG),
    ADDRESS_PLUS_ONE(R_T2_),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(V_T2_),
    STODL(VCV),
    ADDRESS_PLUS_ONE(T2),
    STORE(TET),
    ORDER(DAD),
    ORDER(GOTO),
    ADDRESS(RTENCK3D),
    /* RTENCK3 */ ORDERS(STQ, CALL),
    ADDRESS(RTENCKEX),
    ADDRESS(INTSTALL),
    /* RTENCK3A */ ORDERS(VLOAD, CLEAR),
    ADDRESS_PLUS_ONE(R_T1_),
    SWITCH(CLEAR, INTYPFLG),
    /* RTENCK3B */ STOVL(RCV),
    ADDRESS_PLUS_ONE(V2_T1_),
    STODL(VCV),
    ADDRESS_PLUS_ONE(T1),
    STODL(TET),
    ADDRESS_PLUS_ONE(T2),
    /* RTENCK3D */ STORE(TDEC1),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, MOONFLAG),
    ADDRESS(INTEGRVS),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STORE(R_T2_),
    ORDERS(PDDL, LXC_1),
    ADDRESS_PLUS_ONE(TAT),
    ADDRESS(CONICX1),
    STOVL(T2),
    ADDRESS_PLUS_ONE(VATT),
    STORE(V_T2_),
    ORDER(GOTO),
    ADDRESS(RTENCKEX),
};

const struct interpretive_code RTENCK_CODE = {031u, RTENCK1, COUNT(rtenck_words), rtenck_words};

/* ---- V2T100 -- pp. 917-923 ---- */

/* V2T100, called by STQ: its return in 33D. With RCON negative, or the
 * radius at ignition, R(T1), not above it ("LAMBDA LESS THAN ONE"): the
 * EXIT at V2TERROR, then the abort (below). */
static const word v2t100_words[] = {
    /* V2T100 */ ORDERS(STQ, DLOAD),
    ADDRESS_D(33),
    ADDRESS_PLUS_ONE(RCON),
    ORDERS(BMN, DSU), /* "ABORT IF RCON NEGATIVE" */
    ADDRESS(V2TERROR),
    ADDRESS_PLUS_ONE(R_T1),
    ORDER(BMN),
    ADDRESS(V2T101),
    /* V2TERROR */ ORDER(EXIT), /* "OR IF LAMBDA LESS THAN ONE" */
};

const struct interpretive_code V2T100_CODE = {032u, V2T100, COUNT(v2t100_words), v2t100_words};

/* V2TERROR +1, after the EXIT: the program's abort with alarm 610 (TC
 * POODOO, OCT 00610): "NO SOLUTION IF LAMBDA LESS THAN 1". */
void v2terror_after_exit(void)
{
    tc_to(V2TERROR + 1u, POODOO); /* OCT 00610 */
}

/* V2T101: the push-down list from 0D, F2RTE clear. "R1/RCON = LAMBDA";
 * BETA1, "1+X(T2)**2"; BETA5, BETA1 times LAMBDA (28D); and in the
 * push-down list THETA1, BETA5*LAMBDA-1, THETA2, 2*R(T1)*(LAMBDA-1), and
 * THETA3, the square root of mu (RTMURTE) over R(T1). XT1LIM turns the
 * major axis limits into the range of X(T1), the cotangent of the flight
 * path angle after the burn: from MAMAX1, X(T1)MIN (10D) and the largest
 * step, DX(T1)MAX (12D), a sixteenth of its size; from MAMAX2, X(T1)MAX (14D).
 * A conic (NN1A negative): on at V2T102; a precision trajectory: on at
 * V2T110. */
static const word v2t101_words[] = {
    /* V2T101 */ ORDERS(SETPD, CLEAR),
    ADDRESS_PLUS_ONE_D(0),
    SWITCH(CLEAR, F2RTE),
    ORDERS(DLOAD, NORM),
    ADDRESS_PLUS_ONE(RCON),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE(R_T1),
    ADDRESS_PLUS_ONE(S1),
    STORE_D(10),
    ORDERS(SR1, DDV), /* "R1/RCON = LAMBDA" */
    ORDERS(XSU_1, PDDL),
    ADDRESS(S1),
    ADDRESS_PLUS_ONE(X_T2),
    ORDER(DSQ),
    ORDERS(SR1, DAD),
    ADDRESS_PLUS_ONE(1RTEB1),
    STORE(BETA1), /* "1+X(T2)**2 = BETA1" */
    ORDER(DMP),
    ADDRESS_PLUS_ONE_D(0),
    STORE_D(28), /* "BETAI*LAMBDA = BETA5" */
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE_D(0),
    SL_X1(-7),
    ORDERS(SL_STAR, DSU),
    SL_X1(-7),
    ADDRESS_PLUS_ONE(1RTEB17),
    ORDERS(RTB, PDDL), /* "BETA5*LAMBDA-1 = THETA1" */
    ADDRESS(TPMODE),
    ADDRESS_PLUS_ONE(1RTEB1),
    ORDERS(SR_STAR, DCOMP),
    SR_X1(0),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(R_T1),
    ORDERS(SL_STAR, RTB),
    SL_X1(-7),
    ADDRESS(TPMODE),
    ORDER(PDDL), /* "2*R(T1)*(LAMBDA-1)=THETA2" */
    ADDRESS_PLUS_ONE(RTMURTE),
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(XSU_2, DDV),
    ADDRESS(S1),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(SR_STAR, PDDL), /* "MU**.5/R(T1)=THETA3" */
    SR_X2(6),
    ADDRESS_PLUS_ONE(MAMAX1),
    ORDERS(PUSH, PUSH), /* "MAMAX1=MA" */
    ORDER(CALL),
    ADDRESS(XT1LIM),
    ORDERS(DCOMP, PUSH), /* "X(T1)MIN" */
    ORDERS(DCOMP, SR4),
    ORDERS(PDDL, PUSH), /* "DX(T1)MAX" */
    ADDRESS_PLUS_ONE(MAMAX2),
    ORDERS(PUSH, CALL),
    ADDRESS(XT1LIM),
    ORDERS(PDDL, BMN), /* "X(T1)MAX" */
    ADDRESS_PLUS_ONE(NN1A),
    ADDRESS(V2T102),
    ORDER(GOTO),
    ADDRESS(V2T110),
};

const struct interpretive_code V2T101_CODE = {032u, V2TERROR + 3u, COUNT(v2t101_words),
                                              v2t101_words};

/* V2T102, a conic: the search starts at X(T1)MIN, stepping up (V2T140),
 * when a change in speed is asked for (RTEDVD) or the flight path angle
 * points down (CFPA negative); else at X(T1)MAX, stepping down (V2T145).
 *
 * V2T110, a precision trajectory ("CONSTRAIN THE INDEPENDENT VARIABLE TO
 * INSURE THAT ALL CONICS PASS THROUGH RCON ON THE SAME PASS THROUGH
 * X(T2)"): the last X(T1) kept at 00D ("X(T1),,"), and BETA6,
 * "BETA5(2-LAMBDA)-1". Not above +0 (V2T115): PHI2 made negative if it
 * was not, and the ten tries begun again (NN1A 10RTE). Else (V2T120) the
 * limit X(T1)LIM, "BETA6**.5": with PHI2 not negative the range is
 * -X(T1)LIM to X(T1)LIM; with it negative (V2T130) it keeps only the part
 * beyond X(T1)LIM on X(T1)'s side of +0. V2T125: the search starts at X(T1)MIN
 * if X(T1) is not above +0, else at X(T1)MAX.
 *
 * V2T150: GAMDV10, the "X(T1)-DV ITERATOR". With no change in speed asked
 * for, done ("EXIT IF MINIMUM FUEL MODE"). Else ("TIME CRITICAL MODE"),
 * with RTEDVD not below the DV found: on at V2T175. Below it: a conic
 * that started at X(T1)MIN (X(T1) not above +0) and points up (CFPA
 * positive) searches again down from X(T1)MAX (V2T165) and goes on at
 * V2T175 unless the DV is still above RTEDVD; other conics are done; a
 * precision trajectory goes on at V2T185.
 *
 * V2T175: F2RTE set, and GAMDV10 again, with X(T1) the new X(T1)MIN and
 * stepping up if SLOWFLG is set, else the new X(T1)MAX and stepping down.
 * A conic is done.
 *
 * V2T185, to "PREVENT A LARGE CHANGE IN INDEPENDENT VARIABLE DURING AN
 * ITERATION FOR A PRECISION TRAJECTORY": if X(T1) moved more than twice
 * DX(T1)MAX from the last one ("BETA7"), the last one again, held within
 * X(T1)MIN and X(T1)MAX, and DVCALC the DV for it (V2T205). V2T300: MPAC
 * +0; and back through 33D (V2T1X). */
static const word v2t102_words[] = {
    /* V2T102 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(RTEDVD),
    ORDERS(BZE, GOTO),
    ADDRESS(V2T105),
    ADDRESS(V2T140),
    /* V2T105 */ ORDERS(DLOAD, BMN),
    ADDRESS_PLUS_ONE(CFPA),
    ADDRESS(V2T140),
    ORDER(GOTO),
    ADDRESS(V2T145),
    /* V2T110 */ ORDERS(DLOAD, RTB),
    ADDRESS_PLUS_ONE(1RTEB17),
    ADDRESS(TPMODE),
    ORDERS(DCOMP, PDDL), /* "-1" */
    ADDRESS_PLUS_ONE(2RTEB1),
    ORDERS(SR_STAR, DSU),
    SR_X1(0),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE_D(28),
    SL_X1(-7),
    ORDERS(SL_STAR, TAD),
    SL_X1(-7),
    ORDERS(RTB, PDDL), /* "BETA5(2-LAMBDA)-1=BETA6" */
    ADDRESS(TPMODE),
    ADDRESS_PLUS_ONE(X_T1),
    STORE_D(0), /* "X(T1),," */
    ORDER(TLOAD),
    ORDERS(BMN, BZE),
    ADDRESS(V2T115),
    ADDRESS(V2T115),
    ORDERS(SL, GOTO),
    SL_COUNT(7),
    ADDRESS(V2T120),
    /* V2T115 */ ORDERS(DLOAD, BMN),
    ADDRESS_PLUS_ONE(PHI2),
    ADDRESS(V2T125),
    ORDER(DCOMP),
    STODL(PHI2),
    ADDRESS_PLUS_ONE(10RTE),
    STORE(NN1A),
    ORDER(GOTO),
    ADDRESS(V2T125),
    /* V2T120 */ ORDERS(SQRT, RTB),
    ADDRESS(DPMODE),
    ORDERS(PDDL, BMN), /* "BETA6**.5=X(T1)LIM" */
    ADDRESS_PLUS_ONE(PHI2),
    ADDRESS(V2T130),
    ORDERS(DLOAD, STADR),
    AFTER_STADR(STORE_D(14)), /* "X(T1)LIM = X(T1)MAX" */
    ORDER(DCOMP),
    STORE_D(10), /* "-X(T1)LIM = X(T1)MIN" */
    /* V2T125 */ ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS(V2T140),
    ORDERS(BMN, GOTO),
    ADDRESS(V2T140),
    ADDRESS(V2T145),
    /* V2T130 */ ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS(V2T135),
    ORDERS(BMN, DLOAD),
    ADDRESS(V2T135),
    ORDER(STADR),
    AFTER_STADR(STORE_D(10)), /* "X(T1)LIM = X(T1)MIN" */
    ORDER(GOTO),
    ADDRESS(V2T145),
    /* V2T135 */ ORDERS(DLOAD, DCOMP),
    ORDER(STADR),
    AFTER_STADR(STORE_D(14)), /* "-X(T1)LIM = X(T1)MAX" */
    /* V2T140 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    STODL(X_T1), /* "X(T1)MIN = X(T1)" */
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(PUSH, GOTO), /* "DX(T1)MAX = DX(T1)" */
    ADDRESS(V2T150),
    /* V2T145 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(14),
    STODL(X_T1), /* "X(T1)MAX = X(T1)" */
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(DCOMP, PUSH),      /* "-DX(T1)MAX = DX(T1)" */
    /* V2T150 */ ORDER(CALL), /* "GOTO X(T1)-DV ITERATOR" */
    ADDRESS(GAMDV10),
    ORDERS(DLOAD, BZE), /* "EXIT IF MINIMUM FUEL MODE" */
    ADDRESS_PLUS_ONE(RTEDVD),
    ADDRESS(V2T1X),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(DV),
    ADDRESS(V2T155),
    ORDER(GOTO),
    ADDRESS(V2T175),
    /* V2T155 */ ORDERS(DLOAD, BMN),
    ADDRESS_PLUS_ONE(NN1A),
    ADDRESS(V2T160),
    ORDER(GOTO),
    ADDRESS(V2T185),
    /* V2T160 */ ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS(V2T165),
    ORDERS(BMN, GOTO),
    ADDRESS(V2T165),
    ADDRESS(V2T300),
    /* V2T165 */ ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(CFPA),
    ADDRESS(V2T300),
    ORDERS(BMN, DLOAD),
    ADDRESS(V2T300),
    ADDRESS_PLUS_ONE_D(14),
    STODL(X_T1), /* "X(T1)MAX=X(T1)" */
    ADDRESS_PLUS_ONE_D(12),
    ORDER(DCOMP),
    STCALL_D(16), /* "-DX(T1)MAX=DX(T1)" */
    ADDRESS(GAMDV10),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(RTEDVD),
    ADDRESS_PLUS_ONE(DV),
    ORDER(BMN),
    ADDRESS(V2T300),
    /* V2T175 */ ORDERS(SET, DLOAD),
    SWITCH(SET, F2RTE),
    ADDRESS_PLUS_ONE(X_T1),
    ORDER(BOFF),
    SWITCH(BOFF, SLOWFLG),
    ADDRESS(V2T177),
    STODL_D(10),            /* "X(T1)MIN" */
    ADDRESS_PLUS_ONE_D(12), /* "DX(T1)MAX" */
    ORDER(GOTO),
    ADDRESS(V2T179),
    /* V2T177 */ STODL_D(14),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(DCOMP),
    /* V2T179 */ STCALL_D(16), /* "DX(T1)" */
    ADDRESS(GAMDV10),
    ORDERS(DLOAD, BMN),
    ADDRESS_PLUS_ONE(NN1A),
    ADDRESS(V2T300),
    /* V2T185 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(ABS, PDDL), /* "/X(T1)-X(T1),,/ = BETA7" */
    ADDRESS_PLUS_ONE_D(12),
    ORDERS(SL1, BDSU),
    ORDERS(BMN, DLOAD),
    ADDRESS(V2T300),
    ADDRESS_PLUS_ONE_D(0), /* "CONTINUE IF BETA7 LARGER THAN 2DX(T1)MAX" */
    STORE(X_T1),           /* "X(T1),, = X(T1)" */
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE_D(14),
    ADDRESS(V2T195),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(14),
    STORE(X_T1), /* "X(T1)MAX = X(T1)" */
    ORDER(GOTO),
    ADDRESS(V2T205),
    /* V2T195 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(BMN, GOTO),
    ADDRESS(V2T200),
    ADDRESS(V2T205),
    /* V2T200 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    STORE(X_T1), /* "X(T1)MIN = X(T1)" */
    /* V2T205 */ ORDER(CALL),
    ADDRESS(DVCALC),
    /* V2T300 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZERORTE),
    /* V2T1X */ ORDER(GOTO),
    ADDRESS_D(33),
};

const struct interpretive_code V2T102_CODE = {032u, V2TERROR + 3u + COUNT(v2t101_words),
                                              COUNT(v2t102_words), v2t102_words};

#endif
