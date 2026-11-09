/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, CHECKCTR (LM p. 1183, CM p. 1286): basic code
 * that KEPLER and ITERATOR reach by RTB ("RTB BHIZ CHECKCTR"), to count
 * the iterations down. */
#include "conics.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* CHECKCTR: ITERCTR, in the job's VAC area (INDEX FIXLOC), less one, to
 * ITERCTR and to MPAC, where the BHIZ after the RTB tests it: at zero the
 * iterations have run out. Then the pair goes on (TC DANZIG). (ITERCTR
 * -1 overflows only from -16383, where TS ITERCTR skips the TS MPAC.) */
void checkctr(void)
{
    unsigned iterctr = indexed(ADDRESS_OF(ITERCTR), FIXLOC);

    set_A(negate(ONE)); /* CS ONE */
    ad_at(iterctr);     /* INDEX FIXLOC; AD ITERCTR */
    if (!ts(iterctr))   /* INDEX FIXLOC; TS ITERCTR */
        ts(MPAC_AT(0u));
    tc_at(CHECKCTR + 6u); /* TC DANZIG */
    continue_at(DANZIG);
}
