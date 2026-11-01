/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 3: the display verbs -- LM pp.
 * 414-417 and 424, CM pp. 331-334 and 341-342. The faithful C is the
 * port's Pinball modules (c64-port/src/lanec_pw_*.c, lanec_pin_head_*.c).
 *
 * The verb fan (part 2b) sends a display verb here with the noun looked up
 * and NOUNADD at its first word (for a mixed noun MIXNOUN has fetched the
 * components into MIXTEMP). The octal verbs 01-05 (DSPA, DSPB, DSPC, DSPAB,
 * DSPABC) put one, two or three words in R1-R3 by DSPOCTWD, after a check
 * that the noun has the component the verb asks for (COMPTEST) and is not
 * "decimal only" (DECTEST, DCOMPTST). Verb 06, DECDSP, shows each
 * component in decimal: the word, its scale factor constant (SFCONUM,
 * GTSFOUT) and its scale factor routine (SFRUTNOR or SFRUTMIX), whose
 * output routine (SFOUTABR, output_scale.c) scales it and comes back to
 * DSPDCEND, which puts it on the DSKY. Verb 07, DSPDPDEC, shows a double
 * word as ten digits in R1 and R2.
 *
 * Bank 41 (DSPDPDEC in bank 40), the same code in both missions. As in
 * part 2b, each function runs from a label, or from the word a call
 * returns to, to a call or jump to another routine or to the next label the
 * faithful port enters, which ends it with Q and Z set. The checks
 * (COMPTEST, DECTEST, ...) and GETCOMP and TSTFORDP, which come straight
 * back, are C calls; those that may alarm instead say so. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../pinball_noun_tables/pinball_noun_tables.h"
#include "../agc/names.h"

bool mixed_noun(word at)
{
    word mixbr = fetch(ADDRESS_OF(MIXBR));

    if (mixbr != 1u && mixbr != 2u) not_modelled(); /* the TC would land on another word */
    tc_at(at);
    return mixbr == 2u;
}

/* ---- The component checks (LM pp. 414-416, CM pp. 331-333) ---- */

/* GETCOMP -- LM p. 416, CM p. 333. "GET HI5 OF NNTYPTAB (NORM) OF NNADTAB
 * (MIX)": the component code of the noun, from NNTYPTEM or NNADTEM
 * (COMPICK's ADRES for MIXBR), in A. */
void getcomp(void)
{
    ca(indexed_by(ADDRESS_OF(COMPICK) - 1u, fetch(ADDRESS_OF(MIXBR))));
    ca(indexed_by(0u, a_word())); /* INDEX A; CA 0 */
    mask(HI5);
    tc_return(Q_ADDRESS);
}

/* COMPTEST and COMPTST1 -- LM pp. 414-415, CM pp. 331-332. "COMPTEST
 * ALARMS IF COMPONENT NUMBER OF VERB (LOAD OR OCT DISPLAY) IS GREATER THAN
 * THE HIGHEST COMPONENT NUMBER OF NOUN." A holds minus the verb's
 * component (0, 1, 2 for A, B, C); the noun's highest is the component
 * code's top two bits (LEFT5, MASK THREE). The return in L. True when it
 * went back. */
static bool comptst1_returns(void)
{
    getcomp();
    tc_at(COMPTST1 + 1u);
    left5();
    mask(THREE);                /* "NOUN COMP" */
    ad_at(ADDRESS_OF(SFTEMP1)); /* "NOUN COMP - VERB COMP" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_return(L_ADDRESS); return true; /* "NOUN COMP G/ VERB COMP" */
    case IS_PLUS_ZERO: tc(COMPTST1 + 6u, CCSHOLE); return false;
    case NEGATIVE: tc(COMPTST1 + 7u, GODSPALM); return false; /* "NOUN COMP L/ VERB COMP" */
    case IS_MINUS_ZERO: break;
    }
    tc_return(L_ADDRESS); /* NDCMPTST: "NOUN COMP = VERB COMP" */
    return true;
}

