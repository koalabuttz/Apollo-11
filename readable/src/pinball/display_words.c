/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 2a: the word display routines
 * -- LM pp. 441-445, CM pp. 359-363. The faithful C is
 * c64-port/src/lanec_pin_gaps.c and the display paths of the port's
 * Pinball modules (src/lanec_pw_*.c).
 *
 * "WORD DISPLAY ROUTINES". Each puts a word on the DSKY a character at a
 * time with DSPIN, starting at the DSP number in DSPCOUNT, and goes back
 * through WDRET:
 *   DSPDECWD  a sign and five decimal digits, rounded (DSPRND);
 *   DSPDECNR  the same, not rounded;
 *   DSPDC2NR  a sign and two digits;
 *   DSP2DEC   a sign and ten digits from MPAC and MPAC +1, across R1 and
 *             R2 (R2's own signs turned off);
 *   DSPDECVN  a verb or noun number, two digits, no sign;
 *   DSPOCTWD  five octal digits (the signs blanked);
 *   DSP2BIT   two octal digits.
 * A decimal digit comes out of the top of MPAC each time SHORTMP
 * multiplies the fraction by ten; an octal one is three bits CYL moves to
 * the bottom. Each leaves DSPCOUNT -VD1, "TO BLOCK NUMERICAL CHARACTERS,
 * CLEARS, AND SIGNS AFTER A COMPLETED DISPLAY."
 *
 * Each function is the listing from a label, or from the word a call
 * returns to, to the next TC: the call or jump ends the function with Q
 * and Z set, and the binding (src/binding/pinball_routines.c) goes on
 * with the routine there, as the faithful port's steps go (they stop at
 * some of these calls). A call out of the file that comes back (SHORTMP)
 * is a C call; a TC WDRET or TC DSPWDRET is a TC to the erasable word,
 * which the binding runs as the AGC does. Bank 40, but DSPOCTWD, WDAGAIN,
 * OCTBACK and DSP2BIT in bank 41, which reach DSPIN through DSPOCTIN in
 * bank 40 ("SO DSPOCTWO DOESNT USE SWCALL") by POSTJUMP and come back by
 * BANKJUMP; the same code in both missions. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* ---- The sign and the rounding ---- */

/* DSPSIGN -- LM p. 441, CM p. 359.
 * The sign of the double word in MPAC, MPAC +1 on the DSKY (+ON or -ON,
 * part 1) and MPAC made its magnitude. The return goes to DSPWDRET, as
 * +ON and -ON use L. A positive or +0 word: +ON. A negative one: its
 * high word's magnitude (CCS leaves it less one: AD ONE) into MPAC, -ON,
 * then the low word complemented as well. */
void dspsign(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(DSPWDRET))) { /* an overflow in Q skips the CCS: TC +8D */
        tc_at(DSPSIGN + 3u);
        tc(DSPSIGN + 11u, PLUS_ON);
        return;
    }
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: tc_at(DSPSIGN + 3u); break;     /* TC +8D */
    case IS_PLUS_ZERO: tc_at(DSPSIGN + 4u); break; /* TC +7 */
    case NEGATIVE: ad(ONE);                        /* fall through */
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(MPAC));
        tc(DSPSIGN + 7u, MINUS_ON);
        return;
    }
    tc(DSPSIGN + 11u, PLUS_ON);
}

/* DSPSIGN +8, where -ON returns: the low word's magnitude too. */
void dspsign_after_minus_on(void)
{
    cs(ADDRESS_OF(MPAC) + 1u);
    ts(ADDRESS_OF(MPAC) + 1u);
    tc(DSPSIGN + 10u, ADDRESS_OF(DSPWDRET));
}

/* DSPSIGN +12, where +ON returns. */
void dspsign_after_plus_on(void)
{
    tc(DSPSIGN + 12u, ADDRESS_OF(DSPWDRET));
}

/* DSPRND -- LM p. 441, CM p. 359. "ROUND BY 5 EXP-6": DECROUND, 5 in
 * the sixth decimal place, added to MPAC, MPAC +1 (DAS); a sum that
 * overflows becomes DPOSMAX, the largest the five digits can show. */
void dsprnd(void)
{
    dca(DECROUND - 1u);
    das(ADDRESS_OF(MPAC));
    if (!A_is_zero()) { /* EXTEND; BZF +4: DAS left the overflow in A */
        dca(DPOSMAX);
        dxch(ADDRESS_OF(MPAC));
    }
    tc_return(Q_ADDRESS);
}

