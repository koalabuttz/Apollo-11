/* SPDX-License-Identifier: MIT */
/* Sequence changing and subroutine calling: INTERPRETER.agc LM pp.
 * 1023-1025, CM pp. 1128-1130. The faithful C is c64-port/src/
 * lm_interp_core.c (lmi_goto_any, lmi_call, cgoto, lmi_swbranch, branch,
 * lmi_exit).
 *
 * "THE FOLLOWING OPERATIONS ARE AVAILABLE FOR SEQUENCING CHANGING,
 * BRANCHING, AND CALLING SUBROUTINES": GOTO, CALL (which leaves the return
 * in QPRET), CGOTO and CCALL (computed: an index into a list of CADRs), and
 * the branches on MPAC's sign. An interpretive program's addresses are
 * CADRs (bank and address in one word). A branch to a CADR in erasable
 * memory goes to the CADR held there. */
#include "interpreter.h"
#include "../agc/names.h"

static void gotoers(void);

/* (The AGC follows a chain of erasable CADRs for as long as it goes on. A
 * cycle would run until the night watchman restarted the computer. Here,
 * as in the faithful port, the job stops at GOTO after this many and the
 * next pass goes on, so the rest of the program keeps running.) */
#define GOTO_HOPS 63u

/* CCALL -- LM p. 1023, CM p. 1128.
 * A computed CALL: the operand (at ADDRWD) indexes the list of CADRs whose
 * address follows it. "MAINTAIN LOC FOR QPRET COMPUTATION". */
void ccall(void)
{
    incr(ADDRESS_OF(LOC));
    ca(indexed(0u, LOC));       /* "GET BASE ADDRESS OF CADR LIST." */
    ad_at(indexed(0u, ADDRWD)); /* "ADD INCREMENT." */
    ts(FBANK_ADDRESS);          /* "SELECT DESIRED CADR." */
    mask(LOW10);
    ca(indexed(02000u, a_word())); /* the listing's CA 10000, the bank's first word */
    ts(ADDRESS_OF(POLISH));
    call();
}

/* CALL -- LM p. 1023, CM p. 1128.
 * "FOR ANY OF THE CALL OPTIONS, MAKE UP THE ADDRESS OF THE NEXT OP-CODE
 * PAIR/STORE CODE AND LEAVE IT IN QPRET. NOTE THAT BANKMASK = -(2000 - 1)."
 * (RVQ returns there.) Then GOTO. */
void call(void)
{
    ca(ADDRESS_OF(BANKSET));
    mask(BANKMASK);
    ad(BANKMASK);
    ad_at(ADDRESS_OF(LOC));
    ts(indexed(ADDRESS_OF(QPRET), FIXLOC));
    goto_();
}

/* GOTO -- LM p. 1023, CM p. 1128.
 * "BASIC BRANCHING SEQUENCE": go to the CADR in POLISH. */
void goto_(void)
{
    ca(ADDRESS_OF(POLISH));
    goto_plus1();
}

/* GOTO +1: A holds the target. "SEE IF ADDRESS POINTS TO FIXED OR ERAS."
 * In erasable, GOTOERS fetches the CADR held there and tries again
 * ("ALLOWS ARBITRARY INDIRECTNESS LEVELS"). */
void goto_plus1(void)
{
    unsigned hops;

    for (hops = 0;; ++hops) {
        if (hops == GOTO_HOPS) {
            continue_at(GOTO);
            return;
        }
        mask(HIGH4);
        if (!A_is_zero()) break;
        gotoers();
    }
    goto_plus4();
}

/* GOTO +4: a CADR in fixed memory. "SET EBANK PART OF BBANK. NEXT, SET UP
 * FBANK. THE COMBINATION IS PICKED UP & PUT INTO BANKSET AT INTPRET +2."
 * LOC gets the address within the bank, and the new program starts. */
void goto_plus4(void)
{
    ca(ADDRESS_OF(BANKSET));
    ts(BBANK_ADDRESS);
    ca(ADDRESS_OF(POLISH));
    ts(FBANK_ADDRESS);
    mask(LOW10);
    ad(TWO_K);
    ts(ADDRESS_OF(LOC));
    intpret_plus3();
}

/* GOTOERS -- LM p. 1023, CM p. 1128.
 * "THE GIVEN ADDRESS IS IN ERASABLE -- SEE IF RELATIVE TO THE WORK AREA",
 * and "USE THE GIVEN ADDRESS AS THE ADDRESS OF THE BRANCH ADDRESS": in the
 * work area below 45, or (GOTOGE) in general erasable through EBANK.
 * (One level; GOTO +1 loops.) */
