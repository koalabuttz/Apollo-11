/* SPDX-License-Identifier: MIT */
/* P37_P70.agc of the CM, part 3 (Comanche055 pp. 905-913, banks 32 and
 * 16): the "INITIAL VECTOR SUBROUTINE" (INVC100), which brings the CSM's
 * state to the time of ignition, and the "PRECISION TRAJECTORY
 * COMPUTATION SUBROUTINE" (PREC100), which turns the conic trajectory home
 * into an integrated one: "A NUMERICALLY INTEGRATED TRAJECTORY IS
 * GENERATED WHICH FOR THE RETURN TO EARTH PROBLEM SATISFIES THE REENTRY"
 * "CONSTRAINTS (RCON AND X(T2)) ACHIEVED BY THE INITIAL CONIC TRAJECTORY".
 * Each returns through SPRTEX with MPAC +0, or an alarm's code in it. */
#include "p37_p70.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- INVC100 -- pp. 905-906 ---- */

/* INVC100, called by STQ: its return in SPRTEX. CSMPREC integrates the
 * CSM's state to the time of ignition, SPRTETIG ("PRECISION INTEGRATION
 * R0,V0 TO R1,V1"): R(T1)/, V(T1)/ and T1, and P(T1) the central body
 * it left in X2. Not the earth (P(T1) not +0): alarm 612 (OCT612),
 * "R1,V1 NOT IN PROPER SPHERE OF INFLUENCE", through RTEALRM.
 *
 * INVC109: UR1/ and R(T1), the direction and size of R(T1)/; UV1/ the
 * direction of V(T1)/; CFPA, the "COSINE OF INITIAL FLIGHT PATH ANGLE",
 * the cosine of the angle between them. The orbit's normal N/ is UR1/ X
 * UV1/ (INVC115), or (0,0,1) if the orbit is near rectilinear (|CFPA| not
 * below EPC1RTE). INVC120: if its third component is negative, a
 * "RETROGRADE ORBIT": RETROFLG set and N/ reversed. INVC125: UH/, the
 * "UNIT HORIZONTAL VECTOR", N/ X UR1/; and back through SPRTEX. */
static const word invc100_words[] = {
    /* INVC100 */ ORDERS(STQ, DLOAD),
    ADDRESS(SPRTEX),
    ADDRESS_PLUS_ONE(SPRTETIG),
    STCALL(TDEC1),
    ADDRESS(CSMPREC), /* "PRECISION INTEGRATION  R0,V0 TO R1,V1" */
    ORDERS(VLOAD, SXA_2),
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS(P_T1),
    STOVL(R_T1_),
    ADDRESS_PLUS_ONE(VATT),
    STODL(V_T1_),
    ADDRESS_PLUS_ONE(TAT),
    STORE(T1),
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(P_T1),
    ADDRESS(INVC109),
    /* INVC107 */ ORDERS(SLOAD, GOTO),
    ADDRESS_PLUS_ONE(OCT612),
    ADDRESS(RTEALRM), /* "R1,V1 NOT IN PROPER SPHERE OF INFLUENCE" */
    /* INVC109 */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R_T1_),
    STODL(UR1_), /* "UR1/" */
    ADDRESS_PLUS_ONE_D(36),
    STOVL(R_T1), /* "R(T1)" */
    ADDRESS_PLUS_ONE(V_T1_),
    ORDER(UNIT),
    STORE(UV1_),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UR1_),
    STORE(CFPA), /* "CFPA" */
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(EPC1RTE),
    ORDERS(BMN, DLOAD),
    ADDRESS(INVC115), /* "NOT NEAR RECTILINEAR" */
    ADDRESS_PLUS_ONE(1RTEB2),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(ZERORTE),
    ORDERS(VDEF, PUSH), /* "N/ = (0,0,1)" */
    ORDER(GOTO),
    ADDRESS(INVC120),
    /* INVC115 */ ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(UR1_),
    ADDRESS_PLUS_ONE(UV1_),
    ORDER(PUSH), /* "N/ = UR X UV" */
    /* INVC120 */ ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, RETROFLG),
    ORDERS(PUSH, BPL),
    ADDRESS(INVC125),
    ORDERS(VLOAD, VCOMP), /* "RETROGRADE ORBIT" */
    ORDERS(PUSH, SET),
    SWITCH(SET, RETROFLG),
    /* INVC125 */ ORDER(VLOAD),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UR1_),
    STORE(UH_), /* "UH/" */
    ORDER(GOTO),
    ADDRESS(SPRTEX),
};

const struct interpretive_code INVC100_CODE = {032u, INVC100, COUNT(invc100_words), invc100_words};

/* ---- PREC100 -- pp. 907-913 ---- */

