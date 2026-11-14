/* SPDX-License-Identifier: MIT */
/* SPS_BACK-UP_RCS_CONTROL.agc, the LM's: Luminary099 pages 1507-1510, bank
 * 21, in EBANK 6 (the listing's EBANK= TJU). PROGRAM NAME SPSRCS, by "EDGAR
 * M. OSHIKA (AC ELECTRONICS)", "MODIFIED TO IMPROVE BENDING STABILITY BY G.
 * KALAN, FEB. 14, 1969". The faithful C is c64-port/src/lm_sps_backup.c.
 *
 * The listing's own outline, in five steps: "JET SENSE TEST (SPSRCS)",
 * "RATE DEAD BAND TEST", "OUTER RATE LIMIT TEST (SPSSTART)", "COAST ZONE
 * TEST" and "INHIBITION LOGIC". The code runs them in that order but sits
 * in the bank backwards: the inhibition logic (POSTHRST to CTRCHECK) comes
 * first, the jet sense and rate tests (NEGFIRE, PLUSFIRE, SPSRCS) after
 * it, and the two outer tests last. Every way out of the outer tests
 * stores the new jet time at POSTHRST or POSTHRST +1, so that is one
 * function here (store_jet_time), and the inhibition logic another
 * (inhibition), which ends at TC Q. */
#include "sps_backup.h"

#if LUMINARY099

/* INDEX AXISCTR: the axis's word of a P, U, V triple (TJP, TJU, TJV; the
 * inhibition counters; UTIME's three times). */
static unsigned by_axis(unsigned address)
{
    return indexed(address, fetch(ADDRESS_OF(AXISCTR)));
}

/* TC Q. */
static void tc_q(void)
{
    tc_through(Q_ADDRESS);
}

/* ---- INHIBITION LOGIC -- LM pp. 1508-1509 ---- */

/* ZAPTJ -- LM p. 1509: zero jet time, and TC Q. */
static void zaptj(void)
{
    set_A(ZERO);
    ts(by_axis(ADDRESS_OF(TJU)));
    tc_q();
}

/* POSTHRST +3 -- LM p. 1508, with the axis's new time stored: "IF OLDSENSE
 * IS NON-ZERO:" "RETURN IF JET TIME AS THE SAME SIGN AS OLDSENSE", "SET
 * INHIBITION COUNTER* AND RETURN IF JET TIME IS ZERO", or "SET INHIBITION
 * COUNTER,* SET JET TIME TO ZERO AND RETURN IF SIGN" "OF JET TIME IS
 * OPPOSITE TO THAT OF OLDSENSE". "IF OLDSENSE IS ZERO:" "RETURN IF
 * INHIBITION COUNTER IS NOT POSITIVE", or "SET JET TIME TO ZERO AND RETURN
 * IF INHIBITION COUNTER IS POSITIVE". At CTRCHECK a negative counter takes
 * the positive one's way, to ZAPTJ, and -0 returns like +0. OLDSENSE is
 * never -0 here: every way in sets it to +1, -1 or +0. It would enter
 * NEGCHECK past its INDEX AXISCTR, at a CS TJU. */
static void inhibition(void)
{
    switch (ccs_at(ADDRESS_OF(OLDSENSE))) {
    case POSITIVE: ca(by_axis(ADDRESS_OF(TJU))); break; /* POSCHECK */
    case IS_PLUS_ZERO:
        switch (ccs_at(by_axis(ADDRESS_OF(UJETCTR)))) { /* CTRCHECK */
        case POSITIVE: zaptj(); return;    /* TCF +2: "CTR IS POSITIVE.  INHIBIT FIRINGS" */
        case IS_PLUS_ZERO: tc_q(); return; /* "CTR IS NOT POSITIVE.  RETURN" */
        case NEGATIVE: zaptj(); return;
        case IS_MINUS_ZERO: tc_q(); return;
        }
        return;
    case NEGATIVE: cs(by_axis(ADDRESS_OF(TJU))); break; /* NEGCHECK */
    case IS_MINUS_ZERO: cs(ADDRESS_OF(TJU)); break;     /* never */
    }
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* NEGCHECK +2 */
        tc_q();                          /* "RETURN" */
        return;
    }
    ca(by_axis(UTIME)); /* SETCTR: "JET FIRING REVERSAL COMMANDED.  SET CTR," */
    ts(by_axis(ADDRESS_OF(UJETCTR)));
    zaptj();
}

/* POSTHRST +1 -- LM p. 1508, with the jet time in A: store it as the axis's,
 * then the inhibition logic. */
static void store_jet_time(void)
{
    ts(by_axis(ADDRESS_OF(TJU)));
    inhibition();
}