static bool comptest_returns(void)
{
    if (ts(ADDRESS_OF(SFTEMP1))) not_modelled(); /* an overflow would skip LXCH Q */
    lxch(Q_ADDRESS);
    tc_at(COMPTST1);
    return comptst1_returns();
}

void comptest(void)
{
    (void)comptest_returns();
}

void comptst1(void)
{
    tc_at(COMPTST1);
    (void)comptst1_returns();
}

/* DECTEST -- LM p. 415, CM p. 332. "ALARMS IF DEC ONLY BIT = 1 (BIT4 OF
 * COMP CODE NUMBER). RETURNS IF NOT." The return in MPAC +2 (QXCH). */
static bool dectest_returns(void)
{
    qxch(ADDRESS_OF(MPAC) + 2u);
    tc_at(DECTEST + 2u);
    getcomp();
    mask(BIT14);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc(DECTEST + 5u, GODSPALM);
        return false;
    }
    tc_return(ADDRESS_OF(MPAC) + 2u); /* TC MPAC +2 */
    return true;
}

void dectest(void)
{
    (void)dectest_returns();
}

/* DCOMPTST -- LM p. 415, CM p. 332. "DCOMPTST ALARMS IF DECIMAL ONLY BIT
 * (BIT4 OF COMP CODE NUMBER) = 1. IF NOT, IT PERFORMS REGULAR COMPTEST."
 * The return in L. */
static bool dcomptst_returns(void)
{
    if (ts(ADDRESS_OF(SFTEMP1))) not_modelled(); /* "- VERB COMP"; an overflow would skip LXCH Q */
    lxch(Q_ADDRESS);
    tc_at(DCOMPTST + 2u);
    if (!dectest_returns()) return false;
    tc_at(DCOMPTST + 3u);
    return comptst1_returns();
}

void dcomptst(void)
{
    (void)dcomptst_returns();
}

/* DCTSTCYC -- LM p. 415, CM p. 332. "ALARMS AND RECYCLES IF DEC ONLY BIT =
 * 1 (BIT4 OF COMP CODE NUMBER). RETURNS IF NOT. USED BY LOAD VERBS." */
bool dctstcyc_returns(void)
{
    lxch(Q_ADDRESS);
    tc_at(DCTSTCYC + 1u);
    getcomp();
    mask(BIT14);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        tc(DCTSTCYC + 4u, ALMCYCLE);
        return false;
    }
    tc_return(L_ADDRESS);
    return true;
}

void dctstcyc(void)
{
    (void)dctstcyc_returns();
}

/* NOUNTEST -- LM p. 415, CM p. 332. "NOUNTEST ALARMS IF NO-LOAD BIT (BIT5
 * OF COMP CODE NUMBER) = 1. IF NOT, IT RETURNS." The no-load bit is the
 * word's sign. */
void nountest(void)
{
    lxch(Q_ADDRESS);
    tc_at(NOUNTEST + 1u);
    getcomp();
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
    case IS_PLUS_ZERO: tc_return(L_ADDRESS); return;
    case NEGATIVE: tc(NOUNTEST + 5u, GODSPALM); return;
    case IS_MINUS_ZERO: not_modelled(); /* MASK HI5 never leaves -0 */
    }
}

/* TSTFORDP -- LM pp. 415-416, CM pp. 332-333. "TEST FOR DP. IF SO, GET
 * MINOR PART ONLY." A noun whose NNADTEM is -1 is a channel ("CHANNEL TO
 * BE SPECIFIED"): to CHANDSP. A mixed noun is "ALREADY HANDLED IN
 * MIXNOUN". A normal noun whose scale factor routine is double precision
 * (SFRUTNOR, DPTEST) has NOUNADD moved to its second word ("E+1 INTO
 * NOUNADD FOR MINOR PART"). The return in L. True when it went back. */
