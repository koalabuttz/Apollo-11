/* SPDX-License-Identifier: MIT */
/* Service routines: SERVICE_ROUTINES.agc of Luminary099 (LM, listing pages
 * 1374-1380) and Comanche055 (CM, pages 1485-1492). The faithful C is
 * c64-port/src/service_routines.c, with service_transfer.c.
 *
 * The two missions share UPFLAG, DOWNFLAG, DELAYJOB, GENTRAN and B5OFF.
 * UPENT2 and DOWNENT2 differ: the LM's work on FLAGWRD4 only, the CM's on
 * the flag word the low three bits of A name. Only the CM has TRFAILOF and
 * TRFAILON.
 *
 * The binding (src/binding/service_routines.c) hands these routines the
 * states the original's callers make: return addresses and data words, so
 * no TS here stores an overflow and skips unless the code says so. */
#include "service.h"
#include "../interbank/interbank.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

static void ok2delay(void);
static void b5off_plus1(void);
#if COMANCHE055
static void req(void);
#endif

/* INDEX Q; <op> n: the word n after the return address Q holds, as an
 * address (INDEX adds Q as a word, overflow-corrected). */
static unsigned past_q(word n)
{
    return indexed(n, overflow_corrected(Q));
}

/* ======================================================================
 * Several flags of one flag word
 * ====================================================================== */

#if LUMINARY099
static void join(void);

/* UPENT2 -- LM p. 1374.
 * Set the bits of A (all but the low three) in FLAGWRD4. The caller's TC
 * is followed by a word the caller uses for itself (WAITMASK, the mask it
 * built A with), so the return is one word later. */
void upent2(void)
{
    inhint();
    mask(OCT77770);
    ts(L_ADDRESS);
    cs(ADDRESS_OF(FLAGWRD4));
    mask_at(L_ADDRESS);        /* the bits not yet set */
    ads(ADDRESS_OF(FLAGWRD4)); /* ... added in: now set */
    join();
}

/* DOWNENT2 -- LM p. 1374.
 * Clear the bits of A (all but the low three) in FLAGWRD4. */
void downent2(void)
{
    inhint();
    mask(OCT77770);
    com();
    mask_at(ADDRESS_OF(FLAGWRD4));
    ts(ADDRESS_OF(FLAGWRD4));
    join();
}

/* JOIN -- LM p. 1374.
 * TCF Q+1: back past the word after the caller's TC. */
static void join(void)
{
    relint();
    q_plus_1();
}

#else

/* UPENT2 -- CM p. 1485.
 * A names the flag word in its low three bits ("WHICH FLAGWORD IS IT") and
 * the bits to set in the rest. The caller's TC is followed by a word the
 * caller uses for itself (WAITMASK, the mask it built A with), so the
 * return is one word later: INCR Q; TC Q. */
void upent2(void)
{
    if (!ts(L_ADDRESS)) mask(OCT7); /* TS skips the MASK if A overflowed */
    xch(L_ADDRESS);                 /* "SAVE IN L FOR INDEXING" */
    mask(OCT77770);                 /* "OBTAIN THE BIT INFORMATION" */
    inhint();
    ts(ADDRESS_OF(ITEMP1));
    cs(indexed(ADDRESS_OF(FLAGWRD0), overflow_corrected(L)));
    mask_at(ADDRESS_OF(ITEMP1)); /* the bits not yet set */
    ads(indexed(ADDRESS_OF(FLAGWRD0), overflow_corrected(L)));
    relint();
    incr(Q_ADDRESS); /* "OBTAIN THE CORRECT RETURN ADDRESS" */
    tc_through(Q_ADDRESS);
}

/* DOWNENT2 -- CM p. 1485.
 * As UPENT2, clearing the bits. */
void downent2(void)
{
    if (!ts(L_ADDRESS)) mask(OCT7);
    xch(L_ADDRESS);
    mask(OCT77770);
    com();
    inhint();
    mask_at(indexed(ADDRESS_OF(FLAGWRD0), overflow_corrected(L)));
    ts(indexed(ADDRESS_OF(FLAGWRD0), overflow_corrected(L)));
    relint();
    incr(Q_ADDRESS);
    tc_through(Q_ADDRESS);
}
#endif

/* ======================================================================
 * One named flag
 * ====================================================================== */

