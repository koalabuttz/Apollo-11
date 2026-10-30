/* SPDX-License-Identifier: MIT */
/* Inter-bank communication: INTER-BANK_COMMUNICATION.agc of Luminary099
 * (LM, listing pages 998-1001) and Comanche055 (CM, pages 1103-1106). The
 * faithful C is c64-port/src/lm_interbank.c and cm_interbank_complete.c.
 *
 * The two listings are the same code at different addresses. (IBNKCALL
 * reads its CADR with CAF 0 in the LM and CA 0 in the CM: the same
 * instruction word, 030000.)
 *
 * Every word of memory and every register ends as the original leaves it
 * (STYLE.md, Fidelity), so the routines below keep the original's order of
 * exchanges: A, L and Q carry the caller's values through each one, and
 * the exchanges are how they are kept. */
#include "interbank.h"
#include "../agc/names.h"

static void call_in_bank(unsigned buffer, word return_entry);
static void return_to_caller(unsigned buffer);
static void prio12_as_ccs(void);

/* INDEX Q; TC 10000 (or TCF 10000): TC 10000 assembles to TC 02000, the
 * first word of the bank FBANK selects, and INDEX Q adds the address within
 * the bank that Q holds. */
static word in_bank(word relative)
{
    return (word)(02000u + relative);
}

/* ======================================================================
 * Calls that return
 * ====================================================================== */

/* BANKCALL -- LM p. 998, CM p. 1103.
 * Call a subroutine in another bank. Its CADR follows the TC BANKCALL; A
 * and L reach the subroutine unchanged. The subroutine's RETURN comes back
 * through SWRETURN, which puts the caller's bank back and continues after
 * the CADR. BUF2 holds the return meanwhile, so a subroutine reached this
 * way must not BANKCALL in turn without saving it. */
void bankcall(void)
{
    dxch(ADDRESS_OF(BUF2)); /* "SAVE INCOMING A,L." */
    set_A(read_word(Q));    /* INDEX Q; CA 0: "PICK UP CADR." */
    Q = add16(Q, 1u);       /* INCR Q: "SO WE RETURN TO THE LOC. AFTER THE CADR." */
    call_in_bank(ADDRESS_OF(BUF2), SWRETURN);
}

/* SWCALL -1 -- LM p. 998, CM p. 1103.
 * BANKCALL's INCR Q, entered on its own: the interpreter's RTB does TC
 * SWCALL -1 so that the call returns one word further on. */
void swcall_minus1(void)
{
    Q = add16(Q, 1u);
    continue_at(SWCALL);
}

/* SWCALL -- LM p. 998, CM p. 1103.
 * "SWCALL IS IDENTICAL TO BANKCALL, EXCEPT THAT THE CADR ARRIVES IN A."
 * Here A and L do not reach the subroutine: the DXCH that gives a
 * BANKCALL's subroutine its A and L back gives it whatever BUF2 held. */
void swcall(void)
{
    call_in_bank(ADDRESS_OF(BUF2), SWRETURN);
}

/* SWCALL and ISWCALL, from TS L on: the CADR is in A. buffer is the pair
 * that keeps the return (BUF2, or RUPTREG3 for ISWCALL) and return_entry
 * the routine the call returns through (SWRETURN or ISWRETRN), the word
 * after the TC that makes the call. */
static void call_in_bank(unsigned buffer, word return_entry)
{
    word subroutine;

    if (!ts(L_ADDRESS))      /* the CADR, kept in L */
        lxch(FBANK_ADDRESS); /* "SWITCH BANKS, SAVING RETURN." */
    /* An overflow in A (a CADR is never one) makes the TS skip the LXCH:
     * FBANK stays, and the call goes to address 1 or 01776 of the bank. */
    mask(LOW10);    /* "GET SUB-ADDRESS OF CADR." */
    xch(Q_ADDRESS); /* "A,L NOW CONTAINS DP RETURN." */
    dxch(buffer);   /* "RESTORING INPUTS IF THIS IS A BANKCALL." */
    /* INDEX Q; TC 10000: "SETTING Q TO SWRETURN." */
    subroutine = in_bank(Q);
    tc_at((word)(return_entry - 1u));
    continue_at(subroutine);
}

/* SWRETURN -- LM p. 998, CM p. 1103.
 * "COMES HERE TO RETURN TO CALLER. C(A,L) ARE PRESERVED FOR RETURN." */
void swreturn(void)
{
    return_to_caller(ADDRESS_OF(BUF2));
}