/* PREC100, called by STQ: its return in SPRTEX. Ten tries (10RTE, counted
 * down in NN1A) at the final radius RD, the conic's RCON.
 *
 * PREC120: RTENCK3 integrates the trajectory from R(T1)/ and V2(T1)/ to
 * T2, with the last change in time DT21PR as large as it goes ("DT21PR =
 * POSMAX") and fifteen tries (M15RTE, counted up in NN2) at the angle.
 * PREC125: PARAM, the conic through the state there: P/RPRE, R/APRE and
 * RPRE, its P/R, R/A and radius, and X(T2)PRE the cotangent of its flight
 * path angle (COGA). Near enough the angle the crew asked for, X(T2)
 * (within EPC4RTE): on at PREC175. */
static const word prec100_words[] = {
    /* PREC100 */ ORDERS(STQ, DLOAD),
    ADDRESS(SPRTEX),
    ADDRESS_PLUS_ONE(10RTE),
    STODL(NN1A),
    ADDRESS_PLUS_ONE(RCON),
    STORE(RD),
    /* PREC120 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(2RTEB1),
    STODL(DT21PR), /* "DT21PR = POSMAX" */
    ADDRESS_PLUS_ONE(M15RTE),
    STCALL(NN2),
    ADDRESS(RTENCK3),
    /* PREC125 */ ORDER(CALL),
    ADDRESS(PARAM),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(P),
    STODL(P_RPRE),
    ADDRESS_PLUS_ONE(R1A),
    STODL(R_APRE),
    ADDRESS_PLUS_ONE(R1),
    STODL(RPRE),
    ADDRESS_PLUS_ONE(COGA),
    ORDER(SL),
    SL_COUNT(5),
    STORE(X_T2PRE),
    ORDERS(DCOMP, DAD),
    ADDRESS_PLUS_ONE(X_T2),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(EPC4RTE),
    ORDERS(BOV, BMN),
    ADDRESS(PREC130),
    ADDRESS(PREC175),
};

/* PREC130, "DESIRED REENTRY ANGLE NOT ACHIEVED". Fifteen tries made (NN2
 * up to +0): alarm 605 (OCT605, PREC132), "TOO MANY ITERATIONS".
 *
 * PREC140, the "RADIUS AT WHICH THE DESIRED REENTRY ANGLE WILL BE
 * ACHIEVED", as BETA4 times RPRE: BETA4 is RD/RPRE once the ten tries at
 * the radius are spent (PREC162); else from BETA2=BETA1*P/R and
 * BETA3=1-BETA2*R/A, "BETA4=BETA2/(1-PHI2*SQRT(BETA3))" (PREC155), or the
 * inverse of R/APRE if BETA3 is negative (PREC160).
 *
 * PREC165: BETA12, 1 less BETA4, with its sign turned to X(T2)PRE's when
 * BETA4 is below 1. Within EPC6RTE of +0: on at PREC175. Else PREC170:
 * one more try (NN2), and TIMERAD (RVSW set, for the time alone) from
 * R(T2)/, and V(T2)/ with BETA12's sign, to that radius ("RF = NEW
 * RADIUS"), with SGNRDOT against BETA12's sign: DT21, its time with
 * BETA12's sign ("DT21=(PHI4)DT21"). BETA13=(DT21)/(DT21PR); a step that
 * would be too large, or reverse (BETA13 negative, PREC172), cut to
 * DT21PR (BETA14=1) or to .6 of it (M.6RTE). PREC174: RTENCK2 integrates
 * on by it, and again from PREC125. */
