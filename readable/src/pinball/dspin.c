/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc: DSPIN and 11DSPIN, which put a
 * character into the display table -- LM pp. 444-445, CM pp. 362-363.
 * The faithful C is the display paths of c64-port/src/lanec_pw_*.c.
 *
 * DSPTAB holds the DSKY's relay words, one for each pair of characters
 * (and the signs), each with the relay code of the two characters in bits
 * 1-5 and 6-10 and a sign bit in bit 11. T4RUPT sends a word to the DSKY
 * while it is negative ("its sign the flag that it has not gone out");
 * NOUT counts the words waiting. A character goes in by masking it into
 * its half of the word, which goes in negative; NOUT goes up if the word
 * was waiting no more. All of it is in bank 40, the same in both
 * missions. */
#include "pinball.h"
#include "../agc/names.h"

/* DSPIN1 -- LM p. 445, CM p. 363.
 * "FOR DSPIN1 PLACE 0,1 INTO BIT11 OF CODE, 2 INTO COUNT, REL ADDRESS OF
 * DSPTAB ENTRY INTO DSREL." COUNT picks the half (DSMSK: 0 the right
 * character, 1 the left, 2 the sign bit, 3 the left character and the
 * sign). The word's magnitude goes to DSMAG (a +0 word, which never
 * happens, is TC CCSHOLE); if its part already holds CODE ("SAME") the
 * word stays as it is (DSLV), else the part is replaced and the word
 * stored negative (DFRNT). Interrupts are off meanwhile. Back through
 * DSEXIT. */
static bool dspin1_returns(void)
{
    word dsrel = fetch(ADDRESS_OF(DSREL));
    word count = fetch(ADDRESS_OF(COUNT));

    inhint();
    switch (ccs_at(indexed_by(ADDRESS_OF(DSPTAB), dsrel))) {
    case POSITIVE:
        tc_at(DSPIN1 + 3u); /* TC +2: "IF +" */
        ad(ONE);
        break;
    case IS_PLUS_ZERO: tc(DSPIN1 + 4u, CCSHOLE); return false;
    case NEGATIVE: ad(ONE); break; /* "IF -" */
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(DSMAG));
    mask_at(indexed_by(ADDRESS_OF(DSMSK), count));
    su_at(ADDRESS_OF(CODE));
    if (!A_is_zero()) { /* EXTEND; BZF DSLV: "SAME" */
        /* DFRNT: "MASK WITH 77740, 76037, 76777, OR 74037" */
        cs(indexed_by(ADDRESS_OF(DSMSK), count));
        mask_at(ADDRESS_OF(DSMAG));
        ad_at(ADDRESS_OF(CODE));
        cs(A_ADDRESS);
        xch(indexed_erasable(ADDRESS_OF(DSPTAB), dsrel));
        if (!A_is_zero_or_minus()) incr(ADDRESS_OF(NOUT)); /* EXTEND; BZMF DSLV */
    }
    /* DSLV */
    relint();
    tc_return(ADDRESS_OF(DSEXIT));
    return true;
}

/* DSPIN -- LM p. 444, CM p. 362.
 * "FOR DSPIN PLACE 0/25 OCT INTO COUNT, 5 BIT RELAY CODE INTO CODE. BOTH
 * ARE DESTROYED. IF BIT14 OF COUNT IS 1, SIGN IS BLANKED WITH LEFT CHAR."
 * The return goes to DSEXIT: "CANT USE L FOR RETURN, SINCE MANY OF THE
 * ROUTINES CALLING DSPIN USE L AS RETURN." COUNT (a DSP number) halved is
 * the DSPTAB word (DSREL, by TS SR); "LEFT IF COUNT IS ODD", the code
 * moved up five bits (SLEFT5), "RIGHT IF COUNT IS EVEN". COUNT then picks
 * DSPIN1's mask: "+0 INTO COUNT FOR RIGHT, +1 INTO COUNT FOR LEFT (SIGN
 * LEFT ALONE), +3 INTO COUNT FOR LEFT (TO BLANK SIGN)". */
bool dspin_returns(void)
{
    xch(Q_ADDRESS);
    ts(ADDRESS_OF(DSEXIT));
    set_A(LOW5);
    mask_at(ADDRESS_OF(COUNT));
    ts(ADDRESS_OF(SR));
    xch(ADDRESS_OF(SR));
    ts(ADDRESS_OF(DSREL));
    set_A(BIT1);
    mask_at(ADDRESS_OF(COUNT)); /* CCS A: MASK leaves 0 or 1 */
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc_at(DSPIN + 10u); /* TC +2: "LEFT IF COUNT IS ODD" */
        xch(ADDRESS_OF(CODE));
        tc_at(DSPIN + 13u);
        sleft5(); /* "DOES NOT USE CYL" */
        ts(ADDRESS_OF(CODE));
        set_A(BIT14);
        mask_at(ADDRESS_OF(COUNT));                    /* CCS A: 0 or BIT14 */
        if (ccs_at(A_ADDRESS) == POSITIVE) set_A(TWO); /* "BIT14 = 1, BLANK SIGN" */
        ad(ONE);
    } else
        tc_at(DSPIN + 11u); /* TC DSPIN1 -1: "RIGHT IF COUNT IS EVEN" */
    ts(ADDRESS_OF(COUNT));
    return dspin1_returns();
}

void dspin(void)
{
    dspin_returns();
}

/* 11DSPIN -- LM p. 445, CM p. 363.
 * "FOR 11DSPIN, PUT REL ADDRESS OF DSPTAB ENTRY INTO A, 1 IN BIT11 OR 0 IN
 * BIT11 OF CODE." A sign bit: COUNT 2, and the return in DSEXIT ("MUST
 * USE SAME RETURN AS DSPIN"). */
bool eleven_dspin_returns(void)
{
    ts(ADDRESS_OF(DSREL));
    set_A(TWO);
    ts(ADDRESS_OF(COUNT));
    xch(Q_ADDRESS);
    ts(ADDRESS_OF(DSEXIT));
    tc_at(ELEVEN_DSPIN + 5u);
    return dspin1_returns();
}

void eleven_dspin(void)
{
    eleven_dspin_returns();
}