/* SWRETURN and ISWRETRN: buffer holds the return address, then the
 * caller's FBANK. */
static void return_to_caller(unsigned buffer)
{
    xch(buffer + 1u);   /* the caller's FBANK to A */
    xch(FBANK_ADDRESS); /* ... and to FBANK */
    xch(buffer + 1u);   /* the subroutine's A back */
    /* TC BUF2: BUF2 holds the return address, and the AGC runs it as the
     * instruction TC <return address>. */
    tc_through(buffer);
}

/* ======================================================================
 * Jumps
 * ====================================================================== */

/* POSTJUMP -- LM p. 998, CM p. 1103.
 * "A UNILATERAL JUMP WITH C(A,L) PRESERVED AND THE CADR IMMEDIATELY
 * FOLLOWING THE TC POSTJUMP INSTRUCTION." */
void postjump(void)
{
    xch(Q_ADDRESS);      /* "SAVE INCOMING C(A)." */
    set_A(read_word(A)); /* INDEX A; CA 0: "GET CADR." */
    bankjump();
}

/* BANKJUMP -- LM p. 998, CM p. 1103.
 * "BANKJUMP IS THE SAME AS POSTJUMP, EXCEPT THAT THE CADR ARRIVES IN A."
 * A gets what Q held (for a POSTJUMP, the caller's A), and Q is left with
 * the address within the bank. */
void bankjump(void)
{
    if (!ts(FBANK_ADDRESS)) /* the CADR's bank to FBANK */
        mask(LOW10);
    /* With an overflow in A, the TS skips the MASK and leaves +1 or -1 in
     * A, which the XCH puts in Q (see Q+10000). */
    xch(Q_ADDRESS); /* "RESTORING INPUT C(A) IF THIS WAS A POSTJUMP." */
    q_plus_10000();
}

/* Q+10000 -- LM p. 998, CM p. 1103.
 * INDEX Q; TCF 10000: continue at the address within the bank that Q
 * holds. The instruction word TCF 10000 is labelled PRIO12: as a number
 * it is 012000, job priority 12, and other routines (ENGINOFF, GOPOST,
 * the fresh start) load it as that ("PRIO12 = TCF 10000 = 12000"). */
void q_plus_10000(void)
{
    if (Q == 0177776u) { /* -1: see prio12_as_ccs */
        prio12_as_ccs();
        return;
    }
    correct_L();
    continue_at(in_bank(Q));
}

/* BANKJUMP with a negative overflow in A leaves -1 in Q, and INDEX Q then
 * makes PRIO12's TCF 10000 (012000) the instruction 011777: CCS 01777, a
 * CCS of the last word of the EBANK window. Its four outcomes run on into
 * MAKECADR's four instructions (CAF, MASK, AD, TC Q) and so to a TC Q with
 * -1 in Q, which runs register Q as an instruction. No caller passes such a
 * CADR. The faithful port stops at that TC Q, with Z at register Q, and so
 * does this. */
static void prio12_as_ccs(void)
{
    switch (ccs(fetch(01777u))) {
    case POSITIVE: set_A(LOW10);      /* fall through */
    case IS_PLUS_ZERO: mask(BUF2(0)); /* fall through */
    case NEGATIVE:
        A = add16(A, sign_extend(BUF2(1)));
        correct_L();
        break;
    case IS_MINUS_ZERO: break;
    }
    continue_at(Q_ADDRESS);
}

/* ======================================================================
 * The return address as a CADR
 * ====================================================================== */

/* MAKECADR -- LM p. 999, CM p. 1104.
 * "GETS THE RETURN CADR SAVED BY SWCALL OR BANKCALL AND LEAVES IT IN A":
 * the return address's place within its bank plus the bank (BUF2 +1). */
void makecadr(void)
{
    set_A(LOW10); /* CAF LOW10 */
    mask(BUF2(0));
    A = add16(A, sign_extend(BUF2(1)));
    correct_L();
    tc_through(Q_ADDRESS); /* TC Q */
}

/* ======================================================================
 * Data from another bank
 * ====================================================================== */

/* SUPDACAL -- LM p. 999, CM p. 1104.
 * Read one word of fixed memory in any bank: A is the word's CADR and L
 * holds the superbank bits for it (bits 7-5). The word comes back in A and
 * in MPTEMP, its address within the bank in L; FBANK and the superbank are
 * put back. "24 WDS. DATACALL 516 MU, SUPDACAL 432 MU" */
