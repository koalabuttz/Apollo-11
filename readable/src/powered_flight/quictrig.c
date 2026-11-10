/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, QUICTRIG (LM p. 1261, CM p. 1367), bank
 * 23. The faithful C is c64-port/src/lm_powered_subs.c and
 * cm_powered_subs.c (and, for the LM's RTB, lm_powered_quick.c).
 *
 * QUICTRIG, "INTENDED FOR QUIDANCE CYCLE USE WHERE TIME IS CRITICAL, IS A
 * MUCH FASTER VERSION OF CD*TR*GS. QUICTRIG COMPUTES AND STORES THE SINES
 * AND COSINES OF THE 2'S COMPLEMENT ANGLES AT CDUSPOT, CDUSPOT +2, AND
 * CDUSPOT +4." Each by SPSIN and SPCOS, times BIT14, "SCALE DOWN TO MATCH
 * INTERPRETER OUTPUTS", to SINCDU and COSCDU at the same offset (4, 2,
 * then 0: CCS ITEMP2 and MASK SIX take two off). Interrupts are inhibited
 * throughout, "SINCE DAP USES THE SAME TEMPS"; the return waits in ITEMP1
 * and goes back by TC A. "QUICTRIG MAY BE CALLED FROM INTERPRETIVE AS AN
 * RTB OP-CODE, OR FROM BASIC VIA BANKCALL OR IBNKCALL."
 *
 * Called by BANKCALL, the run leaves the file at each TC SPSIN and TC
 * SPCOS, and goes on at the word after it (quictrig_to_spsin() and the
 * two after it); as the LM's RTB (FINDCDUW), SPSIN and SPCOS run in line
 * (quictrig()). */
#include "powered_flight.h"
#include "../single_precision/single_precision.h"
#include "../agc/names.h"

/* QUICTRIG +4: the next angle's offset into ITEMP2, and the angle into A
 * for SPSIN. */
static void next_angle(void)
{
    mask(SIX);
    ts(ADDRESS_OF(ITEMP2));
    ca(indexed(ADDRESS_OF(CDUSPOT), ITEMP2));
    tc_at(QUICTRIG + 010u); /* TC SPSIN */
}

/* QUICTRIG: interrupts off, the return into ITEMP1, the first angle
 * (CDUSPOT +4). */
static void start(void)
{
    inhint();
    qxch(ADDRESS_OF(ITEMP1));
    set_A(FOUR);
    next_angle();
}

/* After SPSIN: the sine, scaled, to SINCDU, and the angle into A again
 * for SPCOS. */
static void store_sine(void)
{
    mp(BIT14);
    dxch(indexed(ADDRESS_OF(SINCDU), ITEMP2));
    ca(indexed(ADDRESS_OF(CDUSPOT), ITEMP2));
    tc_at(QUICTRIG + 017u); /* TC SPCOS */
}

/* After SPCOS: the cosine, scaled, to COSCDU; then the next angle (true),
 * or, after the last, interrupts on and back by TC A. Not modelled:
 * ITEMP2 negative, which the MASK never leaves. */
static bool store_cosine(void)
{
    mp(BIT14);
    dxch(indexed(ADDRESS_OF(COSCDU), ITEMP2));
    switch (ccs_at(ADDRESS_OF(ITEMP2))) {
    case POSITIVE: next_angle(); return true;
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(ITEMP1));
        relint();
        tc_through(A_ADDRESS); /* TC A */
        return false;
    default: not_modelled(); return false;
    }
}

void quictrig(void)
{
    start();
    do {
        spsin();
        store_sine();
        spcos();
    } while (store_cosine());
}

void quictrig_to_spsin(void)
{
    start();
    continue_at(SPSIN);
}

void quictrig_after_spsin(void)
{
    store_sine();
    continue_at(SPCOS);
}

void quictrig_after_spcos(void)
{
    if (store_cosine()) continue_at(SPSIN);
}