static bool tstfordp_returns(void)
{
    lxch(Q_ADDRESS);
    ca(ADDRESS_OF(NNADTEM));
    ad(ONE);
    if (A_is_zero()) { /* EXTEND; BZF CHANDSP */
        continue_at(CHANDSP);
        return false;
    }
    if (mixed_noun(TSTFORDP + 6u)) {
        tc_return(L_ADDRESS); /* "MIXED CASE ALREADY HANDLED IN MIXNOUN" */
        return true;
    }
    tc_at(TSTFORDP + 7u); /* TC +2: "NORMAL" */
    tc_at(TSTFORDP + 9u);
    sfrutnor();
    tc_at(TSTFORDP + 10u);
    if (dptest()) incr(ADDRESS_OF(NOUNADD)); /* "DP" */
    tc_return(L_ADDRESS);
    return true;
}

/* ---- The octal display verbs (LM pp. 414-416, CM pp. 331-333) ---- */

static void dspcom1(void);
static void dspcom2_plus2(void);

/* DSPABC, DSPAB and DSPA -- LM p. 414, CM p. 331. Verbs 05, 04 and 01:
 * three, two or one words, complemented, into BUF +2, BUF +1 and BUF.
 * DSPA takes a channel's word for a channel noun, and only the second word
 * of a double precision one. */
void dspabc(void)
{
    set_A(negate(TWO));
    tc_at(DSPABC + 1u);
    if (!comptest_returns()) return;
    cs(indexed_by(2u, fetch(ADDRESS_OF(NOUNADD)))); /* INDEX NOUNADD; CS 2 */
    xch(ADDRESS_OF(BUF) + 2u);
    dspab();
}

void dspab(void)
{
    set_A(negate(ONE));
    tc_at(DSPAB + 1u);
    if (!comptest_returns()) return;
    cs(indexed_by(1u, fetch(ADDRESS_OF(NOUNADD)))); /* INDEX NOUNADD; CS 1 */
    xch(ADDRESS_OF(BUF) + 1u);
    dspa();
}

void dspa(void)
{
    tc_at(DSPA);
    if (!dectest_returns()) return;
    tc_at(DSPA + 1u);
    if (!tstfordp_returns()) return;
    cs(indexed_by(0u, fetch(ADDRESS_OF(NOUNADD)))); /* INDEX NOUNADD; CS 0 */
    dspcom1();
}

/* DSPCOM1: the first word into BUF, and on to show them. */
static void dspcom1(void)
{
    xch(ADDRESS_OF(BUF));
    tc_at(DSPCOM1 + 1u);
    /* DSPCOM2: "A B C AB ABC" are verbs 1 to 5. */
    set_A(negate(TWO));
    ad_at(ADDRESS_OF(VERBREG)); /* "-1 -0 +1 +2 +3 IN A" */
    dspcom2_plus2();
}

/* DSPB and DSPC -- LM p. 414, CM p. 331. Verbs 02 and 03: the second or
 * the third word only (DCOMPTST: not for a decimal only noun). */
void dspb(void)
{
    set_A(negate(ONE));
    tc_at(DSPB + 1u);
    if (!dcomptst_returns()) return;
    cs(indexed_by(1u, fetch(ADDRESS_OF(NOUNADD))));
    tc_at(DSPB + 4u);
    dspcom1();
}

void dspc(void)
{
    set_A(negate(TWO));
    tc_at(DSPC + 1u);
    if (!dcomptst_returns()) return;
    cs(indexed_by(2u, fetch(ADDRESS_OF(NOUNADD))));
    tc_at(DSPC + 4u);
    dspcom1();
}

/* DSPCOM2 +2 and DSPCOM3 -- LM p. 414, CM p. 331. "+0 +0 +0 +1 +2 IN A
 * AFTER CCS": the count of words still to show after this one. Each word
 * (BUF +K, complemented back) goes in octal to R1, R2 or R3 (R1D1 +K),
 * the last first, by DSPOCTWD; DISTEM holds the count across it. With
 * none left, the verb is done (ENTEXIT). */
