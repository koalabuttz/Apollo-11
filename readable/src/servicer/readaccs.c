/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 857-859 (Luminary099 only, bank 37): PREREAD, the
 * start of average G, and READACCS, the task that reads the PIPAs every
 * two seconds and sets up the SERVICER job. Restart group 5 follows them:
 * GNUFAZE5 and GNUTFAZ5 set its phase. The faithful C is
 * c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- PREREAD -- p. 857 ---- */

/* PREREAD: phase 5.7, the "SPOT TO SKIP LASTBIAS AFTER RESTART" (TC
 * GNUFAZE5), ... */
void preread(void)
{
    set_A(SEVEN);
    tc_to(PREREAD + 1u, GNUFAZE5);
}

/* ... and the LASTBIAS job, "LAST GYRO COMPENSATION IN FREE FALL" (NOVAC
 * at priority 21, 2CADR LASTBIAS). */
void preread_after_gnufaze5(void)
{
    set_A(PRIO21);
    tc_to(PREREAD + 3u, NOVAC);
}

/* BIBIBIAS: the PIPAs cleared and read (TC PIPASR +3; "DO NOT DESTROY
 * VALUE OF PIPTIME1"), ... */
void bibibias(void)
{
    tc_to(BIBIBIAS, PIPASR + 3u);
}

/* ... V37FLAG and AVEGFLAG set ("BITS 5 AND 6 OF FLAGWRD7"), DRIFTFLAG
 * reset, PIPAGE 4 ("INITIALIZE DV MONITOR"), OUTROUTE "TO END-OF-JOB"
 * (SERVEXIT +2), and the first average-G job, NORMLIZE (FINDVAC at
 * priority 22, 2CADR NORMLIZE). */
void bibibias_after_pipasr(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(SUPER011);
    ads(ADDRESS_OF(FLAGWRD7));

    set_A(negate(DRFTBIT));
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2));

    set_A(FOUR);
    ts(ADDRESS_OF(PIPAGE));

    set_A(ENDJBCAD); /* CADR SERVEXIT +2 */
    ts(ADDRESS_OF(OUTROUTE));

    set_A(PRIO22);
    tc_to(BIBIBIAS + 014u, FINDVAC);
}

/* Then phase 5.2, the "5.2SPOT FOR REREADAC AND NORMLIZE", and GOREADAX. */
void bibibias_after_findvac(void)
{
    set_A(TWO);
    goreadax();
}

/* GOREADAX: the phase in A set, with TBASE5 (TC GNUTFAZ5), ... */
void goreadax(void)
{
    tc_to(GOREADAX, GNUTFAZ5);
}

/* ... and READACCS two seconds on: "WAIT TWO SECONDS FOR READACCS"
 * (VARDELAY). */
void goreadax_after_gnutfaz5(void)
{
    set_A(TWO_SECS);
    tc_to(GOREADAX + 2u, VARDELAY);
}

/* ---- READACCS -- pp. 858-859 ---- */

/* READACCS: TIME5 nudged by one centisecond toward 037771, so that the
 * DAP's P-axis interrupt comes about 70 ms after this one: "THIS PIECE OF
 * CODING ATTEMPTS TO SYNCHRONIZE READACCS WITH THE DIGITAL AUTOPILOT".
 * CCS of TIME5 - 037771: positive, -1 added; negative, +1; -0 (TIME5 at
 * 037771), +0. (+0 needs TIME5 - 037771 to be +0, which one's complement
 * never gives: equal words make -0.) Then the PIPAs read (TC PIPASR), on
 * to PIPSDONE. */
void readaccs(void)
{
    set_A(negate(OCT37771));
    ad_at(ADDRESS_OF(TIME5));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: set_A(negate(ONE)); break; /* CS ONE; TCF +2 */
    case IS_PLUS_ZERO: break;                 /* TCF +2, A +0 */
    case NEGATIVE: set_A(ONE); break;         /* CA ONE */
    case IS_MINUS_ZERO: break;                /* +2, A +0 */
    }
    ads(ADDRESS_OF(TIME5));
    tc_to(READACCS + 7u, PIPASR);
}

/* PIPSDONE: phase 5.5 (TC GNUFAZE5), on to REDO5.5. */
void pipsdone(void)
{
    set_A(FIVE);
    tc_to(PIPSDONE + 1u, GNUFAZE5);
}

/* REDO5.5: PIPAGE 1, and the SERVICER job "SET UP" (FINDVAC at priority
 * 20, 2CADR SERVICER). */