/* UPFLAG -- LM p. 1375, CM p. 1486.
 * "UPFLAG AND DOWNFLAG ARE ENTIRELY GENERAL FLAG SETTING AND CLEARING
 * SUBROUTINES." The calling sequence is
 *     TC     UPFLAG
 *     ADRES  NAME OF FLAG
 * "RETURN IS TO THE LOCATION FOLLOWING THE "ADRES" ... UPON RETURN A
 * CONTAINS THE CURRENT FLAGWRD SETTING." The flag's name is its number,
 * 15 per flag word: flag word n's bit 15 is flag 15n, its bit 1 flag
 * 15n + 14. DEBIT works out which word and bit. */
void upflag(void)
{
    ca(Q_ADDRESS);
    tc_at(UPFLAG + 1u); /* TC DEBIT */
    debit();
    com();       /* "+(15 - BIT)" */
    ror_lchan(); /* EXTEND; ROR LCHAN: "SET BIT" */
    comflag();
}

/* COMFLAG -- LM p. 1375, CM p. 1486.
 * Store the flag word back and return past the ADRES: ITEMP3 holds that
 * address, and LXCH puts it in L for TC L. (A TS that overflowed would
 * skip the LXCH; the LM's INVFLAG comes here with its own A.) */
void comflag(void)
{
    if (!ts(indexed(ADDRESS_OF(FLAGWRD0), ITEMP1))) lxch(ADDRESS_OF(ITEMP3));
    relint();
    tc_through(L_ADDRESS); /* TC L */
}

/* DOWNFLAG -- LM p. 1375, CM p. 1486.
 * As UPFLAG, clearing the bit: DEBIT leaves the bit's complement in A. */
void downflag(void)
{
    ca(Q_ADDRESS);
    tc_at(DOWNFLAG + 1u); /* TC DEBIT */
    debit();
    mask_at(L_ADDRESS); /* "RESET BIT" */
    comflag();
}

/* DEBIT -- LM pp. 1375-1376, CM pp. 1486-1487.
 * A holds the address of the caller's ADRES. Divide the flag's number by
 * 15: the quotient is the flag word (ITEMP1), the remainder which bit
 * from the top (ITEMP2). Return with the flag word in L and the bit,
 * negated, in A: "-(15 - BIT)". ITEMP3 keeps the address after the
 * ADRES, for COMFLAG. */
void debit(void)
{
    ad(ONE);
    inhint();
    ts(ADDRESS_OF(ITEMP3));
    set_A(LOW4); /* "DEC15" */
    ts(ADDRESS_OF(ITEMP1));
    ca(indexed(ITEMP3, NEGONE)); /* INDEX ITEMP3; CA 0 -1: "ADRES" */
    ts(L_ADDRESS);
    set_A(ZERO);
    dv_at(ADDRESS_OF(ITEMP1)); /* "A = FLAGWRD, L = (15 - BIT)" */
    dxch(ADDRESS_OF(ITEMP1));
    ca(indexed(ADDRESS_OF(FLAGWRD0), ITEMP1));
    ts(L_ADDRESS);                      /* "CURRENT STATE" */
    cs(indexed(BIT15_ADDRESS, ITEMP2)); /* BIT15, BIT14, ... BIT1 */
    tc_through(Q_ADDRESS);              /* TC Q */
}

/* ======================================================================
 * Putting a job to sleep for a while
 * ====================================================================== */

/* DELAYJOB -- LM p. 1377, CM p. 1488.
 * "A GENERAL ROUTINE TO DELAY A JOB A SPECIFIC AMOUNT OF TIME BEFORE
 * PICKING UP AGAIN":
 *     CAF    DT          "DELAY JOB FOR DT CENTISECS"
 *     TC     BANKCALL
 *     CADR   DELAYJOB
 * Find a free DELAYLOC slot (+0), last first, and have WAITLIST call
 * WAKER after DT. "THIS MUST REMAIN IN BANK 0": the slot's number goes to
 * the task as its BBANK, which in bank 0 is just its EBANK, and WAKER
 * reads it back from there. The loop's test is a CCS of the slot number:
 *     CCS   RUPTREG1
 *     TCF   DELLOOP        positive: "TRY NEXT DELAYLOC"
 *     ...                  +0: no slot free, abort (1104)
 * after which a negative number (never one DELAYJOB makes) runs into the
 * TC BAILOUT1 (LM) or the OCT 1104 (CM, as the instruction TC 1104), and
 * -0 one word further. */
