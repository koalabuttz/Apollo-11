/* SPDX-License-Identifier: MIT */
/* The switch orders: INTERPRETER.agc LM pp. 1092-1094, CM pp. 1197-1199 (in
 * bank 1). The faithful C is c64-port/src/lm_interp_core.c (lmi_switch).
 *
 * "THE FOLLOWING OPERATIONS ARE AVAILABLE FOR ALTERING AND TESTING
 * INTERPRETATIVE SWITCHES:
 *	00 BONSET	SET A SWITCH AND DO A GOTO IF IT WAS ON.
 *	01 SETGO	SET A SWITCH AND DO A GOTO.
 *	02 BOFSET	SET A SWITCH AND DOA GOTO IF IT WAS OFF
 *	03 SET		SET A SWITCH.
 *	04 BONINV	INVERT A SWITCH AND BRANCH IF IT WAS ON.
 *	05 INVGO	INVERT A SWITCH AND DO A GOTO.
 *	06 BOFINV	INVERT A SWITCH AND BRANCH IF IT WAS OFF
 *	07 INVERT	INVERT A SWITCH.
 *	10 BONCLR	CLEAR A SWITCH AND BRANCH IF IT WAS ON.
 *	11 CLRGO	CLEAR A SWITCH AND DO A GOTO.
 *	12 BOFCLR	CLEAR A SWITCH AND BRANCH IF IT WAS OFF.
 *	13 CLEAR	CLEAR A SWITCH.
 *	14 BON		BRANCH IF A SWITCH WAS ON.
 *	16 BOFF		BRANCH IF A SWITCH WAS OFF.
 * THE ADDRESS SUPPLIED WITH THE SWITCH INSTRUCTION IS INTERPRETED AS
 * FOLLOWS:
 *	BITS 1-4	SWITCH BIT NUMBER (1-15).
 *	BITS 5-8	SWITCH OPERATION NUMBER
 *	BITS 9-		SWITCH WORD NUMBER (UP TO 64 SWITCH WORDS).
 * THE ADDRESS ITSELF IS MADE UP BY THE YUL SYSTEM ASSEMBLER. THE BRANCH
 * INSTRUCTIONS REQUIRE TWO ADDRESSES, THE SECOND TAKEN AS THE DIRECT (OR
 * INDIRECT IF IN ERASABLE) ADDRESS OF THE BRANCH."
 *
 * The switches are the bits of the STATE words (the flagwords). The
 * operation number is two 2-bit codes: what to do to the switch (bits 7-8)
 * and what to do next (bits 5-6). */
#include "interpreter.h"
#include "../agc/names.h"

static void swstore(void);
static void test(void);
static void swskip(void);

/* SWITCHES (SW/) -- LM pp. 1092-1094, CM pp. 1197-1199.
 * "LEAVE THE SWITCH BIT IN SWBIT" ("NUMBER FROM LEFT TO RIGHT"), "LEAVE THE
 * SWITCH NUMBER IN SWWORD", "LEAVE THE SWITCH WORD ITSELF IN L" (in Q: "Q
 * WILL BE USED AS A CHANNEL", channel 2, for ROR and RXOR). INHINT
 * "DURING SWITCH CHANGE SO RUPT CAN USE TOO". Then "DISPATCH SWITCH BIT
 * OPERATION AS IN BITS 7-8 OF POLISH" (INDEX A; TCF +1 on "4X2-BIT
 * CODE"). */
void switches(void)
{
    set_A(LOW4);
    mask_at(ADDRESS_OF(POLISH));
    ca(indexed(ADDRESS_OF(BIT15), a_word()));
    ts(ADDRESS_OF(SWBIT));

    set_A(BIT7);
    mp_at(ADDRESS_OF(POLISH));
    ts(ADDRESS_OF(SWWORD));

    inhint();
    ca(indexed(ADDRESS_OF(STATE), a_word()));
    ts(Q_ADDRESS);

    set_A(BIT11);
    mp_at(ADDRESS_OF(POLISH));
    mask(B3TOB4);
    switch (a_word()) {
    case 000: /* "00 -- SET SWITCH IN QUESTION." */
        ca(ADDRESS_OF(SWBIT));
        ror_qchan();
        swstore();
        break;
    case 004: /* "01 -- INVERT SWITCH." */
        ca(ADDRESS_OF(SWBIT));
        rxor_qchan();
        swstore();
        break;
    case 010: /* "10 -- CLEAR." */
        cs(ADDRESS_OF(SWBIT));
        mask_at(Q_ADDRESS);
        swstore();
        break;
    default: break; /* "11 -- NOOP." */
    }
    relint();

    /* "DISPATCH SEQUENCE CHANGING OR BRANCING CODE", "ORIGINALLY STORED IN
     * BITS 5-6". Q still holds the switch word as it was. */
    set_A(BIT13);
    mp_at(ADDRESS_OF(POLISH));
    mask(B3TOB4);
    switch (a_word()) {
    case 000: /* "00 -- BRANCH IF ON." */
        cs(Q_ADDRESS);
        test();
        return;
    case 004: swbranch(); return; /* "01 -- GO TO." */
    case 010:                     /* "10 -- BRANCH IF OFF." */
        ca(Q_ADDRESS);
        test();
        return;
    default: continue_at(DANZIG); return; /* "11 -- NOOP." */
    }
}

/* SWSTORE: "NEW SWITCH WORD." */
static void swstore(void)
{
    ts(indexed(ADDRESS_OF(STATE), SWWORD));
}

/* TEST: the switch bit of the word as it was (complemented for "BRANCH IF
 * ON"): zero branches, otherwise skip the branch address. (Negative "HERE
 * ONLY ON BIT 15"; -0 never: that slot is TC CCSHOLE.) */
static void test(void)
{
    mask_at(ADDRESS_OF(SWBIT));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
    case NEGATIVE: swskip(); return;
    case IS_PLUS_ZERO: swbranch(); return;
    case IS_MINUS_ZERO:
        tc_at(TEST + 5u);
        continue_at(CCSHOLE);
        return;
    }
}

/* SWSKIP: past the branch address, to the next pair. */
static void swskip(void)
{
    incr(ADDRESS_OF(LOC));
    continue_at(DANZIG);
}
