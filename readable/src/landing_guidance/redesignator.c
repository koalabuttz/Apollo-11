/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 820-822 (Luminary099 only): the
 * REDESIGNATOR TRAP (bank 11) and the R.O.D. TRAP (bank 20). In P64 the
 * astronaut moves the landing site by pulsing the hand controller: each
 * pulse's interrupt runs PITFALL, which watches the controller (REDESMON)
 * until it comes back to rest and then adds the pulse to the site's
 * azimuth or elevation (AZINCR1, ELINCR1). In P66 each click of the rate
 * of descent switch runs DESCBITS, which counts it in RODCOUNT. The
 * faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- REDESIGNATOR TRAP -- p. 820 ---- */

/* PITFALL: the hand controller's interrupt. BANKRUPT and QRUPT keep the
 * interrupted program's FB and Q, and CHECKMM asks whether this is P64:
 * "IF NOT IN P64, NO REASON TO CONTINUE". */
void pitfall(void)
{
    xch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    tc_to(PITFALL + 3u, CHECKMM); /* DEC 64 */
}

/* Not P64: back to the interrupted program (TCF RESUME). */
void pitfall_not_p64(void)
{
    continue_at(RESUME);
}

/* P64: ELVIRA gets the controller's four pulse bits that are on (channel
 * 31 reads a deflection as 0, so COM; ALL4BITS: + and - pitch, + and -
 * yaw), ZERLINA counts two quiet passes, and REDESMON runs as a task 50
 * ms from now (TWIDDLE). (A masked word never overflows: no TS skips.) */
void pitfall_in_p64(void)
{
    read_channel(CHAN31);
    com();
    mask(ALL4BITS);
    ts(ADDRESS_OF(ELVIRA));
    set_A(TWO);
    ts(ADDRESS_OF(ZERLINA));
    set_A(FIVE);
    tc_to(PITFALL + 016u, TWIDDLE); /* ADRES REDESMON */
}

/* TWIDDLE's return: TCF RESUME. */
void pitfall_after_twiddle(void)
{
    continue_at(RESUME);
}

/* PREMON1 and PREMON2: REDESMON again in 70 ms (VARDELAY), PREMON1 with
 * ZERLINA one less. */
static void premon2(void)
{
    set_A(SEVEN);
    tc_to(PREMON2 + 1u, VARDELAY);
}

static void premon1(void)
{
    ts(ADDRESS_OF(ZERLINA)); /* (A is CCS's: never an overflow) */
    premon2();
}

static void count_em(void);

/* RESETRPT: "RESET RUPT. TERMINATE": channel 13 bit 12 enables the hand
 * controller's interrupt again, and the task ends (TASKOVER). */
static void resetrpt(void)
{
    set_A(BIT12);
    wor_channel(CHAN13);
    continue_at(TASKOVER);
}

/* REDESMON: the controller's bits now go to ELVIRA, and the last pass's to
 * L. While any bit is on, the monitor goes on ("CONTINUE MONITOR"). Once
 * none is, the last pass's are the pulse to count (COUNT'EM); if that pass
 * had none either, ZERLINA counts down to 0, and then the interrupt is
 * reset. Every CCS here is a word of four masked bits but two: CCS L, of
 * the last pass's ELVIRA, and CCS ZERLINA, which the faithful port takes
 * of any word. The outcomes no monitor sees (ELVIRA or L or ZERLINA
 * negative) are followed as the AGC would; the CCS table is
 *
 *       CCS ELVIRA        CCS L             CCS ZERLINA
 *       TCF PREMON2       TCF COUNT'EM      TCF PREMON1
 *                                           RESETRPT  CAF BIT12
 *                                                     EXTEND
 *                                                     WOR CHAN13
 *                                                     TCF TASKOVER
 *
 * so CCS ELVIRA's - and -0 run CCS L's TCF COUNT'EM and CCS ZERLINA, CCS
 * L's - and -0 run TCF PREMON1 and RESETRPT, and CCS ZERLINA's - runs the
 * WOR with A |ZERLINA| - 1, its -0 the WOR's word without its EXTEND: TC
 * 5013. */
