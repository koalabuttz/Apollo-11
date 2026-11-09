/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, KEPPREP and FBR3 (LM pp. 1227-1229, CM pp.
 * 1334-1336), in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs.
 *
 * KEPPREP moves the conic (RCV, VCV) from its time TC to TAU.: it makes
 * the first guess of Kepler's universal variable for the time TAU. - TC,
 * a short series in the conic's position and velocity, starting from the
 * last one (XKEP), and KEPLERN (CONIC_SUBROUTINES) solves Kepler's
 * equation from it. PBODY (0 the earth, 2 the moon) picks the central
 * body's MU (MUEARTH,2), and X1 KEPLERN's central body (-10 the moon's,
 * -2 the earth's, by MOONFLAG). The scale factors of the numbers
 * are in the listing's comments. FBR3 moves the conic's time and the
 * state vector's (TET) on by a step (DT/2, scaled), and KEPPREP. */
#include "orbital_integration.h"
#include "../agc/names.h"

static const word kepprep_words[] = {
    /* KEPPREP -- LM p. 1227 (CM p. 1334): X2 the central body; SQRT(MU),
     * the unit of RCV, and its length normalized (the shift in X1). */
    /* KEPPREP */ ORDERS(LXA_2, SETPD),
    ADDRESS(PBODY),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DLOAD_STAR, SQRT), /* "SQRT(MU) (+18 OR +15)" */
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, 0),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(RCV),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE_D(36),
    ADDRESS_PLUS_ONE(X1),
    /* The flight path: the unit position's dot VCV, over SQRT(MU); and the
     * time to go, TAU. - TC, normalized (the shift in S1). */
    ORDER(PDVL),
    ORDERS(DOT, PDDL),
    ADDRESS_PLUS_ONE(VCV),
    ADDRESS_PLUS_ONE(TAU_), /* "(+28)" */
    ORDERS(DSU, NORM),
    ADDRESS_PLUS_ONE(TC),
    ADDRESS_PLUS_ONE(S1),
    ORDER(SR1),
    ORDERS(DDV, PDDL),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DMP, PUSH), /* "FS (+6 +N1-N2)" */
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DSQ, PDDL),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DSQ, PDDL_STAR),
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, 0),
    ORDERS(SR3, SR4),
    ORDERS(PDVL, VSQ), /* "PREALIGN MU (+43 OR +37)" */
    ADDRESS_PLUS_ONE(VCV),
    ORDERS(DMP, BDSU),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(DDV, DMP),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE(DP2_3),
    SL_X1(-3),
    ORDERS(XSU_1, DAD), /* "2(FS)SQ - ETCETERA" */
    ADDRESS(S1),        /* "X1 = N2-N1" */
    ORDERS(SL_STAR, DSU),
    SL_X1(8),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(SL_STAR, SL_STAR),
    SL_X1(8),
    SL_X1(0),
    ORDERS(DAD, PDDL),
    ADDRESS_PLUS_ONE(XKEP),
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE_D(0),
    SL_X1(1),
    /* LM p. 1228 (CM p. 1335): an overflow reset (through TCDANZIG); the
     * guess into XKEPNEW, the return into KEPRTN, and KEPLERN with X1 the
     * central body's. */
    ORDERS(BOVB, DAD),
    ADDRESS(TCDANZIG),
    ORDER(STADR),
    AFTER_STADR(STORE(XKEPNEW)),
    ORDERS(STQ, AXC_1),
    ADDRESS(KEPRTN),
    NUMBER(10),
    ORDERS(BON, AXC_1),
    SWITCH(BON, MOONFLAG),
    ADDRESS(KEPLERN),
    NUMBER(2),
    ORDER(GOTO),
    ADDRESS(KEPLERN),
    /* FBR3 -- LM p. 1229 (CM p. 1336): X1 DIFEQCNT, stepped once by TIX,1
     * (S1 -13), the branch to the next word; the step DT/2 SR 9D, rounded;
     * TAU. = TC plus the step, TET plus the step, and KEPPREP. */
    /* FBR3 */ ORDERS(LXA_1, SSP),
    ADDRESS(DIFEQCNT),
    ADDRESS_PLUS_ONE(S1),
    MINUS_NUMBER(13),
    ORDERS(DLOAD, SR),
    ADDRESS_PLUS_ONE(DT_2),
    SR_COUNT(9),
    ORDERS(TIX_1, ROUND),
    ADDRESS_AT(FBR3, 9), /* (+1) */
    ORDERS(PUSH, DAD),
    ADDRESS_PLUS_ONE(TC),
    STODL(TAU_),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(TET),
    STCALL(TET),
    ADDRESS(KEPPREP),
};

const struct interpretive_code KEPPREP_CODE = {CODE_BANK(I_KEPPREP), CODE_Z(I_KEPPREP),
                                               COUNT(kepprep_words), kepprep_words};