void redo5_5(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(PIPAGE));
    set_A(PRIO20);
    tc_to(REDO5_5 + 3u, FINDVAC);
}

/* "TURN ON TEST CONNECTOR OUTBIT" (DSALMOUT bit 9). With AVEGFLAG down,
 * the final exit (AVEGOUT); with MUNFLAG clear, "BYPASS LR AND DISP."
 * (MAKEACCS); with group 2 running ("PHASE 2 ACTIVATED -- AVOID MULTIPLE
 * R10."), MAKEACCS too. Else PIPCTR 7, "FOR 4X/SEC RATE", TBASE2 ".05
 * SECONDS IN THE PAST", phase 2.21 ("2.21SPOT FOR R10,R11") and the first
 * R10,R11 task "IN .200 SECONDS" (WAITLIST, 2CADR R10,R11).
 *
 * CCS PHASE2 has two more outcomes, as the AGC runs them: a negative
 * PHASE2 skips to TS PIPCTR with its magnitude less one in A, and -0 past
 * it, to CS TIME1. */
void redo5_5_after_findvac(void)
{
    set_A(BIT9);
    wor_dsalmout();

    ca(ADDRESS_OF(FLAGWRD7));
    mask(AVEGFBIT);
    if (A_is_zero()) {
        avegout();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD6));
    mask(MUNFLBIT);
    if (A_is_zero()) {
        makeaccs();
        return;
    }
    switch (ccs_at(ADDRESS_OF(PHASE2))) {
    case POSITIVE: makeaccs(); return;
    case IS_PLUS_ZERO:
        set_A(SEVEN);
        ts(ADDRESS_OF(PIPCTR));
        break;
    case NEGATIVE: ts(ADDRESS_OF(PIPCTR)); break;
    case IS_MINUS_ZERO: break;
    }
    cs(ADDRESS_OF(TIME1));
    ad(FIVE);
    ad(NEG1_2);
    ad(NEG1_2);
    xch(ADDRESS_OF(TBASE2));

    set_A(DEC17); /* -PHASE2, PHASE2 = -17, 17 */
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE2));

    set_A(OCT24);
    tc_to(REDO5_5 + 037u, WAITLIST);
}

/* MAKEACCS: phase 5.4, and READACCS again: "DO PHASE CHANGE AND RECALL
 * READACCS". */
void makeaccs(void)
{
    set_A(FOUR);
    goreadax();
}

/* AVEGOUT: "SET UP FINAL SERVICER EXIT" (AVGEXIT gets AVOUTCAD's 2CADR
 * AVGEND), and phase 5.4, the "5.4 SPOT FOR REREADAC AND SERVICER" (TC
 * GNUTFAZ5); ... */
void avegout(void)
{
    dca_constant(AVOUTCAD); /* 2CADR AVGEND */
    dxch(ADDRESS_OF(AVGEXIT));
    set_A(FOUR);
    tc_to(AVEGOUT + 4u, GNUTFAZ5);
}

/* ... "END TASK WITHOUT CALLING READACCS" (TASKOVER). */
void avegout_after_gnutfaz5(void)
{
    tc_to(AVEGOUT + 5u, TASKOVER);
}

/* GNUFAZE5 +1: the phase in L and its complement in A ("-PHASE IN A,
 * PHASE IN L.") into -PHASE5 and PHASE5, and back (TC Q). */
static void set_phase5(void)
{
    cs(L_ADDRESS);
    dxch(ADDRESS_OF(MINUS_PHASE5));
    return_to_q();
}

/* GNUTFAZ5: group 5's phase from A ("SAVE INPUT IN L"), with TBASE5 the
 * time now (-TIME1), and on as GNUFAZE5 (TCF +2). */
void gnutfaz5(void)
{
    /* A TS that overflows stores the corrected word, leaves A +1 or -1 and
     * skips the next instruction: here CS TIME1, so TBASE5 gets that 1. */
    if (!ts(L_ADDRESS)) cs(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(TBASE5));
    set_phase5();
}

/* GNUFAZE5: group 5's phase from A. (A TS L that overflows skips CS L:
 * -PHASE5 gets A's +1 or -1.) */
void gnufaze5(void)
{
    if (ts(L_ADDRESS)) {
        dxch(ADDRESS_OF(MINUS_PHASE5));
        return_to_q();
        return;
    }
    set_phase5();
}

#endif