void delayjob(void)
{
    inhint();
    if (!ts(Q_ADDRESS))  /* "STORE DELAY DT IN Q FOR DLY -1 IN WAITLIST" */
        set_A(DELAYNUM); /* (an overflowed DT leaves +1 or -1 instead) */
    for (;;) {
        ts(ADDRESS_OF(RUPTREG1)); /* DELLOOP */
        ca(indexed(ADDRESS_OF(DELAYLOC), a_word()));
        if (A_is_zero()) { /* EXTEND; BZF OK2DELAY: "IS THIS DELAYLOC AVAILABLE" */
            ok2delay();
            return;
        }
        switch (ccs_at(ADDRESS_OF(RUPTREG1))) {
        case POSITIVE: continue; /* "NO, TRY NEXT DELAYLOC" */
#if LUMINARY099
        case IS_PLUS_ZERO: /* "NO AVAILABLE LOCS." */
            dxch(ADDRESS_OF(BUF2));
            /* fall through */
        case NEGATIVE:
            tc_at(OK2DELAY - 2u); /* TC BAILOUT1; OCT 1104 */
            continue_at(BAILOUT1);
            return;
        case IS_MINUS_ZERO:
            tc_at(OK2DELAY - 1u); /* OCT 1104, run as TC 1104 */
            continue_at(01104u);
            return;
#else
        case IS_PLUS_ZERO:
            tc_at(OK2DELAY - 2u); /* TC BAILOUT; OCT 1104: "NO AVAILABLE LOCS AVAILABLE." */
            continue_at(BAILOUT);
            return;
        case NEGATIVE:
            tc_at(OK2DELAY - 1u); /* OCT 1104, run as TC 1104 */
            continue_at(01104u);
            return;
        case IS_MINUS_ZERO: ok2delay(); return;
#endif
        }
    }
}

/* OK2DELAY -- LM p. 1377, CM p. 1488.
 * Call WAITLIST at DLY2 -1, with DT in Q, the task WAKER in A and its
 * BBANK (bank 0, EBANK the slot number) in L. WAITLIST returns two words
 * past WAITEXIT (past a caller's 2CADR), so TCSLEEP, TCGETCAD -2, brings it
 * back to TCGETCAD: "SET WAITLIST IMMEDIATE RETURN". */
static void ok2delay(void)
{
    set_A(TCSLEEP);
    ts(ADDRESS_OF(WAITEXIT));
    ca(FBANK_ADDRESS);
    ad(RUPTREG1); /* "STORE BBANK FOR TASK CALL" */
    ts(L_ADDRESS);
    set_A(WAKECAD);                 /* "STORE CADR FOR TASK CALL" */
    continue_at((word)(DLY2 - 1u)); /* "DLY IS IN WAITLIST ROUTINE" */
}

/* TCGETCAD -- LM p. 1377, CM p. 1488.
 * Back from WAITLIST: keep the job's return address (MAKECADR, from BUF2,
 * where BANKCALL left it) in its slot and put the job to sleep. JOBSLEEP
 * ends the job's turn; WAKER will wake it with that address. (A CADR that
 * overflowed TS would skip the TC JOBSLEEP into WAKER.) */
void tcgetcad(void)
{
    tc_at(TCGETCAD); /* TC MAKECADR: "GET CALLERS FCADR" */
    makecadr();
    if (ts(indexed(ADDRESS_OF(DELAYLOC), RUPTREG1))) { /* "SAVE DELAY CADRS" */
        waker();
        return;
    }
    tc_at(TCGETCAD + 3u); /* TC JOBSLEEP */
    continue_at(JOBSLEEP);
}

/* WAKER -- LM pp. 1377-1378, CM pp. 1488-1489.
 * The WAITLIST task, DT later. Its BBANK is the slot number: take the
 * sleeping job's address out of that slot, which frees it ("MAKE DELAYLOC
 * AVAILABLE"), and wake the job. */
void waker(void)
{
    set_A(ZERO);
    xch(indexed(ADDRESS_OF(DELAYLOC), BBANK));
    tc_at(WAKER + 3u); /* TC JOBWAKE */
    continue_at(JOBWAKE);
}

/* WAKER +4 -- LM p. 1378, CM p. 1489.
 * Back from JOBWAKE: the task is over. */
void waker_after_jobwake(void)
{
    tc_at(WAKER + 4u); /* TC TASKOVER */
    continue_at(TASKOVER);
}

/* ======================================================================
 * Copying a block
 * ====================================================================== */

