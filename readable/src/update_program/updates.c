/* SPDX-License-Identifier: MIT */
/* UPDATE_PROGRAM.agc, part 4: the updates themselves, V71 (UPEND71), V72
 * (UPEND72) and V70 (UPEND70); LM pages 1393-1396, CM pages 1504-1507. The
 * faithful C is c64-port/src/lm_update.c and cm_update.c. */
#include "update_program.h"

#define CA_CODE   030000u
#define LXCH_CODE 022000u

/* INDEX A; CA k (CAE k): the word at k + A. Not modelled: an index that
 * makes the CA any other instruction, or one of fixed memory. */
static void ca_indexed(word k)
{
    word instruction = indexed_instruction((word)(CA_CODE | k), a_word());

    if (OPERATION_CODE(instruction) != 3u || ADDRESS_FIELD(instruction) >= 02000u) not_modelled();
    ca(ADDRESS_FIELD(instruction));
}

/* INDEX A; LXCH 1400: "UPDATE THE REGISTER BY CONTENTS OF L", the word A
 * words into the bank EBANK selects. Not modelled: an index that makes
 * the LXCH any other instruction, or an update of the central registers or
 * the counters (EBANK 0, below 061), which the faithful port does not
 * write. */
static void lxch_indexed(void)
{
    word instruction = indexed_instruction((word)(LXCH_CODE | 01400u), a_word());
    unsigned address = instruction & 01777u;

    if ((instruction & 076000u) != LXCH_CODE || erasable_index(address) < 061u) not_modelled();
    lxch(address);
}

/* UPEND71 -- LM p. 1393, CM p. 1504, V71: "SET EBANK AND CALCULATE S-REG
 * VALUE OF RECEIVING AREA" (in UPTEMP) from the ECADR in UPBUFF +1. "IN
 * THE PROCESS OF PERFORMING THIS UPDATE WILL WE OVERFLOW INTO THE NEXT
 * EBANK...." (S + II - 3 past 0377: bit 9): "YES", UPERROUT. "NO-
 * CALCULATE NUMBER OF WORDS TO BE STORED MINUS ONE" (II - 3), and
 * STORLP71: from the last word down, "TAKE NEXT UPDATE WORD FROM UPBUFF
 * AND SAVE IT IN L", "CALCULATE NEXT RECEIVING ADDRESS", and store it, as
 * long as "THERE ANY WORDS LEFT TO BE STORED". Then "EXIT UPDATE
 * PROGRAM". A negative count would run ADUPBFM1's word, ADRES UPBUFF -1,
 * as an instruction: not modelled. A -0 one takes the TCF UPOUT after
 * it. */
void upend71(void)
{
    ca(ADDRESS_OF(UPBUFF) + 1u);
    ts(EBANK_ADDRESS);
    mask(LOW8);
    ts(ADDRESS_OF(UPTEMP));
    ad(NEG3);
    ad_at(ADDRESS_OF(COMPNUMB));
    if (!A_is_zero()) { /* BZF STORLP71: "HERE WHEN COMPNUMB = 3" */
        mask(BIT9);
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: uperrout(); return;
        case IS_PLUS_ZERO: set_A(NEG3); /* fall through */
        case NEGATIVE: ad_at(ADDRESS_OF(COMPNUMB)); break;
        case IS_MINUS_ZERO: break;
        }
    }
    for (;;) { /* STORLP71 */
        ts_no_overflow(ADDRESS_OF(MPAC));
        ca_indexed((word)(ADDRESS_OF(UPBUFF) + 2u));
        ts(L_ADDRESS);
        ca(ADDRESS_OF(MPAC));
        ad_at(ADDRESS_OF(UPTEMP));
        lxch_indexed();
        switch (ccs_at(ADDRESS_OF(MPAC))) {
        case POSITIVE: continue;
        case IS_PLUS_ZERO: upout(); return;
        case NEGATIVE:
            /* never: the count is II - 3, and II was let in from 3 to 20 alone. The next word is
             * an address word, ADUPBFM1's ADRES. */
            not_modelled();
            return;
        case IS_MINUS_ZERO: upout(); return;
        }
    }
}

/* UPEND72 -- LM p. 1394, CM p. 1505, V72: "HAVE AN ODD NO. OF COMPONENTS
 * BEEN SENT FOR A V72 UPDATE..." If not, "ERROR- SHOULD BE ODD NO. OF
 * COMPONENTS". LDLOOP72: "NOW PERFORM THE UPDATE", the pairs from the last
 * down, MPAC counting the words: "PICK UP NEXT UPDATE WORD" into L, "SET
 * POINTER TO ECADR(MUST BE CCS)", "PICK UP NEXT ECADR OF REG TO BE
 * UPDATED", "SET EBANK", "ISOLATE RELATIVE ADDRESS", and store the word,
 * until "THORUGH THE V72 UPDATE". Then UPOUT ("NORMAL FINISH OF P27"). The
 * CCS MPAC ways the counting does not take (MPAC negative or -0, after an
 * update of MPAC itself) go to the words after the loop's TCF, as the AGC
 * goes. */