/* ---- The decimal words ---- */

/* DSPDECWD -- LM p. 441, CM p. 359.
 * "DSPDECWD CONVERTS C(MPAC, MPAC+1) INTO A SIGN AND 5 CHAR DECIMAL
 * STARTING IN LOC SPECIFIED IN DSPCOUNT. IT ROUNDS BY 5 EXP-6." The
 * return in WDRET; the sign (DSPSIGN), the rounding (DSPRND), five
 * digits (DSPDCWD1 -1). */
void dspdecwd(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(WDRET))) { /* an overflow in Q skips the sign */
        dspdecwd_after_dspsign();
        return;
    }
    tc(DSPDECWD + 2u, DSPSIGN);
}

void dspdecwd_after_dspsign(void)
{
    tc(DSPDECWD + 3u, DSPRND);
}

/* DSPDCWD1 -1, where DSPRND returns: the count of the digits after this
 * one, four. */
void dspdcwd1_minus1(void)
{
    set_A(FOUR);
    dspdcwd1();
}

/* DSPDCWD1, TRACE1 and TRACE1S -- LM pp. 441-442, CM pp. 359-360.
 * One digit, with the count of those after it in A (into WDCNT). SHORTMP
 * multiplies the triple word MPAC by BINCON, ten: the digit is the whole
 * part, in MPAC, and the fraction left moves up to MPAC, MPAC +1. The
 * digit's relay code (RELTAB) goes into CODE, DSPCOUNT into COUNT, and
 * DSPIN puts it on the DSKY ("DECREMENT DSPCOUNT EXCEPT AT +0"). A
 * DSPCOUNT of -0 goes on to the next digit without it. */
void dspdcwd1(void)
{
    if (ts(ADDRESS_OF(WDCNT))) not_modelled(); /* an overflow would skip CAF BINCON */
    set_A(BINCON);
    tc_at(DSPDCWD1 + 2u);
    shortmp();
    /* TRACE1 */
    ca(indexed_by(ADDRESS_OF(RELTAB), fetch(ADDRESS_OF(MPAC))));
    mask(LOW5);
    ts(ADDRESS_OF(CODE));
    set_A(ZERO);
    xch(ADDRESS_OF(MPAC) + 2u);
    xch(ADDRESS_OF(MPAC) + 1u);
    ts(ADDRESS_OF(MPAC));
    xch(ADDRESS_OF(DSPCOUNT));
    /* TRACE1S */
    ts(ADDRESS_OF(COUNT));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ts(ADDRESS_OF(DSPCOUNT)); /* fall through */
    case IS_PLUS_ZERO: tc(DSPDCWD1 + 15u, DSPIN); return;
    case NEGATIVE: dspdcwd1_after_dspin(); return;
    case IS_MINUS_ZERO: tc(DSPDCWD1 + 17u, DSPDCWD1); return;
    }
}

/* DSPDCWD1 +16, where DSPIN returns: the next digit while WDCNT counts
 * down, then DSPCOUNT -VD1 and back through WDRET. */
void dspdcwd1_after_dspin(void)
{
    switch (ccs_at(ADDRESS_OF(WDCNT))) {
    case POSITIVE: tc(DSPDCWD1 + 17u, DSPDCWD1); return;
    case IS_PLUS_ZERO:
        set_A(negate(VD1)); /* CS VD1 */
        /* fall through */
    case NEGATIVE: ts(ADDRESS_OF(DSPCOUNT)); break;
    case IS_MINUS_ZERO: break;
    }
    tc(DSPDCWD1 + 20u, ADDRESS_OF(WDRET));
}

/* DSPDECNR -- LM p. 442, CM p. 360.
 * "DSPDECNR CONVERTS C(MPAC,MPAC+1) INTO A SIGN AND 5 CHAR DECIMAL
 * STARTING IN LOC SPECIFIED IN DSPCOUNT. IT DOES NOT ROUND" */
void dspdecnr(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(WDRET))) { /* an overflow in Q skips the sign */
        dspdecnr_after_dspsign();
        return;
    }
    tc(DSPDECNR + 2u, DSPSIGN);
}

void dspdecnr_after_dspsign(void)
{
    tc(DSPDECNR + 3u, DSPDCWD1 - 1u);
}

