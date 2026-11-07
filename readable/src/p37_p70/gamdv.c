/* SPDX-License-Identifier: MIT */
/* P37_P70.agc of the CM, part 5 (Comanche055 pp. 924-930, bank 32): the
 * "X(T1)-DV ITERATOR SUBROUTINE" (GAMDV10), which "COMPUTES A POST
 * IMPULSE VELOCITY VECTOR (V2(T1)) WHICH REQUIRES A MINIMUM DV" (or the
 * change in speed asked for), the "DV CALCULATION SUBROUTINE" (DVCALC)
 * and the "SUBROUTINE TO COMPUTE BOUNDS ON INDEPENDENT VARIABLE X(T1)"
 * (XT1LIM). The file's constants follow them (pp. 931-933, banks 36, 32
 * and 16); they are data, not code. */
#include "p37_p70.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- GAMDV10 -- pp. 924-927 ---- */

/* GAMDV10, called by STQ: its return in 31D, and the push-down list from
 * 18D. X(T1) is the cotangent of the flight path angle after the burn,
 * searched for between X(T1)MIN (10D) and X(T1)MAX (14D) in steps DX(T1)
 * (16D) of at most DX(T1)MAX (12D). DVCALC, the change in speed for the
 * X(T1) it starts at. With the bounds within EPC9RTE ("BOUNDS CLOSE
 * TOGETHER"), done; with BETA8, the gap between them, less than
 * DX(T1)MAX, the step is half of BETA8, with the step's own sign
 * (GAMDV15).
 *
 * GAMDV20: 144 tries (M144RTE, counted up in NN2; GAMDV25); then alarm
 * 605 (OCT605), out at V2T1X, V2T100's own exit. GAMDV30: the last X(T1)
 * and DV kept (X(T1), and DV,), one step taken, and DVCALC again. For the
 * least fuel (F2RTE clear) the step goes on while DV falls, and turns back
 * at half its size when it rises (GAMDV32). For the least time (GAMDV35,
 * F2RTE set) "DVD-DV=DVERR": done within EPC10RTE of RTEDVD; else (GAMDV45,
 * the overflow indicator cleared, "ASSURE OVFIND IS 0") the secant step
 * "(X(T1)-X(T1),)DVERR/(DV-DV,)=DX(T1)", no larger than DX(T1)MAX with
 * its own sign (GAMDV47).
 *
 * GAMDV50, the "CHECK TO KEEP INDEPENDENT VARIABLE IN BOUNDS": if
 * "X(T1)+1.1DX(T1)=BETA9" would pass X(T1)MAX or X(T1)MIN, the step is
 * half the way to it (GAMDV60 for X(T1)MIN). GAMDV65: done when the step
 * is within EPC9RTE of +0, else again from GAMDV25. GAMDVX: back through
 * 31D. */