void upend72(void)
{
    set_A(BIT1);
    mask_at(ADDRESS_OF(COMPNUMB));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: set_A(negate(BIT2)); break; /* TCF +2: "YES" */
    case IS_PLUS_ZERO: uperrout(); return;
    case NEGATIVE: set_A(negate(BIT2)); break;
    case IS_MINUS_ZERO: break;
    }
    ad_at(ADDRESS_OF(COMPNUMB));
    for (;;) { /* LDLOOP72 */
        ts_no_overflow(ADDRESS_OF(MPAC));
        ca_indexed((word)(ADDRESS_OF(UPBUFF) + 1u));
        lxch(A_ADDRESS);
        switch (ccs_at(ADDRESS_OF(MPAC))) {
        case POSITIVE: ts(ADDRESS_OF(MPAC)); /* fall through */
        case IS_PLUS_ZERO: ca_indexed((word)(ADDRESS_OF(UPBUFF) + 1u)); break;
        case NEGATIVE: ca(ADDRESS_OF(UPBUFF) + 1u); break;
        case IS_MINUS_ZERO: break;
        }
        ts(EBANK_ADDRESS);
        mask(LOW8);
        lxch_indexed();
        switch (ccs_at(ADDRESS_OF(MPAC))) {
        case POSITIVE: continue;
        case IS_PLUS_ZERO: upout(); return;
        case NEGATIVE: upout4(); return;
        case IS_MINUS_ZERO: upout_plus_2(); return;
        }
    }
}

/* UPEND70 -- LM p. 1395, CM p. 1506: "V70 DOES THE FOLLOWING WITH DP DELTA
 * TIME IN UPBUFF": its negative in UPBUFF +8D for TIMEDIDL, to "DECREMENT
 * AGC CLOCK". */
void upend70(void)
{
    dcs(ADDRESS_OF(UPBUFF));
    dxch(ADDRESS_OF(UPBUFF) + 8u);
    tc_at((word)(UPEND70 + 3u)); /* TC TIMEDIDL */
    timedidl();
}

/* UPEND70 +4 -- LM p. 1395, CM p. 1506, TIMEDIDL's error return: "ERROR
 * WHILE DECREMENTING CLOCK -- EXIT". */
void upend70_clock_error(void)
{
    tc_at((word)(UPEND70 + 4u)); /* TC UPERROUT */
    uperrout();
}

/* UPEND70 +5 -- LM p. 1395, CM p. 1506, TIMEDIDL's normal return: "COPY
 * DECREMENTERS FOR RESTART PROTECTION" in UPBUFF +10D and +12D, and "RESTART
 * PROTECT(GROUP 6)". */
void upend70_after_timedidl(void)
{
    dcs(ADDRESS_OF(UPBUFF));
    dxch(ADDRESS_OF(UPBUFF) + 10u);
    dcs(ADDRESS_OF(UPBUFF));
    dxch(ADDRESS_OF(UPBUFF) + 12u);
    tc_to((word)(UPEND70 + 013u), PHASCHNG); /* OCT 04026 */
}

/* UPEND70 +015 -- LM p. 1395, CM p. 1506, where PHASCHNG returns, in
 * TEPHEM's EBANK: "DECREMENT CSM STATE VECTOR TIME", "DECREMENT LEM STATE
 * VECTOR TIME", and "INCREMENT TP TEPHEM" (a triple precision word: the
 * DAS's carry goes on into its top word). Each copy is zeroed as it is used,
 * so a restart here does none twice. Then "RESTART PROTECT(GROUP 6)"
 * again. */
void upend70_after_phaschng(void)
{
    set_A(ZERO);
    zl();
    dxch(ADDRESS_OF(UPBUFF) + 10u);
    das(ADDRESS_OF(TETCSM));
    set_A(ZERO);
    zl();
    dxch(ADDRESS_OF(UPBUFF) + 12u);
    das(ADDRESS_OF(TETLEM));
    set_A(ZERO);
    zl();
    dxch(ADDRESS_OF(UPBUFF));
    das(ADDRESS_OF(TEPHEM) + 1u);
    ads(ADDRESS_OF(TEPHEM));
    tc_to((word)(UPEND70 + 032u), PHASCHNG); /* OCT 04026 */
}

/* UPEND70 +034 -- LM p. 1396, CM p. 1507, where PHASCHNG returns: "GO TO
 * STANDARD UPDATE PROGRAM EXIT". */
void upend70_after_phaschng2(void)
{
    tc_at((word)(UPEND70 + 034u)); /* TC UPOUT */
    upout();
}
