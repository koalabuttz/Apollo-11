/* SPDX-License-Identifier: MIT */
/* ASCENT_GUIDANCE.agc, pp. 854 and 856 (Luminary099 only, banks 27 and
 * 30): YCOMP, the out-of-plane distance, and LOGSUB, "THE LOGARITHM
 * SUBROUTINE". The faithful C is c64-port/src/lm_ascent.c. */
#include "ascent.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ---- YCOMP -- p. 854 (bank 27) ---- */

/* YCOMP: Y, the distance out of the CSM's plane, at the injection radius:
 *
 *     Y = (UNIT/R/ . QAXIS) RCO
 *
 * and back (RVQ). */
static const word ycomp_words[] = {
    /* YCOMP */ ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ADDRESS_PLUS_ONE(QAXIS),
    ORDERS(SL2, DMP),
    ADDRESS_PLUS_ONE(RCO),
    STORE(Y),
    ORDER(RVQ),
};

const struct interpretive_code YCOMP_CODE = {027u, YCOMP, COUNT(ycomp_words), ycomp_words};

/* ---- LOGSUB -- p. 856 (bank 30) ---- */

/* LOGSUB: "INPUT ..... X IN MPAC", "OUTPUT ..... -LOG(X) IN MPAC". NORM
 * shifts X up to between 1/2 and 1 (the shift count to MPAC +6), and BDSU
 * takes it from NEARONE: 1 less the shifted X, for the polynomial in the
 * basic code after the EXIT. */
static const word logsub_words[] = {
    /* LOGSUB */ ORDERS(NORM, BDSU),
    ADDRESS_PLUS_ONE_AT(MPAC, 6),
    ADDRESS_PLUS_ONE(NEARONE),
    ORDER(EXIT),
};

const struct interpretive_code LOGSUB_CODE = {030u, LOGSUB, COUNT(logsub_words), logsub_words};

/* After the EXIT, TC POLY: the polynomial whose degree and coefficients
 * are the words after the TC (DEC 6, then eight 2DECs); POLY returns past
 * them, to LOGSUB +026. */
void logsub_after_exit(void)
{
    tc_to(LOGSUB + 4u, POLY);
}

/* LOGSUB +026: the polynomial's value in MPAC (its third word cleared),
 * and the log of the shifted part: the shift count times CLOG2/32 (the
 * log of 2, scaled by 32). The DXCHes put CLOG2/32 in MPAC and the value
 * in BUF +1, and SHORTMP multiplies MPAC by the count in A. (TS of +0
 * never overflows: it does not skip.) */
void logsub_after_poly(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(MPAC_AT(2u));
    set_A(CLOG2_32[0]); /* EXTEND; DCA CLOG2/32 */
    set_L(CLOG2_32[1]);
    dxch(MPAC_AT(0u));
    dxch(ADDRESS_OF(BUF) + 1u);
    ca(MPAC_AT(6u));
    tc_to(LOGSUB + 035u, SHORTMP);
}

/* SHORTMP's return: the product to A,L and the polynomial's value back,
 * the two added (DAS), and the interpreter again (LOGSUB_RVQ_CODE). */
void logsub_after_shortmp(void)
{
    dxch(MPAC_AT(1u));
    dxch(MPAC_AT(0u));
    dxch(ADDRESS_OF(BUF) + 1u);
    das(MPAC_AT(0u));
    tc_at(LOGSUB + 042u);
    intpret();
}

/* The sum negated, -LOG(X), and back (RVQ). */
static const word logsub_rvq_words[] = {
    /* LOGSUB +043 */ ORDERS(DCOMP, RVQ),
};

const struct interpretive_code LOGSUB_RVQ_CODE = {030u, LOGSUB + 043u, COUNT(logsub_rvq_words),
                                                  logsub_rvq_words};

#endif