static const word prec130_words[] = {
    /* PREC130 */ ORDERS(DLOAD, BMN),
    ADDRESS_PLUS_ONE(NN2),
    ADDRESS(PREC140),
    /* PREC132 */ ORDERS(SLOAD, GOTO), /* "TOO MANY ITERATIONS" */
    ADDRESS_PLUS_ONE(OCT605),          /* "EXIT WITH ALARM" */
    ADDRESS(PRECX),
    /* PREC140 */ ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(NN1A),
    ADDRESS(PREC162),
    /* PREC150 */ ORDERS(DLOAD, SL2),
    ADDRESS_PLUS_ONE(P_RPRE),
    ORDERS(DMP, SL1), /* "BETA2=BETA1*P/R" */
    ADDRESS_PLUS_ONE(BETA1),
    ORDERS(PUSH, DLOAD),
    ADDRESS_PLUS_ONE(R_APRE),
    ORDERS(SL4, DMP),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(BDSU, BMN), /* "BETA3=1-BETA2*R/A" */
    ADDRESS_PLUS_ONE(1RTEB4),
    ADDRESS(PREC160),
    /* PREC155 */ ORDERS(SL2, SQRT),
    ORDERS(DMP, BDSU),
    ADDRESS_PLUS_ONE(PHI2),
    ADDRESS_PLUS_ONE(1RTEB3),
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(SR1, DDV), /* "BETA4=BETA2/(1-PHI2*SQRT(BETA3))" */
    ORDERS(SL_STAR, GOTO),
    SL_X1(-1),
    ADDRESS(PREC165),
    /* PREC160 */ ORDERS(DLOAD, NORM),
    ADDRESS_PLUS_ONE(R_APRE),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BDDV, SL_STAR),
    ADDRESS_PLUS_ONE(1RTEB1),
    SL_X1(-6),
    ORDER(GOTO),
    ADDRESS(PREC165),
    /* PREC162 */ ORDERS(DLOAD, NORM),
    ADDRESS_PLUS_ONE(RPRE),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BDDV, SL_STAR), /* "BETA4=RD/RPRE" */
    ADDRESS_PLUS_ONE(RD),
    SL_X1(-1),
    /* PREC165 */ ORDERS(SETPD, PUSH),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DSU, DCOMP),
    ADDRESS_PLUS_ONE(1RTEB1),
    STORE(BETA12),
    ORDERS(BMN, DLOAD),
    ADDRESS(PREC168),
    ADDRESS_PLUS_ONE(X_T2PRE),
    ORDERS(BMN, DLOAD),
    ADDRESS(PREC167),
    ADDRESS_PLUS_ONE(BETA12),
    ORDER(DCOMP),
    STORE(BETA12),
    /* PREC167 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(BETA12),
    /* PREC168 */ ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(EPC6RTE),
    ORDERS(BMN, DLOAD),
    ADDRESS(PREC175),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(RPRE),
    ORDER(PUSH), /* "RF = NEW RADIUS" */
    /* PREC170 */ ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(NN2),
    ADDRESS_PLUS_ONE(1RTEB28),
    STORE(NN2),
    ORDERS(VLOAD, SET),
    ADDRESS_PLUS_ONE(R_T2_),
    SWITCH(SET, RVSW),
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(V_T2_),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE(BETA12),
    STODL(VVEC),
    ADDRESS_PLUS_ONE(1RTEB1),
    ORDERS(SIGN, DCOMP),
    ADDRESS_PLUS_ONE(BETA12),
    ORDERS(LXA_2, DLOAD),
    ADDRESS(MPAC),
    ORDERS(LXC_1, SXA_2),
    ADDRESS(CONICX1),
    ADDRESS(SGNRDOT),
    STCALL(RDESIRED), /* "COMPUTED DT12 (CORRECTION TO TIME OF" */
    ADDRESS(TIMERAD), /* "NEW RADIUS)" */
    ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE(T),
    ADDRESS_PLUS_ONE(BETA12),
    ORDERS(PDDL, NORM), /* "DT21=(PHI4)DT21" */
    ADDRESS_PLUS_ONE(DT21PR),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BDDV, SL_STAR),
    ADDRESS_PLUS_ONE_D(0),
    SL_X1(-3),
    ORDERS(PUSH, BMN), /* "BETA13=(DT21)/(DT21PR)" */
    ADDRESS(PREC172),
    ORDERS(DLOAD, PDDL), /* "BETA14=1" */
    ADDRESS_PLUS_ONE(2RTEB1),
    ORDER(GOTO),
    ADDRESS(PREC173),
    /* PREC172 */ ORDERS(DLOAD, PDDL), /* "BETA14=.6" */
    ADDRESS_PLUS_ONE(M_6RTE),
    /* PREC173 */ ORDERS(DDV, DSU),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE(1RTEB3),
    ORDERS(BMN, DLOAD),
    ADDRESS(PREC174),
    ORDER(DMP),
    ADDRESS_PLUS_ONE(DT21PR),
    STORE_D(0), /* "DT21=(BETA14)DT21PR" */
    /* PREC174 */ ORDERS(DLOAD, PUSH),
    ADDRESS_PLUS_ONE_D(0),
    STCALL(DT21PR),
    ADDRESS(RTENCK2),
    ORDER(GOTO),
    ADDRESS(PREC125),
};

/* PREC175: "RPRE-RD = RERR". Within EPC7RTE: on at PREC220, "DESIRED
 * RADIUS ACHIEVED". Else, the ten tries spent (NN1A +0), alarm 605
 * (PREC132); a new final radius for the conic. On the first try (PREC207,
 * NN1A still 10RTE) "RD**2/RPRE=RCON", and DRCON the change; on the
 * others (PREC205) by the secant method, the last change over the change
 * it made to RPRE ("DRCON/(RPRE'-RPRE)=S"), S held to -4 when it is above
 * +0 or below -4: "DRCON=S(RERR)", "RCON+DRCON=RCON". PREC210: RPRE'
 * RPRE, one try fewer (NN1A), and V2T100 the conic for the new RCON: if
 * it found one (+0, BHIZ), again from PREC120; else to PRECX with its
 * alarm's code. */