/* ---- OUTER RATE LIMIT and COAST ZONE TESTS -- LM pp. 1509-1510 ---- */

/* SPSSTART -- LM p. 1509: "IF MAGNITUDE OF EDOT IS GREATER THAN 1.73 DEG/SEC
 * SET JET FIRING TIME" "TO REDUCE RATE": MP RATELIM1 leaves A nonzero just
 * when it is. Then the coast zone, with DKDB the "PAD LOADED DEADBAND.
 * FRESHSTART: 1.4 DEG": "IF STATE (E,EDOT) IS BELOW LINE E + 4 X EDOT > -1.4
 * DEG AND EDOT IS LESS THAN 1.30 DEG/SEC SET JET TIME" "POSITIVE", and "IF
 * STATE IS ABOVE LINE E + 4 X EDOT > +1.4 DEG AND EDOT IS GREATER THAN -1.30
 * DEG/SEC, SET JET TIME NEGATIVE"; "OTHERWISE, SET JET FIRING TIME TO ZERO".
 * The outline's 1.30 DEG/SEC is RATELIM2, OCT 00632, whose own comment reads
 * "1.125 DEG/SEC". */
static void spsstart(void)
{
    ca(ADDRESS_OF(EDOT));
    mp(RATELIM1);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: /* NEGTHRST */
        set_A(negate(HALF));
        store_jet_time();
        return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: /* POSTHRST */
        set_A(HALF);
        store_jet_time();
        return;
    case IS_MINUS_ZERO: break;
    }
    ca(ADDRESS_OF(EDOT)); /* "COAST ZONE TEST" */
    ad_at(ADDRESS_OF(E));
    mp_at(ADDRESS_OF(DKDB));
    if (A_is_zero()) {
        set_A(ZERO); /* TJZERO */
    } else if (!A_is_zero_or_minus()) {
        ca(ADDRESS_OF(EDOT));
        ad(RATELIM2);
        set_A(A_is_zero_or_minus() ? ZERO : negate(HALF)); /* TJZERO or NEGTHRST */
    } else {
        set_A(negate(RATELIM2)); /* +7 */
        ad_at(ADDRESS_OF(EDOT));
        set_A(A_is_zero_or_minus() ? HALF : ZERO); /* POSTHRST or TJZERO */
    }
    store_jet_time();
}

/* ---- JET SENSE and RATE DEAD BAND TESTS -- LM p. 1509 ---- */

/* SPSRCS -- LM p. 1509: "IF JETS ARE FIRING NEGATIVELY, SET OLDSENSE
 * NEGATIVE AND CONTINUE", positively "SET OLDSENSE POSITIVE", and "IF JETS
 * ARE NOT FIRING, SET OLDSENSE TO ZERO AND GO TO OUTER RATE LIMIT TEST".
 * Firing, the "RATE DEAD BAND TEST": "IF JETS ARE FIRING NEGATIVELY AND RATE
 * IS GREATER THAN TARGET RATE, LEAVE" "JETS ON AND GO TO INHIBITION LOGIC",
 * the same positively when the rate is less; "IF DRIFTBIT = 1, USE ZERO
 * TARGET RATE", "IF DRIFTBIT = 0, USE 0.10 RATE TARGET" (RATEDB1, which is
 * TBUILDFX, the fixed-fixed pool's DEC 37, shared by name). */
void spsrcs(void)
{
    /* "TJ MUST NOT BE NEGATIVE ZERO": a -0 one runs as +0, jets off. */
    switch (ccs_at(by_axis(ADDRESS_OF(TJU)))) {
    case POSITIVE: /* PLUSFIRE */
        set_A(ONE);
        ts(ADDRESS_OF(OLDSENSE));
        cs(ADDRESS_OF(EDOT));
        break;
    case IS_PLUS_ZERO:
        ts(ADDRESS_OF(OLDSENSE)); /* "JETS OFF" */
        spsstart();
        return;
    case NEGATIVE: /* NEGFIRE */
        set_A(negate(ONE));
        ts(ADDRESS_OF(OLDSENSE));
        ca(ADDRESS_OF(EDOT));
        break;
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(OLDSENSE)); /* "JETS OFF" */
        spsstart();
        return;
    }
    lxch(A_ADDRESS);
    cs(ADDRESS_OF(DAPBOOLS));
    mask(DRIFTBIT);
    if (!A_is_zero()) set_A(RATEDB1);
    ad_at(L_ADDRESS);
    if (A_is_zero_or_minus()) {
        spsstart();
        return;
    }
    inhibition(); /* POSTHRST +3: the jets stay on */
}

#endif