static const word gamdv10_words[] = {
    /* GAMDV10 */ ORDER(STQ),
    ADDRESS_D(31),
    ORDERS(SETPD, CALL),
    ADDRESS_PLUS_ONE_D(18),
    ADDRESS(DVCALC),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(14),
    ADDRESS_PLUS_ONE_D(10),
    ORDER(BOV),
    ADDRESS(GAMDV20),
    ORDERS(PUSH, DSU), /* "X(T1)MAX-X(T1)MIN=BETA8" */
    ADDRESS_PLUS_ONE(EPC9RTE),
    ORDERS(BMN, DLOAD),
    ADDRESS(GAMDVX), /* "BOUNDS CLOSE TOGETHER" */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(DSU, BMN), /* "BETA8-DX(T1)MAX" */
    ADDRESS_PLUS_ONE_D(12),
    ADDRESS(GAMDV15),
    ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(18),
    ADDRESS(GAMDV20),
    /* GAMDV15 */ ORDER(DLOAD),
    ORDERS(SIGN, SR1),
    ADDRESS_PLUS_ONE_D(16),
    STORE_D(16), /* "BETA8(SIGNDX(T1))/2=DX(T1)" */
    /* GAMDV20 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(M144RTE),
    STORE(NN2),
    /* GAMDV25 */ ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(NN2),
    ADDRESS_PLUS_ONE(1RTEB28),
    ORDERS(BMN, SLOAD),
    ADDRESS(GAMDV30),
    ADDRESS_PLUS_ONE(OCT605),
    ORDER(GOTO),
    ADDRESS(V2T1X),
    /* GAMDV30 */ STORE(NN2), /* "NN2=NN2+1" */
    ORDERS(DLOAD, PDDL),      /* "X(T1)=X(T1)," */
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS_PLUS_ONE(DV),
    ORDERS(PDDL, DAD), /* "DV=DV," */
    ADDRESS_PLUS_ONE(X_T1),
    ADDRESS_PLUS_ONE_D(16),
    STCALL(X_T1), /* "X(T1)+DX(T1)=X(T1)" */
    ADDRESS(DVCALC),
    ORDERS(BON, DLOAD),
    SWITCH(BON, F2RTE),
    ADDRESS(GAMDV35),
    ADDRESS_PLUS_ONE(DV),
    ORDERS(DSU, BMN), /* "CONTINUE IF FUEL CRITICAL MODE" */
    ADDRESS_PLUS_ONE_D(20),
    ADDRESS(GAMDV33),
    /* GAMDV32 */ ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE_D(16),
    ORDER(SR1),
    STORE_D(16),
    /* GAMDV33 */ ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(18),
    ADDRESS(GAMDV50),
    /* GAMDV35 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(RTEDVD),
    ADDRESS_PLUS_ONE(DV),
    ORDERS(PDDL, PUSH),               /* "DVD-DV=DVERR" */
    /* GAMDV40 */ ORDERS(DLOAD, ABS), /* "DV," */
    ADDRESS_PLUS_ONE_D(20),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(EPC10RTE),
    ADDRESS(GAMDVX),
    /* GAMDV45 */ ORDERS(BOVB, DLOAD),
    ADDRESS(TCDANZIG), /* "ASSURE OVFIND IS 0" */
    ORDERS(BDSU, NORM),
    ADDRESS_PLUS_ONE(DV),
    ADDRESS_PLUS_ONE(X2),
    ORDER(PDDL),       /* "DV-DV," */
    ORDERS(NORM, SR1), /* "DVERR" */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DDV, PDDL), /* "DVERR/ DV - DV" */
    ORDERS(BDSU, DMP),
    ADDRESS_PLUS_ONE(X_T1),
    ORDER(XSU_1),
    ADDRESS(X2),
    STORE_D(16), /* "PRESERVE SIGN IF OVERFLOW" */
    ORDERS(SR_STAR, BOV),
    SR_X1(-1),
    ADDRESS(GAMDV47),
    STORE_D(16), /* "(X(T1)-X(T1),)DVERR/(DV-DV,)=DX(T1)" */
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(BMN),
    ADDRESS(GAMDV50),
    /* GAMDV47 */ ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE_D(12),
    ADDRESS_PLUS_ONE_D(16),
    STORE_D(16), /* "DX(T1)MAX(SIGNDX(T1))=DX(T1)" */
    /* GAMDV50 */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(16),
    ADDRESS_PLUS_ONE(1_1RTEB1),
    ORDERS(SL1, DAD),
    ADDRESS_PLUS_ONE(X_T1),
    STORE_D(24), /* "X(T1)+1.1DX(T1)=BETA9" */
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE_D(14),
    ADDRESS(GAMDV55),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(14),
    ADDRESS_PLUS_ONE(X_T1),
    ORDER(SR1),
    STCALL_D(16), /* "(X(T1)MAX-X(T1))/2=DX(T1)" */
    ADDRESS(GAMDV65),
    /* GAMDV55 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(24),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(BMN, GOTO),
    ADDRESS(GAMDV60),
    ADDRESS(GAMDV65),
    /* GAMDV60 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(10),
    ADDRESS_PLUS_ONE(X_T1),
    ORDER(SR1),
    STORE_D(16), /* "(X(T1)MIN-X(T1))/2=DX(T1)" */
    /* GAMDV65 */ ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE_D(16),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(EPC9RTE),
    ADDRESS(GAMDVX),
    ORDER(GOTO),
    ADDRESS(GAMDV25),
    /* GAMDVX */ ORDER(GOTO),
    ADDRESS_D(31),
};

const struct interpretive_code GAMDV10_CODE = {032u, GAMDV10, COUNT(gamdv10_words), gamdv10_words};

/* ---- DVCALC -- pp. 928-929 ---- */

/* DVCALC, called by STQ: its return in 32D. The semi-latus rectum
 * "THETA2/(THETA1-X(T1)**2)=PCON" (THETA1 at 02D, THETA2 at 05D, from
 * V2T100); "THETA3*PCON**.5" (28D); then the velocity after the burn,
 * V2(T1)/, THETA3 times the root of PCON times X(T1)*UR1/ + UH/, and DV,
 * "ABVAL(V2(T1)/-V1(T)/)". "PUSHLOC IS RESTORED TO ITS ENTRANCE VALUE
 * UPON EXITING DVCALC", which goes back through 32D. */