static void dspcom2_plus2(void)
{
    word k;

    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_at(DSPCOM2 + 3u); break;
    case IS_PLUS_ZERO: tc(DSPCOM2 + 4u, ADDRESS_OF(ENTRET)); return; /* TC ENTEXIT */
    case NEGATIVE: tc_at(DSPCOM2 + 5u); break;                       /* TC +1 */
    case IS_MINUS_ZERO: break;
    }
    /* DSPCOM3 */
    ts(ADDRESS_OF(DISTEM)); /* "+0 +1 +2 INTO DISTEM" */
    k = a_word();
    ca(indexed_by(ADDRESS_OF(R1D1), k));
    ts(ADDRESS_OF(DSPCOUNT));
    cs(indexed_by(ADDRESS_OF(BUF), fetch(ADDRESS_OF(DISTEM))));
    tc(DSPCOM3 + 6u, DSPOCTWD);
}

/* DSPCOM3 +7, where DSPOCTWD comes back: the count back into A, and round
 * again (TC DSPCOM2 +2). */
void dspcom3_after_dspoctwd(void)
{
    xch(ADDRESS_OF(DISTEM));
    tc_at(DSPCOM3 + 8u);
    dspcom2_plus2();
}

/* CHANDSP -- LM p. 416, CM p. 333. A channel noun: the channel (NOUNCADR,
 * low 9 bits) read, complemented, and shown as verb 01 shows a word. */
void chandsp(void)
{
    ca(ADDRESS_OF(NOUNCADR));
    mask(LOW9);
    read_channel(a_word()); /* EXTEND; INDEX A; READ 0 */
    cs(A_ADDRESS);
    dspcom1(); /* TCF DSPCOM1 */
}

/* ---- The decimal display verb (LM pp. 416-417, CM pp. 333-334) ---- */

/* DECDSP and DSPDCGET -- LM p. 416, CM p. 333. Verb 06. "COMP NUMBER INTO
 * DECOUNT": 0, 1 or 2 for one, two or three components. DSPDCGET "PICKS
 * UP DATA": each word, complemented, into XREG, YREG, ZREG ("CANT USE BUF
 * SINCE DMP USES IT."), the last first. */
void decdsp(void)
{
    getcomp();
    tc_at(DECDSP + 1u);
    left5();
    mask(THREE);
    ts(ADDRESS_OF(DECOUNT));
    for (;;) {
        /* DSPDCGET */
        word k;

        ts(ADDRESS_OF(DECTEM)); /* "DECTEM 1COMP +0, 2COMP +1, 3COMP +2" */
        ad_at(ADDRESS_OF(NOUNADD));
        cs(indexed_by(0u, a_word())); /* INDEX A; CS 0 */
        k = fetch(ADDRESS_OF(DECTEM));
        xch(indexed_erasable(ADDRESS_OF(XREG), k));
        switch (ccs_at(ADDRESS_OF(DECTEM))) {
        case POSITIVE: tc_at(DSPDCGET + 7u); continue; /* TC DSPDCGET: "MORE TO GET" */
        case IS_PLUS_ZERO: break;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled(); /* MASK THREE leaves 0 to 3 */
        }
        break;
    }
    continue_at(DSPDCPUT); /* DECTEM +0 */
}

static void decdsp3(void);

/* DSPDCPUT -- LM pp. 416-417, CM pp. 333-334. "DISPLAYS DATA": component
 * DECOUNT, from R1D1 +DECOUNT, into MPAC (MPAC +1 and +2 zero). Its scale
 * factor constant into SFTEMP1, SFTEMP2 ("SWITCH BANKS TO SF CONSTANT
 * TABLE READING ROUTINE.": GTSFOUT, with twice the constant's number from
 * SFCONUM), and its scale factor routine's number into A (SFRUTNOR for a
 * normal noun, SFRUTMIX for a mixed one), for DECDSP3. */
void dspdcput(void)
{
    word k;

    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 1u);
    ts(ADDRESS_OF(MPAC) + 2u);
    k = fetch(ADDRESS_OF(DECOUNT));
    ca(indexed_by(ADDRESS_OF(R1D1), k));
    ts(ADDRESS_OF(DSPCOUNT));
    cs(indexed_by(ADDRESS_OF(XREG), k));
    ts(ADDRESS_OF(MPAC));
    tc_at(DSPDCPUT + 9u);
    sfconum(); /* "2X (SF CON NUMB) IN A" */
    ts(ADDRESS_OF(SFTEMP1));
    dca(ADDRESS_OF(GTSFOUTL));
    dtcb(DSPDCPUT + 13u); /* "LOADS SFTEMP1, SFTEMP2" */
    gtsfout();
    if (mixed_noun(DSPDCPUT + 15u)) {
        tc_at(DSPDCPUT + 17u);
        sfrutmix();
        tc_at(DSPDCPUT + 18u);
    } else {
        tc_at(DSPDCPUT + 16u);
        /* DSPSFNOR */
        tc_at(DSPSFNOR);
        sfrutnor();
        tc_at(DSPSFNOR + 1u);
    }
    decdsp3();
}

