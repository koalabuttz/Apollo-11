/* SPDX-License-Identifier: MIT */
/* RTB_OP_CODES.agc (Luminary099) and RT8_OP_CODES.agc (Comanche055), the
 * same code in both: the routines an interpretive RTB runs. Only CDULOGIC
 * so far. */
#include "rtb_op_codes.h"
#include "../agc/names.h"

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