static const word prec175_words[] = {
    /* PREC175 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(RPRE),
    ADDRESS_PLUS_ONE(RD),
    ORDERS(PUSH, ABS), /* "RPRE-RD = RERR" */
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(EPC7RTE),
    ADDRESS(PREC220),
    ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(NN1A),
    ADDRESS(PREC132), /* "TOO MANY ITERATIONS" */
    ORDERS(DSU, BZE),
    ADDRESS_PLUS_ONE(10RTE),
    ADDRESS(PREC207),
    /* PREC205 */ ORDERS(DLOAD, DSU), /* "NOT FIRST PASS OF ITERATION" */
    ADDRESS_PLUS_ONE(RPRE_PRIME),
    ADDRESS_PLUS_ONE(RPRE), /* "RPRE'-RPRE" */
    ORDERS(NORM, BDDV),
    ADDRESS_PLUS_ONE(X2),
    ADDRESS_PLUS_ONE(DRCON),
    ORDERS(SL_STAR, PUSH), /* "DRCON/(RPRE'-RPRE)=S" */
    SL_X2(-2),
    ORDERS(DAD, BOV), /* "S GR +4 OR LS -4" */
    ADDRESS_PLUS_ONE(1RTEB1),
    ADDRESS(PREC205M),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(1RTEB1),
    ORDER(BMN),
    ADDRESS(PREC206),
    /* PREC205M */ ORDERS(DLOAD, DCOMP), /* "S GR 0 OR LS -4" */
    ADDRESS_PLUS_ONE(2RTEB1),
    ORDER(PDDL), /* "S=-4" */
    /* PREC206 */ ORDERS(DLOAD, DMP),
    ORDER(SL2),
    STORE(DRCON), /* "DRCON=S(RERR)" */
    ORDER(DAD),
    ADDRESS_PLUS_ONE(RCON),
    STORE(RCON), /* "RCON+DRCON=RCON" */
    ORDER(GOTO),
    ADDRESS(PREC210),
    /* PREC207 */ ORDERS(DLOAD, DSQ), /* "FIRST PASS OF ITERATION" */
    ADDRESS_PLUS_ONE(RD),
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE(RPRE),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(XSU_1, BDDV),
    ADDRESS(X2),
    ORDER(SR_STAR),
    SR_X1(-1),
    STORE(RCON), /* "RD**2/RPRE=RCON" */
    ORDER(DSU),
    ADDRESS_PLUS_ONE(RD),
    STORE(DRCON),               /* "RCON-RD=DRCON" */
    /* PREC210 */ ORDER(DLOAD), /* "PREPARE FOR NEXT ITERATION" */
    ADDRESS_PLUS_ONE(RPRE),
    STODL(RPRE_PRIME),
    ADDRESS_PLUS_ONE(NN1A),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(1RTEB28),
    STCALL(NN1A),
    ADDRESS(V2T100),
    ORDERS(BHIZ, GOTO),
    ADDRESS(PREC120),
    ADDRESS(PRECX),
    /* PREC225, "DESIRED FINAL ANGLE HAS BEEN REACHED": MPAC +0 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZERORTE),
    /* PRECX: back through SPRTEX */
    ORDER(GOTO),
    ADDRESS(SPRTEX),
};

const struct interpretive_code PREC100_CODE = {032u, PREC100, COUNT(prec100_words), prec100_words};
const struct interpretive_code PREC130_CODE = {032u, PREC100 + COUNT(prec100_words),
                                               COUNT(prec130_words), prec130_words};
const struct interpretive_code PREC175_CODE = {
    032u, PREC100 + COUNT(prec100_words) + COUNT(prec130_words), COUNT(prec175_words),
    prec175_words};

/* PREC220, in bank 16 (RTE2): the angle checked again. Further than
 * EPC8RTE (.002) from X(T2): alarm 613 (OCT613), "IF REENTRY ANGLE OUT OF
 * LIMITS"; else on at PREC225, MPAC +0. (EPC8RTE and OCT613 follow it in
 * the bank.) */
static const word prec220_words[] = {
    /* PREC220 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(X_T2),
    ADDRESS_PLUS_ONE(X_T2PRE),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(EPC8RTE),
    ORDERS(BMN, SLOAD),
    ADDRESS(PREC225),
    ADDRESS_PLUS_ONE(OCT613),
    ORDER(GOTO),
    ADDRESS(PRECX), /* "IF REENTRY ANGLE OUT OF LIMITS" */
};

const struct interpretive_code PREC220_CODE = {016u, PREC220, COUNT(prec220_words), prec220_words};

#endif