/* DSPDC2NR -- LM p. 442, CM p. 360.
 * "DSPDC2NR CONVERTS C(MPAC,MPAC+1) INTO A SIGN AND 2 CHAR DECIMAL
 * STARTING IN LOC SPECIFIED IN DSPCOUNT. IT DOES NOT ROUND" */
void dspdc2nr(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(WDRET))) { /* an overflow in Q skips the sign */
        dspdc2nr_plus3();
        return;
    }
    tc(DSPDC2NR + 2u, DSPSIGN);
}

/* DSPDC2NR +3, where DSPSIGN returns (and where DSPDECVN goes): one digit
 * after this one. */
void dspdc2nr_plus3(void)
{
    set_A(ONE);
    tc(DSPDC2NR + 4u, DSPDCWD1);
}

/* DSP2DEC and END2DEC -- LM p. 442, CM p. 360.
 * "DSP2DEC CONVERTS C(MPAC) AND C(MPAC+1) INTO A SIGN AND 10 CHAR DECIMAL
 * STARTING IN THE LOC SPECIFIED IN DSPCOUNT." R2's two sign bits off
 * (11DSPIN with CODE +0, DSPTAB +3 "-R2 OFF" and +4 "+R2 OFF"), the sign
 * (DSPSIGN), then DSPDCWD1 with R2D1 (nine) digits after the first. */
void dsp2dec(void)
{
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(WDRET))) set_A(ZERO); /* an overflow in Q skips CAF ZERO */
    ts(ADDRESS_OF(CODE));
    set_A(THREE);
    tc(DSP2DEC + 5u, ELEVEN_DSPIN);
}

void dsp2dec_after_minus_r2(void)
{
    set_A(FOUR);
    tc(DSP2DEC + 7u, ELEVEN_DSPIN);
}

void dsp2dec_after_plus_r2(void)
{
    tc(DSP2DEC + 8u, DSPSIGN);
}

void dsp2dec_after_dspsign(void)
{
    set_A(R2D1);
    end2dec();
}

void end2dec(void)
{
    tc(END2DEC, DSPDCWD1);
}

/* DSPDECVN -- LM p. 443, CM p. 361.
 * "DSPDECVN DISPLAYS C(A) UPON ENTRY AS A 2 CHAR DECIMAL BEGINNING IN THE
 * DSP LOC SPECIFIED IN DSPCOUNT. C(A) SHOULD BE IN FORM N X 2EXP-14. THIS
 * IS SCALED TO FORM N/100 BEFORE DISPLAY CONVERSION." MP VNDSPCON (".01
 * ROUNDED UP") leaves N/100 in L: into MPAC, MPAC +1 +0, then two digits
 * from DSPDC2NR +3 ("NO SIGN, NO ROUND, 2 CHAR"). */
void dspdecvn(void)
{
    mp(VNDSPCON);
    lxch(ADDRESS_OF(MPAC));
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 1u);
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(WDRET))) not_modelled(); /* an overflow would run VNDSPCON */
    tc(DSPDECVN + 7u, DSPDC2NR + 3u);
}

/* GOVNUPDT -- LM p. 443, CM p. 361. "THIS IS NOT FOR GENERAL USE. REALLY
 * PART OF UPDATVB.": the number in A on the DSKY, then back to UPDAT1 +2
 * in bank 41 by POSTJUMP. */
void govnupdt(void)
{
    tc(GOVNUPDT, DSPDECVN);
}

void govnupdt_after_dspdecvn(void)
{
    tc(GOVNUPDT + 1u, POSTJUMP); /* CADR UPDAT1 +2 */
}

/* ---- The octal words ---- */

/* DSPOCTWD -- LM p. 443, CM p. 361 (bank 41).
 * "DSPOCTWD DISPLAYS C(A) UPON ENTRY AS A 5 CHAR OCT STARTING IN THE DSP
 * CHAR SPECIFIED IN DSPCOUNT. IT STOPS AFTER 5 CHAR HAVE BEEN DISPLAYED."
 * The word into CYL, the return in WDRET ("MUST USE SAME RETURN AS
 * DSP2BIT"), BIT14 into DSPCOUNT ("TO BLANK SIGNS": DSPIN blanks the sign
 * with the left character), and four digits after the first. */