/* DECDSP3 -- LM p. 417, CM p. 334. To the scale factor routine's output
 * routine, through SFOUTABR, by BANKJUMP: "ALARM IF DEC DISP WITH OCTAL
 * ONLY NOUN" (PREDSPAL), no scaling (DSPDCEND), or one of the routines of
 * output_scale.c, which "ALL ... END" at DSPDCEND. */
static void decdsp3(void)
{
    ca(indexed_by(ADDRESS_OF(SFOUTABR), a_word()));
    tc(DECDSP3 + 2u, BANKJUMP);
}

/* DSPDCEND -- LM p. 417, CM p. 334. "ALL SFOUT ROUTINES END HERE": the
 * scaled word in MPAC, MPAC +1 on the DSKY in decimal (DSPDECWD, by
 * BANKCALL). */
void dspdcend(void)
{
    tc(DSPDCEND, BANKCALL); /* CADR DSPDECWD */
}

/* DSPDCEND +2, where DSPDECWD comes back (and 2INTOUT and M/SOUT go):
 * "MORE TO DISPLAY" while DECOUNT counts down, else the verb is done. */
void dspdcend_plus2(void)
{
    switch (ccs_at(ADDRESS_OF(DECOUNT))) {
    case POSITIVE: tc_at(DSPDCEND + 3u); break;                       /* TC +2 */
    case IS_PLUS_ZERO: tc(DSPDCEND + 4u, ADDRESS_OF(ENTRET)); return; /* TC ENTEXIT */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: tc(DSPDCEND + 6u, DSPDCPUT); return;
    }
    ts(ADDRESS_OF(DECOUNT));
    tc(DSPDCEND + 6u, DSPDCPUT);
}

/* ---- Verb 07 (LM p. 424, CM pp. 341-342) ---- */

/* DSPDPDEC -- "THIS IS A SPECIAL PURPOS VERB FOR DISPLAYING A DOUBLE
 * PRECISION AGC WORD AS 10 DECIMAL DIGITS ON THE AGC DISPLAY PANEL. IT
 * CAN BE USED WITH ANY NOUN, EXCEPT MIXED NOUNS." The double word NOUNADD
 * points at into MPAC, made to agree in sign (TPAGREE), for DSP2DEC in R1
 * and R2. The faithful port's step ends at TPAGREE, and so does this
 * one. */
void dspdpdec(void)
{
    if (mixed_noun(DSPDPDEC + 1u)) {
        tc(DSPDPDEC + 3u, DSPALARM);
        return;
    }
    tc_at(DSPDPDEC + 2u);                            /* TC +2: "NORMAL NOUN" */
    dca(indexed_by(0u, fetch(ADDRESS_OF(NOUNADD)))); /* EXTEND; INDEX NOUNADD; DCA 0 */
    dxch(ADDRESS_OF(MPAC));
    set_A(R1D1);
    ts(ADDRESS_OF(DSPCOUNT));
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 2u);
    tc(DSPDPDEC + 12u, TPAGREE); /* "MAKE DP DATA AGREE" */
}

/* DSPDPDEC +13, where TPAGREE (INTERPRETER.agc) comes back. */
void dspdpdec_plus13(void)
{
    tc(DSPDPDEC + 13u, DSP2DEC);
}

/* ENDDPDEC, where DSP2DEC comes back: the verb is done. */
void enddpdec(void)
{
    tc(ENDDPDEC, ADDRESS_OF(ENTRET)); /* TC ENTEXIT */
}