void supdacal(void)
{
    if (!ts(ADDRESS_OF(MPTEMP))) xch(FBANK_ADDRESS); /* "SET FBANK FOR DATA." */
    /* An overflow in A skips the XCH: the data comes from the caller's own
     * bank, and the +1 or -1 left in A becomes FBANK at the end. */
    ror_superbnk();          /* "SAVE FBANK IN BITS 15-11, AND */
    xch(ADDRESS_OF(MPTEMP)); /*  SUPERBANK IN BITS 7-5." */
    mask(LOW10);
    xch(L_ADDRESS);   /* "SAVE REL. ADR. IN BANK, FETCH SUPERBITS." */
    inhint();         /* "BECAUSE RUPT DOES NOT SAVE SUPERBANK." */
    write_superbnk(); /* "SET SUPERBANK FOR DATA." */
    /* INDEX L; CA 10000: "PINBALL (FIX MEM DISP) PREVENTS DCA HERE" */
    set_A(fetch_fixed(in_bank((word)L)));
    xch(ADDRESS_OF(MPTEMP)); /* "SAVE 1ST WD, FETCH OLD FBANK AND SBANK." */
    write_superbnk();        /* "RESTORE SUPERBANK." */
    relint();
    ts(FBANK_ADDRESS);     /* "RESTORE FBANK." */
    set_A(MPTEMP);         /* "RECOVER FIRST WORD OF DATA." */
    tc_through(Q_ADDRESS); /* RETURN */
}

/* ======================================================================
 * The interrupt routines' calls
 * ====================================================================== */

/* IBNKCALL -- LM p. 1000, CM p. 1105.
 * BANKCALL for interrupt routines: the same, with the return kept in
 * RUPTREG3 and RUPTREG4 ("USES RUPTREG3,4 FOR DP RETURN ADDRESS"), so that
 * an interrupt does not disturb a BANKCALL's BUF2. */
void ibnkcall(void)
{
    dxch(ADDRESS_OF(RUPTREG3));
    set_A(read_word(Q)); /* INDEX Q; CAF 0 (CA 0 in the CM) */
    Q = add16(Q, 1u);    /* INCR Q */
    call_in_bank(ADDRESS_OF(RUPTREG3), ISWRETRN);
}

/* ISWCALL -- LM p. 1000, CM p. 1105.
 * SWCALL for interrupt routines (ISWCALLL in the LM listing's text). */
void iswcall(void)
{
    call_in_bank(ADDRESS_OF(RUPTREG3), ISWRETRN);
}

/* ISWRETRN -- LM p. 1000, CM p. 1105.
 * SWRETURN for interrupt routines. */
void iswretrn(void)
{
    return_to_caller(ADDRESS_OF(RUPTREG3));
}

/* ======================================================================
 * Into the interpreter
 * ====================================================================== */

/* USPRCADR -- LM p. 1000, CM p. 1105.
 * Run a piece of interpretive code in another bank, from basic code: the
 * interpretive code's CADR follows the TC USPRCADR. Sets up the job's
 * interpretive state (LOC, BANKSET and EDOP) so that the interpretive code
 * can EXIT back to the instruction after the CADR, and jumps in. */
void usprcadr(void)
{
    if (!ts(ADDRESS_OF(LOC))) /* "SAVE A" */
        set_A(BIT8);
    ts(ADDRESS_OF(EDOP)); /* "EXIT INSTRUCTION TO EDOP" */
    set_A(BBANK);
    ts(ADDRESS_OF(BANKSET)); /* "USER'S BBANK TO BANKSET" */
    set_A(read_word(Q));     /* INDEX Q; CA 0 */
    ts(FBANK_ADDRESS);       /* "INTERPRETIVE BANK TO FBANK" */
    mask(LOW10);             /* "YIELDS INTERPRETIVE RELATIVE ADDRESS" */
    xch(Q_ADDRESS);          /* "INTERPRETIVE ADDRESS TO Q, FETCHING L+1" */
    xch(ADDRESS_OF(LOC));    /* "L+1 TO LOC, RETRIEVING ORIGINAL A" */
    q_plus_10000();          /* TCF Q+10000 */
}

/* ======================================================================
 * The superbank
 * ====================================================================== */

/* SUPERSW -- LM p. 1001, CM p. 1106.
 * "WRITE BITS 7-6-5 OF THE ACCUMULATOR INTO CHANNEL 07", which select
 * which banks the addresses of banks 030-037 reach, and return. */
void supersw(void)
{
    write_superbnk();
    tc_through(Q_ADDRESS); /* TC Q */
}
