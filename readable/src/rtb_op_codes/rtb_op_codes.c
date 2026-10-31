/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055), the
 * same code in both: the routines an interpretive RTB runs. So far
 * CDULOGIC, and 2V1STO2S with its 1TO2SUB. */
#include "rtb_op_codes.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

static void one_to2sub(void);

/* CDULOGIC -- LM p. 1397, CM p. 1508.
 * "CONVERT THE SINGLE PRECISION 2'S COMPLEMENT NUMBER ARRIVING IN MPAC
 * (SCALED IN HALF-REVOLUTIONS) TO A DP 1'S COMPLEMENT NUMBER SCALED IN
 * REVOLUTIONS": a CDU angle. MPAC times HALF is the angle in revolutions;
 * a negative one also gets -HALF in its low word (CS HALF, kept in MPAC +1
 * before the product is added in), which turns the two's complement count
 * into ones' complement. "MODE IS ALREADY AT DOUBLE-PRECISION": back to
 * DANZIG. */
void cdulogic(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: set_A(ZERO); break;              /* CAF ZERO; TCF +3 */
    case IS_PLUS_ZERO: break;                       /* TCF +3 */
    case NEGATIVE:                                  /* NOOP */
    case IS_MINUS_ZERO: set_A(negate(HALF)); break; /* CS HALF */
    }
    ts(ADDRESS_OF(MPAC) + 1u);
    set_A(ZERO); /* CAF ZERO */
    xch(ADDRESS_OF(MPAC));
    mp(HALF); /* EXTEND; MP HALF */
    das(ADDRESS_OF(MPAC));
    continue_at(DANZIG);
}

/* 2V1STO2S -- LM p. 1398, CM p. 1509.
 * "V1STO2S FOR 2 COMPONENT VECTOR. USED BY RR": the two angles in MPAC and
 * MPAC +3 (DP, in revolutions) to two single precision CDU angles in two's
 * complement, MPAC and MPAC +1, the mode double precision. Each goes
 * through 1TO2SUB, the first one's answer kept in MPAC +3 meanwhile. */
void two_v1sto2s(void)
{
    tc_at(TWO_V1STO2S);
    one_to2sub();
    dxch(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(MPAC));
    tc_at(TWO_V1STO2S + 3u);
    one_to2sub();
    ts(L_ADDRESS);
    ca(ADDRESS_OF(MPAC) + 3u);
    sload2(); /* TCF SLOAD2 */
}

/* 1TO2SUB -- LM p. 1398, CM p. 1509.
 * "SUBROUTINE TO DO DOUBLING AND 1'S TO 2'S CONVERSION": MPAC, doubled
 * (revolutions to half-revolutions), as one word in two's complement, to
 * MPAC and A. The CCS table makes a negative angle's two's complement form
 * ("THIS WAS REVERSE OF MSU"); a doubling that overflowed gets the limit
 * of its sign added back in (INDEX A; CAF LIMITS). "FINAL MPAC +1
 * UNSPECIFIED." */
static void one_to2sub(void)
{
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ad(ONE); break; /* AD ONE; TCF +2 */
    case IS_PLUS_ZERO: break;      /* TCF +2 */
    case NEGATIVE: com(); break;   /* COM: "THIS WAS REVERSE OF MSU." */
    case IS_MINUS_ZERO: break;
    }
    if (ts(ADDRESS_OF(MPAC))) { /* "AND SKIP ON OVERFLOW." */
        /* "OVERFLOW UNCORRECT AND IN MSU." */
        ca(indexed(ADDRESS_OF(LIMITS), a_word()));
        ads(ADDRESS_OF(MPAC));
    }
    tc_through(Q_ADDRESS); /* TC Q */
}
