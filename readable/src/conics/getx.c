/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, GETX (LM pp. 1189-1192, CM pp. 1292-1295): the
 * universal variable X for TIME-THETA's angle, in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs, and
 * the basic code around its TC POLY.
 *
 * In: P in MPAC ("ASSUMES P (+4) IN MPAC"), with PARAM's COGA and R1A and
 * GEOM's SNTH, CSTH and R1. W comes from them and goes through three steps
 * of a continued fraction (WLOOP); a series in R1A over W squared (POLY)
 * gives X over the root of R1 from 1/W. Near 360 degrees, when W
 * overflows, the inverse sequence (INVRSEQN, 1/WLOOP) gives 1/W instead,
 * and TRUE360X takes the X of the other way round, 2PI/SQRT(R1A) less it.
 * Out: X, XI (R1A X squared over R1), KEPC1 and KEPC2 for DELTIME, X
 * squared at 0D; or, with no solution (R1A negative where a root is
 * wanted: "CLOSURE THROUGH INFINITY"), INFINFLG set. */
#include "conics.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

static const word getx_words[] = {
    /* GETX -- LM p. 1189 (CM p. 1292): X2 3, the loop's count, with S2 1
     * its step; 360SW cleared. The root of P pushed, and the first W from
     * CSTH, SNTH and COGA: an overflow is 360CHECK. */
    /* GETX */ ORDERS(AXT_2, SSP),
    NUMBER(3),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(1),
    ORDER(CLEAR),
    SWITCH(CLEAR, 360SW),
    ORDERS(SQRT, PDDL), /* "0D=SQRT(P) PL AT 2" */
    ADDRESS_PLUS_ONE(CSTH),
    ORDERS(SR1, BDSU),
    ADDRESS_PLUS_ONE(D1_4),
    ORDERS(PDDL, SRR), /* "PL AT 4D" */
    ADDRESS_PLUS_ONE(SNTH),
    SRR_COUNT(6),
    ORDER(DDV), /* "PL AT 2" */
    ORDER(BOV),
    ADDRESS(360CHECK),
    ORDERS(DSU, DMP),
    ADDRESS_PLUS_ONE(COGA), /* "PL AT 0" */
    ORDERS(SL2R, BOV),
    ADDRESS(360CHECK),
    /* WLOOP: W plus the root of W squared plus R1A, three times by X2.
     * A negative sum under the root is INFINITY; an overflow, RESETX2.
     * Then 1/W. */
    /* WLOOP */ ORDERS(PUSH, DSQ), /* "0D=W (+5) PL AT 2" */
    ORDERS(TLOAD, PDDL),           /* "2D=WSQ (+10) PL AT 5" */
    ADDRESS_PLUS_ONE(MPAC),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(SR4, TAD), /* "PL AT 2" */
    ORDERS(BMN, SQRT),
    ADDRESS(INFINITY),
    ORDERS(ROUND, DAD), /* "PL AT 0D" */
    ORDERS(BOV, TIX_2),
    ADDRESS(RESETX2),
    ADDRESS(WLOOP),
    ORDERS(BDDV, BOV),
    ADDRESS_PLUS_ONE(D1_128),
    ADDRESS(INFINITY),
    /* POLYCOEF: 1/W pushed (negative, INFINITY), and R1A/W squared, the
     * series' argument, normalized by X1; to the basic code for the
     * series. */
    /* POLYCOEF */ ORDERS(BMN, PUSH), /* "0D=1/W (+2) OR 16/W (+6) PL AT 2" */
    ADDRESS(INFINITY),
    ORDER(DSQ),
    ORDERS(NORM, DMP),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(SRR_STAR, EXIT),
    SRR_X1(-10),
};

const struct interpretive_code GETX_CODE = {I_GETX >> 10, K_GETX, COUNT(getx_words), getx_words};

/* After the EXIT, TC POLY: the series, DEC 5 and seven 2DECs, from .5 on;
 * POLY returns past them, to POLYCOEF +030. */
void getx_after_exit(void)
{
    tc_to(POLYCOEF + 010u, POLY);
}

/* POLYCOEF +030: the series' value in MPAC, and the interpreter again
 * (GETX_SERIES_CODE). */
void getx_after_poly(void)
{
    tc_at(POLYCOEF + 030u);
    intpret();
}

