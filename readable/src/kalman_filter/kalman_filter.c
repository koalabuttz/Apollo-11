/* SPDX-License-Identifier: MIT */
/* KALMAN_FILTER.agc, the LM's: Luminary099 pages 1470-1471, bank 16, in
 * EBANK 6 (the listing's EBANK= NO.UJETS). */
#include "kalman_filter.h"

#if LUMINARY099

/* The jet time this pass fires on the axis DAPTEMP6 names, at 1 second,
 * signed, into A. A time of 100 ms or less (in TIME6's units, 160) all
 * fires now (SMALLTJU: TJP, TJU or TJV becomes +0, and MP ELEVEN rescales
 * it, "10.24 PLUS"); a longer one fires 100 ms, its magnitude cut by 160
 * for the next pass. A TJ of +0 fires nothing; one of -0 counts as a
 * short one. */
static void jet_time(void)
{
    unsigned tj = indexed(ADDRESS_OF(TJP), fetch(ADDRESS_OF(DAPTEMP6)));

    switch (ccs_at(tj)) {
    case IS_PLUS_ZERO: return; /* TCF LOOPRATE, A +0 */
    case POSITIVE:
    case NEGATIVE: ad(MINUS_100MST6); break;
    case IS_MINUS_ZERO: break; /* A +0 */
    }
    if (A_is_zero_or_minus()) {
        set_A(ZERO); /* SMALLTJU */
        xch(tj);
        mp(ELEVEN);
        ca(L_ADDRESS);
        return;
    }
    switch (ccs_at(tj)) {
    case POSITIVE: set_A(MINUS_100MST6); break;
    case NEGATIVE: set_A(negate(MINUS_100MST6)); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* ADS of +0 */
    }
    ads(tj);
    switch (ccs_at(tj)) {
    case POSITIVE: set_A(negate(MINUS_100MS)); break; /* "0.1 AT 1" */
    case NEGATIVE: set_A(MINUS_100MS); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* A +0 */
    }
}

/* ---- RATELOOP -- LM pp. 1470-1471 ---- */

/* RATELOOP: DAPTEMP6 counts the axes down, 2, 1, 0 (V, U, P), and Q holds
 * twice it, the axis's pair of DOWNTORK counters. LOOPRATE: the torque,
 * the time times the axis's jet count, is the "SIGNED TORQUE AT 1
 * JET-SEC FOR FILTER"; MP BIT10 "RESCALE TO 32; ONE BIT ABOUT 2 JET-MSEC";
 * and STORTORK adds its magnitude to the axis's positive counter, or, for
 * a negative torque (NEGTORK), to the next one: "INCREMENT DOWNLIST
 * REGISTER." ("NOTE: NOT INITIALIZED; OVERFLOWS."). Then ROTORQUE: the
 * jet rates about R and Q, from the sum and the difference of the U and V
 * torques, and TCF BACKP. */
void rateloop(void)
{
    bool more = true;

    set_A(TWO);
    while (more) {
        unsigned axis;

        ts(ADDRESS_OF(DAPTEMP6));
        double_A();
        ts(Q_ADDRESS); /* never overflows: A is at most 4 */
        jet_time();
        axis = fetch(ADDRESS_OF(DAPTEMP6));
        mp_at(indexed(ADDRESS_OF(NO_PJETS), axis)); /* LOOPRATE */
        ca(L_ADDRESS);
        ts(indexed(ADDRESS_OF(DAPTEMP1), axis));
        mp(BIT10);
        if (A_is_zero_or_minus()) {
            com(); /* NEGTORK */
            incr(Q_ADDRESS);
        }
        ads(indexed(ADDRESS_OF(DOWNTORK), Q)); /* STORTORK */
        switch (ccs_at(ADDRESS_OF(DAPTEMP6))) {
        case POSITIVE: break;                   /* TCF RATELOOP +1, A the next axis */
        case IS_PLUS_ZERO: more = false; break; /* TCF ROTORQUE */
        default: not_modelled(); return;        /* DAPTEMP6 is never negative */
        }
    }
    ca(ADDRESS_OF(DAPTEMP2)); /* ROTORQUE */
    ad_at(ADDRESS_OF(DAPTEMP3));
    mp_at(ADDRESS_OF(1JACCR));
    ts(ADDRESS_OF(JETRATER));
    cs(ADDRESS_OF(DAPTEMP3));
    ad_at(ADDRESS_OF(DAPTEMP2));
    mp_at(ADDRESS_OF(1JACCQ));
    ts(ADDRESS_OF(JETRATEQ));
    continue_at(BACKP);
}

#endif