void redesmon(void)
{
    read_channel(CHAN31);
    com();
    mask(ALL4BITS);
    xch(ADDRESS_OF(ELVIRA));
    ts(L_ADDRESS);                        /* (A, a word: no overflow) */
    switch (ccs_at(ADDRESS_OF(ELVIRA))) { /* "DO ANY BITS APPEAR THIS PASS?" */
    case POSITIVE: premon2(); return;     /* "Y: CONTINUE MONITOR" */
    case IS_PLUS_ZERO: break;
    case NEGATIVE: count_em(); return; /* (never) */
    case IS_MINUS_ZERO: goto zerlina;  /* (never) */
    }
    switch (ccs_at(L_ADDRESS)) {       /* "N: ANY LAST PASS?" */
    case POSITIVE: count_em(); return; /* "Y: COUNT 'EM, RESET RUPT, TERMINATE" */
    case IS_PLUS_ZERO: break;
    case NEGATIVE: premon1(); return;       /* (never) */
    case IS_MINUS_ZERO: resetrpt(); return; /* (never) */
    }
zerlina:
    switch (ccs_at(ADDRESS_OF(ZERLINA))) { /* "N: HAS ZERLINA REACHED ZERO YET?" */
    case POSITIVE: premon1(); return;      /* "N: DIMINISH ZERLINA, CONTINUE" */
    case IS_PLUS_ZERO: resetrpt(); return; /* "Y: RESET RUPT. TERMINATE" */
    case NEGATIVE:                         /* (never) */
        wor_channel(CHAN13);
        continue_at(TASKOVER);
        return;
    case IS_MINUS_ZERO: tc_to(RESETRPT + 2u, 05013u); return; /* (never) */
    }
}

/* TC 5013's return: TCF TASKOVER. */
void resetrpt_taskover(void)
{
    continue_at(TASKOVER);
}

/* One pulse's bit: CCS A on it, masked from L, then ADS. A bit on runs
 * the C(S) or CA of the step; a bit off skips it, and +0 is added. */
static void add_if(word bit, word step, unsigned increment)
{
    ca(L_ADDRESS);
    mask(bit);
    if (ccs_at(A_ADDRESS) == POSITIVE) set_A(step);
    ads(increment);
}

/* COUNT'EM: "ARE WE IN ATTITUDE-HOLD?" (channel 31 bit 13 off): then
 * "SKIP REDESIGNATION LOGIC". Else each of the last pass's bits moves the
 * site: azimuth 2 degrees (AZEACH), elevation half a degree (ELEACH),
 * -AZBIT and -ELBIT down, +AZBIT and +ELBIT up ("THESE EQUIVALENCES ARE
 * BASED ON GSOP CHAPTER 4, REVISION 16 OF P64LM"); then RESETRPT. */
static void count_em(void)
{
    set_A(BIT13);
    rand_channel(CHAN31);
    if (A_is_zero()) { /* EXTEND; BZF RESETRPT */
        resetrpt();
        return;
    }
    add_if(BIT6, negate(AZEACH), ADDRESS_OF(AZINCR1)); /* -AZBIT: CS AZEACH */
    add_if(BIT5, AZEACH, ADDRESS_OF(AZINCR1));         /* +AZBIT: CA AZEACH */
    add_if(BIT1, negate(ELEACH), ADDRESS_OF(ELINCR1)); /* -ELBIT: CS ELEACH */
    add_if(BIT2, ELEACH, ADDRESS_OF(ELINCR1));         /* +ELBIT: CA ELEACH */
    resetrpt();
}

/* ---- R.O.D. TRAP -- p. 822 ---- */

/* DESCBITS: "COME HERE FROM MARKRUPT CODING WITH BIT 7 OR 6 OF CHANNEL 16
 * IN A; BIT 7 MEANS - RATE INCREMENT, BIT 6 + INCREMENT." RODCOUNT gets
 * -1 or +1 (CS TWO, AD ONE; or, with bit 7 off, AD ONE alone). "TRAP IS
 * RESET WHEN SWITCH IS RELEASED". */
void descbits(void)
{
    mask(BIT7);
    if (ccs_at(A_ADDRESS) == POSITIVE) set_A(negate(TWO));
    ad(ONE);
    ads(ADDRESS_OF(RODCOUNT));
    continue_at(RESUME);
}

#endif