static const word getx_series_words[] = {
    /* POLYCOEF +031: the series times 1/W, pushed: X over the root of R1
     * (or, with 360SW set, TRUE360X first). */
    ORDERS(DMP, SL1R), /* "PL AT 0D" */
    ORDERS(PUSH, BON),
    SWITCH(BON, 360SW),
    ADDRESS(TRUE360X),
    /* XCOMMON -- LM p. 1190 (CM p. 1293): XI, R1A times it squared
     * ("XI (+6)"); X, it times the root of R1 ("X (+17 OR +16)"). X squared
     * pushed for DELTIME; KEPC1, the root of P R1 times COGA, and KEPC2,
     * 1 - R1A; INFINFLG cleared, and RVQ returns. */
    /* XCOMMON */ ORDERS(DSQ, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE(R1A),
    SRR_X1(-12),
    STODL(XI),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(SR1, SQRT),
    ORDERS(ROUND, DMP),
    ORDER(SL4R), /* "PL AT 0" */
    STORE(X),
    ORDERS(DSQ, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, DMP), /* "0D=XSQ (+34 OR +32 -N1) PL AT 2" */
    ADDRESS_PLUS_ONE(P),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(SL3, SQRT),
    ORDERS(DMP, SL3R),
    ADDRESS_PLUS_ONE(COGA),
    STODL(KEPC1),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(BDSU, CLEAR),
    ADDRESS_PLUS_ONE(D1_64),
    SWITCH(CLEAR, INFINFLG),
    STORE(KEPC2),
    ORDER(RVQ),
    /* RESETX2 -- LM p. 1191 (CM p. 1294): WLOOP overflowed; X2 3 again,
     * for 1/WLOOP. */
    /* RESETX2 */ ORDER(AXT_2),
    NUMBER(3),
    /* 360CHECK: the push-down list at 0D; with MPAC negative, 360SW set
     * too. */
    /* 360CHECK */ ORDERS(SETPD, BPL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(INVRSEQN),
    ORDER(SET),
    SWITCH(SET, 360SW),
    /* INVRSEQN: "NOTE: NEAR 360 CASE TREATED DIFFERENTLY". 1/W from the
     * root of P, SNTH, COGA and CSTH, pushed, and its square to 34D; G
     * starts at 1/16. */
    /* INVRSEQN */ ORDERS(DLOAD, SQRT),
    ADDRESS_PLUS_ONE(P),
    ORDERS(PDDL, DMP), /* "0D=SQRT(P) (+2) PL AT 2" */
    ADDRESS_PLUS_ONE(SNTH),
    ADDRESS_PLUS_ONE(COGA),
    ORDERS(SL1, PDDL), /* "2D=SNTH COGA (+5) PL AT 4" */
    ADDRESS_PLUS_ONE(CSTH),
    ORDERS(SR4, DAD),
    ADDRESS_PLUS_ONE(D1_32),
    ORDERS(DSU, DMP), /* "PL AT 2,0" */
    ORDERS(NORM, BDDV),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(SNTH),
    ORDERS(SLR_STAR, ABS),
    SLR_X1(-5),
    ORDERS(PUSH, DSQ), /* "0D=1/W (-1) PL AT 2" */
    STODL_D(34),
    ADDRESS_PLUS_ONE(D1_16),
    /* 1/WLOOP: G plus the root of G squared plus R1A/W squared, three
     * times by X2; RTB TPMODE makes G squared triple precision for the
     * push-down list. A negative sum under the root is INFINITY. Then 1/W
     * over G, shifted, is the 1/W for POLYCOEF. */
    /* 1/WLOOP */ ORDERS(PUSH, DSQ), /* "2D=G (+4) PL AT 4" */
    ORDERS(RTB, PDDL),               /* "PL AT 7" */
    ADDRESS(TPMODE),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(DMP, SR4),
    ADDRESS_PLUS_ONE_D(34),
    ORDER(TAD), /* "PL AT 4" */
    ORDERS(BMN, SQRT),
    ADDRESS(INFINITY),
    ORDER(DAD), /* "PL AT 2" */
    ORDERS(TIX_2, NORM),
    ADDRESS(1_WLOOP),
    ADDRESS_PLUS_ONE(X1),
    ORDER(BDDV),
    ORDERS(SLR_STAR, GOTO), /* "PL AT 0" */
    SLR_X1(-7),
    ADDRESS(POLYCOEF),
    /* TRUE360X: the X of the whole ellipse, 2PI over the root of R1A
     * (none if R1A is negative: INFINITY), less X, and to XCOMMON. */
    /* TRUE360X */ ORDERS(DLOAD, BMN),
    ADDRESS_PLUS_ONE(R1A),
    ADDRESS(INFINITY),
    ORDERS(SQRT, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BDDV, SL_STAR),
    ADDRESS_PLUS_ONE(2PISC),
    SL_X1(-3),
    ORDERS(DSU, PUSH), /* "0D=2PI/SQRT(R1A) -X PL AT 0,2" */
    ORDER(GOTO),
    ADDRESS(XCOMMON),
    /* INFINITY: "NO SOLUTION EXISTS SINCE CLOSURE THROUGH INFINITY IS
     * REQUIRED". SETPD 0, and BOV OVFLCLR clears the overflow indicator. */
    /* INFINITY */ ORDERS(SETPD, BOV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(OVFLCLR),
    /* OVFLCLR -- LM p. 1192 (CM p. 1295): INFINFLG set, and RVQ. */
    /* OVFLCLR */ ORDERS(SET, RVQ),
    SWITCH(SET, INFINFLG),
};

const struct interpretive_code GETX_SERIES_CODE = {I_GETX >> 10, POLYCOEF + 031u,
                                                   COUNT(getx_series_words), getx_series_words};