void dspoctwd(void)
{
    if (ts(ADDRESS_OF(CYL))) not_modelled(); /* an overflow would skip XCH Q */
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(WDRET))) set_A(BIT14); /* an overflow in Q skips CAF BIT14 */
    ads(ADDRESS_OF(DSPCOUNT));
    set_A(FOUR);
    wdagain();
}

/* WDAGAIN -- LM p. 443, CM p. 361 (bank 41).
 * One digit: CYL three times brings its three bits to the bottom (three
 * CS CYL and a CS A: the word complemented four times, as it was), DSPMSK
 * keeps them, RELTAB gives the relay code; DSPCOUNT into COUNT
 * ("DECREMENT DSPCOUNT EXCEPT AT +0") and DSPIN, through DSPOCTIN in bank
 * 40. A DSPCOUNT of -0 skips the digit (OCTBACK). The faithful port takes
 * no negative DSPCOUNT here, nor does the readable one: the CCS would run
 * the CADR after TC POSTJUMP as an instruction. */
void wdagain(void)
{
    if (ts(ADDRESS_OF(WDCNT))) not_modelled(); /* an overflow would skip a CS CYL */
    cs(ADDRESS_OF(CYL));
    cs(ADDRESS_OF(CYL));
    cs(ADDRESS_OF(CYL));
    cs(A_ADDRESS);
    wdagain_plus5();
}

/* WDAGAIN +5, where DSP2BIT comes in with its digits in CYL. */
void wdagain_plus5(void)
{
    mask(DSPMSK);
    ca(indexed_by(ADDRESS_OF(RELTAB), a_word()));
    mask(LOW5);
    ts(ADDRESS_OF(CODE));
    xch(ADDRESS_OF(DSPCOUNT));
    ts(ADDRESS_OF(COUNT));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ts(ADDRESS_OF(DSPCOUNT));                /* fall through */
    case IS_PLUS_ZERO: tc(WDAGAIN + 14u, POSTJUMP); return; /* CADR DSPOCTIN */
    case NEGATIVE: not_modelled(); return;
    case IS_MINUS_ZERO: octback(); return;
    }
}

/* DSPOCTIN and ENDSPOCT -- LM p. 445, CM p. 363 (bank 40). DSPIN for the
 * octal words, then back to OCTBACK in bank 41 by BANKJUMP, the CADR in A
 * (CAF +2: ENDSPOCT). */
void dspoctin(void)
{
    tc(DSPOCTIN, DSPIN);
}

void dspoctin_after_dspin(void)
{
    ca(ENDSPOCT);
    tc(DSPOCTIN + 2u, BANKJUMP);
}

/* OCTBACK and DSPLV -- LM pp. 443-444, CM pp. 361-362 (bank 41). The next
 * digit while WDCNT counts down; then DSPCOUNT -VD1 (DSPLV) and back
 * through WDRET. */
void octback(void)
{
    switch (ccs_at(ADDRESS_OF(WDCNT))) {
    case POSITIVE: tc(OCTBACK + 1u, WDAGAIN); return;
    case IS_PLUS_ZERO:
        set_A(negate(VD1)); /* DSPLV: CS VD1 */
        /* fall through */
    case NEGATIVE: ts(ADDRESS_OF(DSPCOUNT)); break;
    case IS_MINUS_ZERO: break;
    }
    tc(DSPLV + 2u, ADDRESS_OF(WDRET));
}

/* DSP2BIT -- LM p. 444, CM p. 362 (bank 41).
 * "DSP2BIT DISPLAYS C(A) UPON ENTRY AS A 2 CHAR OCT BEGINNING IN THE DSP
 * LOC SPECIFIED IN DSPCOUNT BY PRE CYCLING RIGHT C(A) AND USING THE LOGIC
 * OF THE 5 CHAR OCTAL DISPLAY". CYR three times, then the word into CYL:
 * WDAGAIN's three CYLs bring the two digits' bits down in turn. */
void dsp2bit(void)
{
    if (ts(ADDRESS_OF(CYR))) not_modelled(); /* an overflow would skip XCH Q */
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(WDRET))) set_A(ONE); /* an overflow in Q skips CAF ONE */
    ts(ADDRESS_OF(WDCNT));
    cs(ADDRESS_OF(CYR));
    cs(ADDRESS_OF(CYR));
    xch(ADDRESS_OF(CYR));
    ts(ADDRESS_OF(CYL));
    tc(DSP2BIT + 9u, WDAGAIN + 5u);
}
