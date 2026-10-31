/* SPDX-License-Identifier: MIT */
/* The keyboard and uplink interrupts: KEYRUPT_UPRUPT.agc of Luminary099
 * (LM, listing pages 1338-1340) and Comanche055 (CM, pages 1449-1451). The
 * faithful C is c64-port/src/keyrupt_uprupt.c.
 *
 * The two missions run the same code. Where the LM's listing names a flag
 * bit (DSKYFBIT, UPLOCBIT) the CM's writes the bit itself (BIT15, BIT4);
 * the code shows both in place.
 *
 * How an uplink word carries a key code. The ground sends each five-bit
 * code three times in one word: the code in bits 1-5, its complement in
 * bits 6-10 and the code again in bits 11-15. "TRIPLE CHAR REDUNDANCY":
 * a word whose three groups do not agree is a failure, which locks out
 * the uplink (UPLOCKFL) until the ground sends ERROR RESET.
 *
 * The binding (src/binding/keyrupt_routines.c) hands these routines the
 * states the original's lead-ins and callers make: inside the interrupt,
 * no overflow in A at an entry, so no TS here skips unless the code says
 * so. */
#include "keyrupt.h"
#include "../agc/names.h"

static void acceptup(void);
static void uprpt1(void);
static bool uptest(word at);
static void upok(void);
static void cluplock(void);
static void tmfail2(void);

/* ======================================================================
 * A key on the main DSKY
 * ====================================================================== */

/* KEYRUPT1 -- LM p. 1338, CM p. 1449.
 * The KEYRUPT1 interrupt's lead-in comes here with the interrupted BBANK in
 * A. Save it (BANKRUPT) and Q (QRUPT), and take the time: "TIME IS
 * SNATCHED IN RUPT FOR NOUN 65." LODSAMPT returns to
 * keyrupt1_after_lodsampt(). */
void keyrupt1(void)
{
    ts(ADDRESS_OF(BANKRUPT));
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(QRUPT))) { /* a Q that held an overflow: TS skips the TC */
        keyrupt1_after_lodsampt();
        return;
    }
    tc_at(KEYRUPT1 + 3u); /* TC LODSAMPT */
    continue_at(LODSAMPT);
}

/* KEYRUPT1 +4 -- LM p. 1338, CM p. 1449 (LODSAMPT's return).
 * The key's code: the low five bits of channel 15 ("CHECK IF KEYS 5M-1M
 * ON"). */
void keyrupt1_after_lodsampt(void)
{
    set_A(LOW5);
    rand_channel(MNKEYIN);
    keycom();
}

/* KEYCOM -- LM p. 1338, CM p. 1449.
 * A holds a key code. (The CM's MARKRUPT comes here too, with a key code
 * from the navigation DSKY: SXTMARK's "IT,S A KEY CODE, NOT A MARK.")
 * Keep it in RUPTREG4 and set DSKYFLAG ("DISPLAYS SENT TO DSKY"), bit 15 of
 * FLAGWRD5, then start CHARIN. */
void keycom(void)
{
    ts(ADDRESS_OF(RUPTREG4));
    cs(ADDRESS_OF(FLAGWRD5));
#if LUMINARY099
    mask(DSKYFBIT);
#else
    mask(BIT15);
#endif
    ads(ADDRESS_OF(FLAGWRD5)); /* the bit if it was clear, nothing if set */
    acceptup();
}

/* ACCEPTUP -- LM p. 1338, CM p. 1449.
 * Start CHARIN as a job: TC NOVAC, with CHARIN's 2CADR in the two words
 * after the TC. NOVAC comes back to acceptup_after_novac() with LOCCTR
 * at the new job's core set. (The note in the listing: "RUPTREG4 =
 * KEYTEMP1", one word, which holds the code by either name.) */
static void acceptup(void)
{
    set_A(CHRPRIO);
    tc_at(ACCEPTUP + 1u); /* TC NOVAC; 2CADR CHARIN */
    continue_at(NOVAC);
}

/* ACCEPTUP +4 -- LM p. 1338, CM p. 1449 (NOVAC's return).
 * "LEAVE 5 BIT KEY CDE IN MPAC FOR CHARIN": the code goes to MPAC in the
 * new job's core set, which CHARIN finds when the job runs. The interrupt
 * is over: RESUME. */
void acceptup_after_novac(void)
{
    ca(ADDRESS_OF(RUPTREG4));
    ts(indexed(ADDRESS_OF(MPAC), LOCCTR)); /* INDEX LOCCTR; TS MPAC */
    tc_at(ACCEPTUP + 7u);                  /* TC RESUME */
    continue_at(RESUME);
}

/* ======================================================================
 * A word from the ground
 * ====================================================================== */

/* UPRUPT -- LM p. 1339, CM p. 1450.
 * As KEYRUPT1, for the UPRUPT interrupt: the uplink has put a word in
 * INLINK. LODSAMPT returns to uprupt_after_lodsampt(). */
void uprupt(void)
{
    ts(ADDRESS_OF(BANKRUPT));
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(QRUPT))) { /* a Q that held an overflow: TS skips the TC */
        uprupt_after_lodsampt();
        return;
    }
    tc_at(UPRUPT + 3u); /* TC LODSAMPT */
    continue_at(LODSAMPT);
}