static void gotoers(void)
{
    ca(ADDRESS_OF(POLISH));
    ad(MINUS_ENDVAC);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        ca(ADDRESS_OF(POLISH)); /* "GENERAL ERASABLE." */
        /* fall through */
    case IS_PLUS_ZERO: /* GOTOGE */
        ts(EBANK_ADDRESS);
        mask(LOW8);
        ca(indexed(01400u, a_word())); /* CA 1400, under "EBANK= 1400" */
        break;
    case NEGATIVE:
        ca(ADDRESS_OF(FIXLOC)); /* "WORK AREA." */
        /* fall through */
    case IS_MINUS_ZERO:
        ad_at(ADDRESS_OF(POLISH));
        ca(indexed(0u, a_word()));
        break;
    }
    ts(ADDRESS_OF(POLISH));
}

/* CGOTO -- LM p. 1024, CM p. 1129.
 * "COMPUTED GO TO. PICK UP ADDRESS OF CADR LIST" (the word after the
 * operand), "ADD MODIFIER" (the operand) and go to the CADR selected. */
void cgoto(void)
{
    ca(indexed(1u, LOC));
    ad_at(indexed(0u, ADDRWD));
    ts(FBANK_ADDRESS); /* "SELECT GOTO ADDRESS" */
    mask(LOW10);
    ca(indexed(02000u, a_word())); /* CA 10000 */
    ts(ADDRESS_OF(POLISH));
    goto_plus1(); /* "WITH ADDRESS IN A." */
}

/* SWBRANCH -- LM p. 1024, CM p. 1129.
 * "SWITCH INSTRUCTIONS WHICH ELECT TO BRANCH COME HERE TO DO SO": the
 * branch address is the word after the switch order's. */
void swbranch(void)
{
    ca(ADDRESS_OF(BANKSET));
    ts(FBANK_ADDRESS);
    ca(indexed(1u, LOC));
    ts(ADDRESS_OF(POLISH));
    goto_plus1();
}

/* BRANCH -- LM p. 1025, CM p. 1130.
 * "TRIPLE PRECISION BRANCHING ROUTINE. IF CALLING TC IS AT L, RETURN IS AS
 * FOLLOWS: L+1 IF MPAC IS GREATER THAN ZERO. L+2 IF MPAC IS EQUAL TO +0 OR
 * -0. L+3 IF MPAC IS LESS THAN ZERO." The sign is that of the first
 * nonzero register (CCS MPAC, MPAC +1, MPAC +2 in turn). */
static enum branch branch_from(unsigned k)
{
    for (; k < 3u; ++k) {
        switch (ccs_at(ADDRESS_OF(MPAC) + k)) {
        case POSITIVE: tc_through(Q_ADDRESS); return BRANCH_POSITIVE;
        case NEGATIVE: neg(); return BRANCH_NEGATIVE;
        default: break; /* +0 or -0: the next register */
        }
    }
    q_plus_1();
    return BRANCH_ZERO;
}

enum branch branch(void)
{
    return branch_from(0u);
}

/* BRANCH +4: from the test of MPAC +1. */
void branch_plus4(void)
{
    (void)branch_from(1u);
}

/* Q+1 and NEG (Q+2) -- LM p. 1025, CM p. 1130.
 * INDEX Q; TC 1 and INDEX Q; TC 2: return one or two words past the
 * caller's return address. (Other routines use them so too.) */
void q_plus_1(void)
{
    word to = indexed(1u, overflow_corrected(Q));
    tc_at(Q_PLUS_1 + 1u);
    continue_at(to);
}

void neg(void)
{
    word to = indexed(2u, overflow_corrected(Q));
    tc_at(NEG + 1u);
    continue_at(to);
}

/* NEG +1: TC 2 without the INDEX is TC Q, the plain return. */
void neg_plus1(void)
{
    tc_through(Q_ADDRESS);
}

/* EXIT -- LM p. 1025, CM p. 1130.
 * "RESTORE USER'S BANK SETTING, AND LEAVE INTERPRETIVE MODE": basic code
 * resumes at the word after the EXIT (INDEX LOC; TC 1). */
void exit_(void)
{
    word to;

    ca(ADDRESS_OF(BANKSET));
    ts(BBANK_ADDRESS);
    to = indexed(1u, LOC);
    tc_at(EXIT + 3u);
    continue_at(to);
}