/* GENTRAN -- LM p. 1379, CM p. 1490.
 * "A BLOCK TRANSFER ROUTINE", by D. Eyles: copy N words, erasable or
 * fixed, to N erasable words.
 *     CA     N-1        "# OF QUANTITIES MINUS ONE."
 *     TC     GENTRAN
 *     ADRES  L          "STARTING ADRES OF DATA TO BE MOVED."
 *     ADRES  M          "STARTING ADRES OF DUPLICATION BLOCK."
 *                       "RETURNS HERE."
 * "A, L, AND ITEMP1 ARE NOT PRESERVED." Interrupts stay inhibited until
 * the caller allows them ("MOD 2 BY SCHULENBERG (REMOVE RELINT)"). */
void gentran(void)
{
    inhint();
    gentran_plus1();
}

/* GENTRAN +1 -- LM p. 1379, CM p. 1490.
 * One word, the last not yet copied, L + N-1 to M + N-1. CCS ITEMP1 then
 * loops while N-1 is positive (A is N-2) and returns past the two ADRES
 * words once it is +0 (TCF Q+2). A negative ITEMP1, which only a copy
 * onto ITEMP1 itself could leave, runs on into B5OFF. */
void gentran_plus1(void)
{
    ts(ADDRESS_OF(ITEMP1)); /* "SAVE N-1." */
    ad_at(past_q(0u));      /* "ADRES (L + N - 1)." */
    ca(a_word());           /* INDEX A; CA 0: "C(ABOVE)." */
    ts(L_ADDRESS);          /* "SAVE DATA." */
    ca(ADDRESS_OF(ITEMP1));
    ad_at(past_q(1u)); /* "ADRES (M + N - 1)." */
    lxch(a_word());    /* INDEX A; LXCH 0: "STUFF IT." */
    switch (ccs_at(ADDRESS_OF(ITEMP1))) {
    case POSITIVE: continue_at((word)(GENTRAN + 1u)); return; /* "LOOP UNTIL N-1 = 0." */
    case IS_PLUS_ZERO: neg(); return; /* TCF Q+2 (NEG): "RETURN TO CALLER." */
    case NEGATIVE: b5off(); return;
    case IS_MINUS_ZERO: b5off_plus1(); return;
    }
}

/* ======================================================================
 * Ending an extended verb
 * ====================================================================== */

/* B5OFF -- LM p. 1380, CM p. 1491.
 * "ZERO BIT 5 OF EXTVBACT, WHICH IS SET BY TESTXACT. MAY BE USED AS
 * NEEDED BY ANY EXTENDED VERB WHICH HAS DONE TESTXACT", and end the job. */
void b5off(void)
{
    set_A(negate(BIT5));
    b5off_plus1();
}

static void b5off_plus1(void)
{
    mask_at(ADDRESS_OF(EXTVBACT));
    ts(ADDRESS_OF(EXTVBACT));
    tc_at(B5OFF + 3u); /* TC ENDOFJOB */
    continue_at(ENDOFJOB);
}

/* ======================================================================
 * The CM's TRACKER light
 * ====================================================================== */

#if COMANCHE055
/* The TRACKER light is bit 8 of DSPTAB +11D (with bit 15, which marks the
 * word for sending to the DSKY). */
#define TRACKER_LIGHT_WORD (ADDRESS_OF(DSPTAB) + 11u) /* DSPTAB +11D */

/* TRFAILOF -- CM p. 1492.
 * "SUBROUTINES TO TURN OFF AND TURN ON TRACKER FAIL LIGHT." Off: clear bit
 * 8, set bit 15. And set bit 7 of OPTMODES, "TO INSURE THAT OCDU FAIL WILL
 * GO ON AGAIN IF IT WAS ON IN ADDITION TO TRACKER FAIL." */
void trfailof(void)
{
    inhint();
    set_A(negate(OCT40200)); /* "TURN OFF TRACKER LIGHT" */
    mask_at(TRACKER_LIGHT_WORD);
    ad(BIT15);
    ts(TRACKER_LIGHT_WORD);
    cs(ADDRESS_OF(OPTMODES));
    mask(BIT7);
    ads(ADDRESS_OF(OPTMODES));
    req();
}

/* REQ -- CM p. 1492. */
static void req(void)
{
    relint();
    tc_through(Q_ADDRESS); /* TC Q */
}

/* TRFAILON -- CM p. 1492.
 * On: set bits 8 and 15, those not already set. */
void trfailon(void)
{
    inhint();
    cs(TRACKER_LIGHT_WORD); /* "TURN ON" */
    mask(OCT40200);
    ads(TRACKER_LIGHT_WORD);
    req();
}
#endif
