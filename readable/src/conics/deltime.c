/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, DELTIME (LM pp. 1180-1181, CM pp. 1283-1284):
 * Kepler's equation in the universal variable, the time T an X gives.
 *
 * In: MPAC is XI, ALPHA X^2 ("MPAC=XI (+6), 0D=XSQ"), and 0D is X^2; X,
 * R1, KEPC1 and KEPC2 as KEPLER left them. Two series in XI by POLY, the
 * basic code's polynomial routine: S(XI) and then C(XI), the universal
 * variable's two functions. Then
 *
 *     T = (C1 X^2 C(XI) + X (C2 X^2 S(XI) + R1)) / ROOTMU,
 *
 * with XSQC(XI) (X^2 C(XI)) kept for KEPLER's next guess. The scaling
 * comments are the listing's: "(+49 OR +46)" is the earth's scale or the
 * moon's. The interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs, and
 * the basic code around the two TC POLYs. */
#include "conics.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* DELTIME: to the basic code, for the first series. */
static const word deltime_words[] = {
    /* DELTIME */ ORDER(EXIT), /* "MPAC=XI (+6), 0D=XSQ (+34 OR +32 -N1)" */
};

const struct interpretive_code DELTIME_CODE = {I_DELTIME >> 10, DELTIME, COUNT(deltime_words),
                                               deltime_words};

/* After the EXIT, TC POLY: S(XI), the polynomial whose degree and
 * coefficients are the words after the TC (DEC 8, then ten 2DECs, from
 * .083333334, 1/12, on); POLY returns past them, to DELTIME +027. */
void deltime_after_exit(void)
{
    tc_to(DELTIME + 1u, POLY);
}

/* DELTIME +027: the series' value in MPAC, and the interpreter again
 * (DELTIME_S_CODE). */
void deltime_after_poly(void)
{
    tc_at(DELTIME + 027u);
    intpret();
}

/* DELTIME +030: S(XI) stored, XI loaded again, and to the basic code for
 * the second series. */
static const word deltime_s_words[] = {
    /* DELTIME +030 */ STODL(S_XI),
    ADDRESS_PLUS_ONE(XI),
    ORDER(EXIT),
};

const struct interpretive_code DELTIME_S_CODE = {I_DELTIME >> 10, DELTIME + 030u,
                                                 COUNT(deltime_s_words), deltime_s_words};

/* After the EXIT, TC POLY: C(XI) (DEC 8 and ten 2DECs again, from
 * .031250001, 1/32, on); POLY returns to DELTIME +061. */
void deltime_after_second_exit(void)
{
    tc_to(DELTIME + 033u, POLY);
}

/* DELTIME +061: C(XI) in MPAC, and the interpreter again
 * (DELTIME_C_CODE). */
void deltime_after_second_poly(void)
{
    tc_at(DELTIME + 061u);
    intpret();
}

/* DELTIME +062: the time. X^2 C(XI) (XSQC(XI)), C1 times it; RTB TPMODE
 * makes it triple precision for the push-down list, which PDDL swaps for
 * X^2 there ("XCH WITH PL"). Then C2 X^2 S(XI), pushed the same way,
 * with R1 loaded; R1 shifted to the sum's scale and C2 X^2 S(XI) added;
 * normalized (the count in X1, "TO PRESERVE SIGNIF.") before it is
 * multiplied by X, and shifted back as C1 X^2 C(XI) is added. Over
 * ROOTMU (times 1/ROOTMU), it is T, and RVQ returns to KEPLER or
 * ITERATOR's caller. */
static const word deltime_c_words[] = {
    /* DELTIME +062 */ ORDERS(DMP, SRR_STAR), /* "PL AT 0" */
    ADDRESS_PLUS_ONE_D(0),
    SRR_X1(-5),
    STORE(XSQC_XI), /* "XSQC(XI) (+33 OR +31)" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(KEPC1),
    ORDERS(RTB, PDDL), /* "XCH WITH PL. 0D=C1 XSQ C(XI) (+49 OR +46)" */
    ADDRESS(TPMODE),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE(S_XI),
    SRR_X1(-5),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(KEPC2),
    ORDERS(RTB, PDDL), /* "3D=C2 XSQ S(XI) (+35 OR +33)" */
    ADDRESS(TPMODE),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(SR, TAD),
    SR_COUNT(6),
    ORDERS(NORM, DMP), /* "TO PRESERVE SIGNIF." */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(X),
    ORDERS(SR_STAR, TAD), /* "X(C2 XSQ S(XI) +R1) (+49 OR +46)" */
    SR_X1(-3),
    ORDERS(SL4R, DMPR),
    ADDRESS_PLUS_ONE(1_ROOTMU),
    STORE(T),
    ORDER(RVQ),
};

const struct interpretive_code DELTIME_C_CODE = {I_DELTIME >> 10, DELTIME + 062u,
                                                 COUNT(deltime_c_words), deltime_c_words};
