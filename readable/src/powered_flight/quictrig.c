/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, p. 1261 (Luminary099, bank 23):
 * QUICTRIG. The faithful C is c64-port/src/lm_powered_quick.c. */
#include "powered_flight.h"
#include "../single_precision/single_precision.h"
#include "../agc/names.h"

#if LUMINARY099

/* QUICTRIG, "INTENDED FOR QUIDANCE CYCLE USE WHERE TIME IS CRITICAL, IS A
 * MUCH FASTER VERSION OF CD*TR*GS. QUICTRIG COMPUTES AND STORES THE SINES
 * AND COSINES OF THE 2'S COMPLEMENT ANGLES AT CDUSPOT, CDUSPOT +2, AND
 * CDUSPOT +4." Each by SPSIN and SPCOS, times BIT14, "SCALE DOWN TO MATCH
 * INTERPRETER OUTPUTS", to SINCDU and COSCDU at the same offset (4, 2,
 * then 0: CCS ITEMP2 and MASK SIX take two off). Interrupts are inhibited
 * throughout, "SINCE DAP USES THE SAME TEMPS"; the return waits in ITEMP1
 * and goes back by TC A. "QUICTRIG MAY BE CALLED FROM INTERPRETIVE AS AN
 * RTB OP-CODE, OR FROM BASIC VIA BANKCALL OR IBNKCALL." */
void quictrig(void)
{
    inhint();
    qxch(ADDRESS_OF(ITEMP1));
    set_A(FOUR);
    do {
        mask(SIX); /* QUICTRIG +4 */
        ts(ADDRESS_OF(ITEMP2));
        ca(indexed(ADDRESS_OF(CDUSPOT), ITEMP2));
        tc_at(QUICTRIG + 010u);
        spsin();
        mp(BIT14);
        dxch(indexed(ADDRESS_OF(SINCDU), ITEMP2));
        ca(indexed(ADDRESS_OF(CDUSPOT), ITEMP2));
        tc_at(QUICTRIG + 017u);
        spcos();
        mp(BIT14);
        dxch(indexed(ADDRESS_OF(COSCDU), ITEMP2));
    } while (ccs_at(ADDRESS_OF(ITEMP2)) == POSITIVE);
    ca(ADDRESS_OF(ITEMP1));
    relint();
    tc_through(A_ADDRESS); /* TC A */
}

#endif