/* UPRUPT +4 -- LM p. 1339, CM p. 1450 (LODSAMPT's return).
 * Take the word from INLINK (leaving it +0) into KEYTEMP1, and "TURN ON
 * UPACT LIGHT" (bit 3 of channel 11). Then test it. */
void uprupt_after_lodsampt(void)
{
    set_A(ZERO);
    xch(ADDRESS_OF(INLINK));
    ts(ADDRESS_OF(KEYTEMP1));
    set_A(BIT3);
    wor_dsalmout();
    uprpt1();
}

/* UPRPT1 -- LM p. 1339, CM p. 1450.
 * "TEST FOR TRIPLE CHAR REDUNDANCY". Split the word: the low five bits
 * (the code) stay in KEYTEMP1; MP BIT10 shifts the rest right five
 * (KEYTEMP2), whose low five are the middle group and whose next five,
 * after another shift, the high group. UPTEST checks each against the
 * code, and comes back only when it agrees. */
static void uprpt1(void)
{
    set_A(LOW5);
    mask_at(ADDRESS_OF(KEYTEMP1));
    xch(ADDRESS_OF(KEYTEMP1)); /* "LOW5 INTO KEYTEMP1", the word into A */
    mp(BIT10);                 /* "SHIFT RIGHT 5" */
    ts(ADDRESS_OF(KEYTEMP2));
    mask(LOW5); /* "MID 5" */
    ad(HI10);   /* -037: with the code added, -0 if the middle is its complement */
    if (!uptest(UPRPT1 + 010u)) return;
    set_A(BIT10);
    mp_at(ADDRESS_OF(KEYTEMP2)); /* "SHIFT RIGHT 5" */
    mask(LOW5);                  /* "HIGH 5" */
    com();                       /* with the code added, -0 if it is the code */
    if (!uptest(UPRPT1 + 016u)) return;
    upok();
}

/* UPTEST -- LM pp. 1339-1340, CM pp. 1450-1451 (TC UPTEST at at).
 * Add the code to A: a group that agrees leaves -0, and CCS A takes the
 * fourth way on, TC Q, back to the caller (true). Any other sum is a
 * failure: TC TMFAIL2, which ends the interrupt (false). */
static bool uptest(word at)
{
    tc_at(at);
    ad_at(ADDRESS_OF(KEYTEMP1));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: tc_at(UPTEST + 2u); break;
    case IS_PLUS_ZERO:
        /* The second word on is the constant HI10, OCT 77740, which the AGC
         * runs as the instruction MASK 7740. It changes only A, and
         * TMFAIL2's CS overwrites A at once. */
        /* fall through */
    case NEGATIVE: tc_at(UPTEST + 4u); break;
    case IS_MINUS_ZERO: tc_through(Q_ADDRESS); return true; /* TC Q */
    }
    tmfail2();
    return false;
}

/* UPOK -- LM p. 1339, CM p. 1450. (This repository's Luminary099 file
 * spells the label UPCK; nothing refers to it.)
 * "CODE IS GOOD. IF CODE = 'ERROR RESET', CLEAR UPLOCKFL ... IF CODE DOES
 * NOT = 'ERROR RESET', ACCEPT CODE ONLY IF UPLOCKFL IS CLEAR (=0)." */
static void upok(void)
{
    set_A(negate(ELRCODE)); /* CS ELRCODE */
    ad_at(ADDRESS_OF(KEYTEMP1));
    if (A_is_zero()) { /* EXTEND; BZF CLUPLOCK */
        cluplock();
        return;
    }
#if LUMINARY099
    set_A(UPLOCBIT); /* "TEST UPLOCKFL FOR 0 OR 1" */
#else
    set_A(BIT4);
#endif
    mask_at(ADDRESS_OF(FLAGWRD7));
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* UPLOCKFL = 1 */
        tc_at(UPOK + 7u);                      /* TC RESUME */
        continue_at(RESUME);
        return;
    }
    tc_at(UPOK + 010u); /* UPLOCKFL = 0: TC ACCEPTUP */
    acceptup();
}

/* CLUPLOCK -- LM p. 1339, CM p. 1450.
 * "CLEAR UPLOCKFL (I.E., SET BIT 4 OF FLAGWRD7 = 0)", and accept the
 * ERROR RESET code as a key. */
static void cluplock(void)
{
#if LUMINARY099
    set_A(negate(UPLOCBIT)); /* CS UPLOCBIT */
#else
    set_A(negate(BIT4)); /* CS BIT4 */
#endif
    mask_at(ADDRESS_OF(FLAGWRD7));
    ts(ADDRESS_OF(FLAGWRD7));
    tc_at(CLUPLOCK + 3u); /* TC ACCEPTUP */
    acceptup();
}

/* TMFAIL2 -- LM p. 1339, CM p. 1450.
 * "CODE IS BAD": "LOCK OUT FURTHER UPLINK ACTIVITY (BY SETTING UPLOCKFL =
 * 1) UNTIL 'ERROR RESET' IS SENT VIA UPLINK." The ground sees the flag in
 * the downlinked flag words. */
static void tmfail2(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
#if LUMINARY099
    mask(UPLOCBIT);
#else
    mask(BIT4);
#endif
    ads(ADDRESS_OF(FLAGWRD7));
    tc_at(TMFAIL2 + 3u); /* TC RESUME */
    continue_at(RESUME);
}

/* The LM's file ends with one more word after ELRCODE, CS XDSPBIT (at
 * 04,3400), that no instruction or address reaches: there is nothing here
 * for it. */