static const word dvcalc_words[] = {
    /* DVCALC */ ORDERS(STQ, DLOAD),
    ADDRESS_D(32),
    ADDRESS_PLUS_ONE(X_T1),
    ORDERS(DSQ, SR),
    SR_COUNT(7),
    ORDERS(DCOMP, TAD),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(NORM, PUSH),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(TLOAD, NORM),
    ADDRESS_PLUS_ONE_D(5),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(RTB, SR1),
    ADDRESS(DPMODE),
    ORDERS(XSU_2, DDV),
    ADDRESS(X1),
    ORDER(SR_STAR),
    SR_X2(6),
    STORE(PCON), /* "THETA2/(THETA1-X(T1)**2)=PCON" */
    ORDERS(SQRT, DMP),
    ADDRESS_PLUS_ONE_D(8),
    ORDER(NORM),
    ADDRESS_PLUS_ONE(X1),
    STODL_D(28), /* "THETA3*PCON**.5" */
    ADDRESS_PLUS_ONE(X_T1),
    ORDERS(NORM, VXSC),
    ADDRESS_PLUS_ONE(X2),
    ADDRESS_PLUS_ONE(UR1_), /* "X(T1)*UR1/" */
    ORDERS(XAD_2, VXSC),
    ADDRESS(X1),
    ADDRESS_PLUS_ONE_D(28),
    ORDERS(VSR_STAR, PDVL), /* "THETA3(PCON**.5)X(T1)*UR1/" */
    SR_X2(-9),
    ADDRESS_PLUS_ONE(UH_),
    ORDERS(VXSC, VSR_STAR), /* "THETA3(PCON**.5)UH/" */
    ADDRESS_PLUS_ONE_D(28),
    SR_X1(-4),
    ORDERS(VAD, STADR),
    AFTER_STADR(STORE(V2_T1_)), /* "V2(T1)/" */
    ORDERS(VSU, ABVAL),
    ADDRESS_PLUS_ONE(V_T1_),
    STORE(DV), /* "ABVAL(V2(T1)/-V1(T)/)=DV" */
    ORDER(GOTO),
    ADDRESS_D(32),
};

const struct interpretive_code DVCALC_CODE = {032u, DVCALC, COUNT(dvcalc_words), dvcalc_words};

/* ---- XT1LIM -- p. 930 ---- */

/* XT1LIM, called by STQ with a major axis MA on the push-down list: its
 * return in 20D. "BETA10=BETA5(MA-RT)/(MA-RC)-1", with RT the radius at
 * ignition, R(T1), and RC the entry radius, RCON; the bound on X(T1) is
 * its square root ("X(T1)=SQRT(BETA10)"), POSMAX's (2RTEB1) if it
 * overflows ("BETA10=POSMAX IF OVERFLOW"), and +0 if BETA10 is negative
 * (XT1LIM5); back through 20D. "PUSHLOC IS RESTORED TO ITS ENTRANCE VALUE
 * UPON EXITING XT1LIM". */
static const word xt1lim_words[] = {
    /* XT1LIM */ ORDERS(STQ, DLOAD),
    ADDRESS_D(20),
    ADDRESS_PLUS_ONE(RCON),
    ORDERS(SR1, BDSU),
    ORDERS(NORM, PDDL), /* "MA-RCON" */
    ADDRESS_PLUS_ONE(X2),
    ORDERS(PDDL, SR1),
    ADDRESS_PLUS_ONE(R_T1),
    ORDERS(BDSU, DDV),
    ORDERS(SL_STAR, DMP),
    SL_X2(-3),
    ADDRESS_PLUS_ONE_D(28),
    ORDERS(SL_STAR, DSU), /* "BETA10=BETA5(MA-RT)/(MA-RC)-1" */
    SL_X1(-6),
    ADDRESS_PLUS_ONE_AT(1RTEB25, 1),
    ORDERS(SL1, BOV),
    ADDRESS(XT1LIM2),
    ORDERS(BMN, GOTO),
    ADDRESS(XT1LIM5),
    ADDRESS(XT1LIM3),
    /* XT1LIM2 */ ORDER(DLOAD), /* "BETA10=POSMAX IF OVERFLOW" */
    ADDRESS_PLUS_ONE(2RTEB1),
    /* XT1LIM3 */ ORDERS(SQRT, GOTO), /* "X(T1)=SQRT(BETA10)" */
    ADDRESS(XT1LIMX),
    /* XT1LIM5 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZERORTE),
    /* XT1LIMX */ ORDER(GOTO),
    ADDRESS_D(20),
};

const struct interpretive_code XT1LIM_CODE = {032u, XT1LIM, COUNT(xt1lim_words), xt1lim_words};

#endif
