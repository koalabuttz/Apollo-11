/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 4: the scale factor routines
 * for loading -- LM pp. 431-436, CM pp. 349-354. The faithful C is the
 * port's Pinball modules (c64-port/src/lanec_pw_*.c).
 *
 * PUTDCSF2 (load_verbs.c) sends each word keyed in decimal to its noun's
 * input routine, by BANKJUMP through SFINTABR, with the word in MPAC,
 * MPAC +1 and the scale factor constant in SFTEMP1, SFTEMP2. "SCALE
 * FACTORS FOR THOSE ROUTINES NEEDING THEM ARE AVAILABLE IN SFTEMP1. ALL
 * SFIN ROUTINES USE MPAC MPAC+1. LEAVE RESULT IN A. END WITH TC DECRET."
 * (through PUTCOM2): degrees (DEGINSF), a fraction of the constant
 * (ARTHINSF, ARTIN1SF), a binary whole number (BINROUND), a double word
 * (DPINSF, DPINSF2, DPINSF4), hours, minutes and seconds (HMSIN).
 * Comanche055 adds the optics angle (OPTDEGIN) and a double fraction
 * (DPFRACIN). Each alarms and recycles (ALMCYCLE) for a value out of
 * range.
 *
 * Bank 40, but HMSIN and its subroutines in bank 42. The faithful port
 * runs each routine to PUTCOM2 or ALMCYCLE in one step, so the labels it
 * enters on their own (SIGNFIX, ENDSCALE, 2ROUND, TESTOFUF, ...) are C
 * calls here, and each has its function too. Luminary099's ENDSCALE calls
 * the scale routine subroutines in bank 41 by BANKCALL, which ends a
 * function. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* The end of every input routine: back to PUTCOM2 in bank 41, by POSTJUMP
 * (Luminary099's ENDSCAL1, Comanche055's ENDSCALE). */
static void to_putcom2(void)
{
#if LUMINARY099
    tc(ENDSCAL1, POSTJUMP); /* CADR PUTCOM2 */
#else
    tc(ENDSCALE, POSTJUMP); /* CADR PUTCOM2 */
#endif
}

/* ---- The triple word subroutines (LM pp. 433-434, CM pp. 351-352) ---- */

/* 2ROUND and 2RNDEND -- LM p. 434, CM p. 352. MPAC +1 doubled (the third
 * word, MPAC +2, in A, rounds it), an overflow carried into MPAC, and one
 * out of MPAC set in MPAC +6. Each TS that overflows skips its TC Q. */
void two_round(void)
{
    xch(ADDRESS_OF(MPAC) + 1u);
    double_A();
    two_round_plus2();
}

void two_round_plus2(void)
{
    if (!ts(ADDRESS_OF(MPAC) + 1u)) {
        tc_return(Q_ADDRESS); /* "IF MPAC+1 DOES NOT OF/UF" */
        return;
    }
    ad_at(ADDRESS_OF(MPAC));
    if (!ts(ADDRESS_OF(MPAC))) {
        tc_return(Q_ADDRESS); /* "IF MPAC DOES NOT OF/UF" */
        return;
    }
    ts(ADDRESS_OF(MPAC) + 6u);
    two_rndend();
}

void two_rndend(void)
{
    tc_return(Q_ADDRESS);
}

/* TESTOFUF -- LM p. 434, CM p. 352. "RETURNS IF NO OF/UF" in MPAC +6;
 * otherwise "ALARM AND RECYCLE." True when it went back. */
static bool testofuf_returns(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC) + 6u)) {
    case POSITIVE: tc(TESTOFUF + 1u, ALMCYCLE); return false; /* "OF" */
    case IS_PLUS_ZERO: tc_return(Q_ADDRESS); return true;
    case NEGATIVE: tc(TESTOFUF + 3u, ALMCYCLE); return false; /* "UF" */
    case IS_MINUS_ZERO: break;
    }
    not_modelled(); /* MPAC +6 holds +1, -1 or +0 */
    return false;
}

void testofuf(void)
{
    (void)testofuf_returns();
}

/* ---- Degrees (LM pp. 431-432, CM pp. 349-350) ---- */

/* DEGINSF -- "DEGINSF APPLIES 1000/180 = 5.55555(10) = 5.43434(8)": "SF
 * ROUTINE FOR DEC DEGREES", "MULT BY 5.5 5(10)X2EXP-3" (DMP by DEGCON1),
 * then "THIS ROUNDS OFF MPAC+1 BEFORE SHIFT LEFT 3, AND CAUSES 360.00 TO
 * OF/UF WHEN SHIFTED LEFT AND ALARM." (half of the low bits shifted out,
 * by sign, through 2ROUND +2), and left 3 by TPSL1 (DEGINSF2). */
void deginsf(void)
{
    tc_at(DEGINSF);
    dmp(); /* ADRES DEGCON1 */
    switch (ccs_at(ADDRESS_OF(MPAC) + 1u)) {
    case POSITIVE:
        set_A(BIT11);
        tc_at(DEGINSF + 4u);
        break; /* TC +2 */
    case IS_PLUS_ZERO: tc_at(DEGINSF + 4u); break;
    case NEGATIVE: set_A(negate(BIT11)); break;
    case IS_MINUS_ZERO: break;
    }
    ad_at(ADDRESS_OF(MPAC) + 1u);
    tc_at(DEGINSF + 7u);
    two_round_plus2();
    tc_at(DEGINSF + 8u);
    tpsl1(); /* "LEFT 1" */
    deginsf2();
}

void signfix(void);

/* DEGINSF2 -- LM p. 431, CM p. 349. "LEFT 2", an alarm if that went out
 * of range ("RETURNS IF NO OF/UF"), "LEFT3"; then the angle in one's
 * complement made two's complement: "IF -, USE -MAGNITUDE +1", "IF -0,
 * USE +0". */
void deginsf2(void)
{
    tc_at(DEGINSF2);
    tpsl1();
    tc_at(DEGINSF2 + 1u);
    if (!testofuf_returns()) return;
    tc_at(DEGINSF2 + 2u);
    tpsl1();
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: tc_at(DEGINSF2 + 4u); break;     /* "IF +, GO TO SIGNFIX" */
    case IS_PLUS_ZERO: tc_at(DEGINSF2 + 5u); break; /* "IF +0, GO TO SIGNFIX" */
    case NEGATIVE: com();                           /* fall through */
    case IS_MINUS_ZERO: ts(ADDRESS_OF(MPAC)); break;
    }
    signfix();
}

static void endscale_minus1(void);

/* SIGNFIX -- LM pp. 431-432, CM pp. 349-350. The third shift's overflow
 * (MPAC +6) gives the sign: "IF OVERFLOW", SGNTO1; "IF UF FORCE SIGN TO 0
 * EXCEPT -180" (NEG180 for MPAC +0). */
void signfix(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC) + 6u)) {
    case POSITIVE:
        tc_at(SIGNFIX + 1u);
        sgnto1();
        return;
    case IS_PLUS_ZERO:
        tc_at(SIGNFIX + 2u);
        endscale();
        return; /* "NO OVERFLOW/UNDERFLOW" */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: tc(SIGNFIX + 4u, CCSHOLE); return;
    }
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: tc(SIGNFIX + 4u, CCSHOLE); return;
    case IS_PLUS_ZERO:
        tc_at(SIGNFIX + 5u);
        neg180();
        return;
    case NEGATIVE: tc_at(SIGNFIX + 6u); break; /* TC +1 */
    case IS_MINUS_ZERO: break;
    }
    xch(ADDRESS_OF(MPAC));
    mask(POSMAX);
    endscale_minus1();
}

/* ENDSCALE -1: the angle into MPAC. */
static void endscale_minus1(void)
{
    ts(ADDRESS_OF(MPAC));
    endscale();
}

/* NEG180 and SGNTO1 -- LM p. 432, CM p. 350. -180 degrees is -POSMAX;
 * "IF OF FORCE SIGN TO 1". */
void neg180(void)
{
    set_A(negate(POSMAX));
    tc_at(NEG180 + 1u);
    endscale_minus1();
}

void sgnto1(void)
{
    cs(ADDRESS_OF(MPAC));
    mask(POSMAX);
    cs(A_ADDRESS);
    tc_at(SGNTO1 + 3u);
    endscale_minus1();
}

#if LUMINARY099
static void norback(void);

/* ENDSCALE -- LM p. 432. "IF ROUTINE NO. IS NOT CDU DEGREES, THEN THIS IS
 * 360 - CDU DEGREES AND ANGLE IN MPAC MUST BE REPLACED BY 360 DEGREES
 * MINUS ITSELF." The routine's number comes from SFRUTMIX or SFRUTNOR, in
 * bank 41, by BANKCALL (SFMIXCAL, SFNORCAL). */
void endscale(void)
{
    if (mixed_noun(ENDSCALE + 1u)) {
        tc_at(ENDSCALE + 3u);
        tc(SFMIXCAL, BANKCALL); /* CADR SFRUTMIX */
        return;
    }
    tc_at(ENDSCALE + 2u); /* TC +3 */
    tc_at(ENDSCALE + 5u);
    tc(SFNORCAL, BANKCALL); /* CADR SFRUTNOR */
}

/* SFMIXCAL +2 and SFNORCAL +2, where SFRUTMIX and SFRUTNOR come back. */
void sfmixcal_plus2(void)
{
    tc_at(SFMIXCAL + 2u);
    tc_at(MIXBACK); /* TC +2 */
    norback();
}

void sfnorcal_plus2(void)
{
    tc_at(SFNORCAL + 2u);
    norback();
}

/* NORBACK: routine 2 is CDU degrees; any other (13, "TESTS AT END FOR
 * 360-CDU") has 360-CDU (output_scale.c) replace the angle. */
static void norback(void)
{
    cs(A_ADDRESS);
    ad(BIT2);
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        tc_at(NORBACK + 4u);
        three_sixty_cdu();
    }
    to_putcom2();
}
#else
/* ENDSCALE -- CM p. 350. Back to PUTCOM2. */
void endscale(void)
{
    to_putcom2();
}
#endif

/* ---- Fractions and whole numbers (LM pp. 432-433, CM p. 350) ---- */

/* ARTHINSF -- "SCALES MPAC, +1 BY SFTEMP1, SFTEMP2. ASSUMES POINT BETWEEN
 * HI AND LO PARTS OF SFCON. SHIFTS RESULTS LEFT BY 14. (BY TAKING RESULTS
 * FROM MPAC+1, MPAC+2)". A high word left over is "TOO LARGE A LOAD.
 * ALARM AND RECYCLE." */
void arthinsf(void)
{
    tc_at(ARTHINSF);
    dmp(); /* ADRES SFTEMP1 */
    xch(ADDRESS_OF(MPAC) + 2u);
    xch(ADDRESS_OF(MPAC) + 1u);
    xch(ADDRESS_OF(MPAC));
    if (!A_is_zero()) { /* EXTEND; BZF BINROUND */
        tc(ARTHINSF + 7u, ALMCYCLE);
        return;
    }
    binround();
}

/* BINROUND -- MPAC +2 rounded into MPAC +1 (2ROUND), "RETURNS IF NO
 * OF/UF". */
void binround(void)
{
    tc_at(BINROUND);
    two_round();
    tc_at(BINROUND + 1u);
    if (!testofuf_returns()) return;
    tc_at(BINROUND + 2u);
    to_putcom2();
}

/* ARTIN1SF -- LM p. 433, CM p. 350. "SCALES MPAC, +1 BY SFTEMP1, SFTEMP2.
 * ROUNDS MPAC+1 INTO MPAC." */
void artin1sf(void)
{
    tc_at(ARTIN1SF);
    dmp(); /* ADRES SFTEMP1 */
    tc_at(ARTIN1SF + 2u);
    binround();
}

#if !LUMINARY099
/* OPTDEGIN and OPDEGIN2 -- CM p. 350. The optics angle: negative is an
 * alarm; the bias (NEG.2) taken off, times DEGCON2, and rounded as DEGINSF
 * rounds, then on through DEGINSF2. */
void optdegin(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: tc_at(OPTDEGIN + 1u); break;     /* TC +4 */
    case IS_PLUS_ZERO: tc_at(OPTDEGIN + 2u); break; /* TC +3 */
    case NEGATIVE: tc(OPTDEGIN + 3u, ALMCYCLE); return;
    case IS_MINUS_ZERO: tc(OPTDEGIN + 4u, ALMCYCLE); return;
    }
    opdegin2();
}

void opdegin2(void)
{
    set_A(NEG_POINT_2);
    ads(ADDRESS_OF(MPAC));
    tc_at(OPDEGIN2 + 2u);
    dmp(); /* ADRES DEGCON2 */
    set_A(BIT12);
    ad_at(ADDRESS_OF(MPAC) + 1u);
    tc_at(OPDEGIN2 + 6u);
    two_round_plus2();
    tc_at(OPDEGIN2 + 7u);
    deginsf2();
}
#endif

/* ---- Double words (LM p. 433, CM p. 351) ---- */

/* DPINSF -- "SCALES MPAC, MPAC +1 BY SFTEMP1, SFTEMP. STORES LOW PART OF
 * RESULT IN (E SUBK) +1 OR E+1": the low word doubled (an overflow of the
 * TS skips CAF ZERO and carries into MPAC +1), rounded, and "RETURNS IF NO
 * OF/UF"; then the low part goes where the high part's address says, one
 * on, and the high part back through PUTCOM2. DPINSF2 and DPINSF4 enter at
 * DPINSF +2. */
void dpinsf(void)
{
    tc_at(DPINSF);
    dmp(); /* ADRES SFTEMP1 */
    dpinsf_plus2();
}

void dpinsf_plus2(void)
{
    xch(ADDRESS_OF(MPAC) + 2u);
    double_A();
    if (!ts(ADDRESS_OF(MPAC) + 2u)) set_A(ZERO);
    ad_at(ADDRESS_OF(MPAC) + 1u);
    tc_at(DPINSF + 7u);
    two_round_plus2();
    tc_at(DPINSF + 8u);
    if (!testofuf_returns()) return;
    dpinsf_plus9();
}

static void dpincom(void);

/* DPINSF +9 (Comanche055's DPFRACIN): "MIXED" NOUNADD +DECOUNT is E SUBK,
 * "NORMAL" NOUNADD (DPINORM) is E. */
void dpinsf_plus9(void)
{
    if (mixed_noun(DPINSF + 10u)) {
        ca(ADDRESS_OF(DECOUNT)); /* "MIXED NOUN" */
        dpincom();
        return;
    }
    tc_at(DPINSF + 11u);
    /* DPINORM */
    set_A(ZERO); /* "E +1 NORMAL" */
    tc_at(DPINORM + 1u);
    dpincom();
}

static void dpincom(void)
{
    ad_at(ADDRESS_OF(NOUNADD));
    if (ts(Q_ADDRESS)) not_modelled(); /* an overflow would skip XCH MPAC +1 */
    xch(ADDRESS_OF(MPAC) + 1u);
    ts(indexed_erasable(1u, Q)); /* INDEX Q; TS 1: "PLACE LOW PART IN (E SUBK) +1" */
    tc_at(DPINCOM + 5u);
    to_putcom2();
}

/* DPINSF2 and DPINSF4 -- LM p. 433, CM p. 351. "ASSUMES POINT BETWEEN BITS
 * 7-8 OF HIGH PART OF SF CONST.": shifted left 7 (TPLEFTN with 6), or
 * between bits 11-12, left 3 (with 2); then as DPINSF. */
void dpinsf2(void)
{
    tc_at(DPINSF2);
    dmp(); /* ADRES SFTEMP1 */
    set_A(SIX);
    tc_at(DPINSF2 + 3u);
    tpleftn(); /* "SHIFT LEFT 7." */
    tc_at(DPINSF2 + 4u);
    dpinsf_plus2();
}

void dpinsf4(void)
{
    tc_at(DPINSF4);
    dmp(); /* ADRES SFTEMP1 */
    set_A(TWO);
    tc_at(DPINSF4 + 3u);
    tpleftn(); /* "SHIFT LEFT 3." */
    tc_at(DPINSF4 + 4u);
    dpinsf_plus2();
}

/* ---- Hours, minutes and seconds (LM pp. 434-436, CM pp. 352-354) ---- */

static bool rnd_tst_returns(void);
static bool mpactst_returns(void);
static bool sizetst_returns(void);
static bool all3dec_returns(void);

/* HMSIN -- "IF ALL 3 WORDS WERE NOT LOADED, ALARM." Hours (XREG, XREGLP,
 * already in MPAC, MPAC +1), minutes (YREG) and seconds (ZREG), each times
 * WHOLECON, "(10EXP5/2EXP14)2EXP14", rounded to a whole number (RND/TST),
 * and checked: hours "G/ 745", minutes over 59, seconds over 59.99 alarm.
 * Hours times HRCON ("1 HOUR IN CENTI-SEC"), minutes times MINCON ("1
 * MINUTE IN CENTI-SEC"), and seconds in centiseconds added up in HITEMIN
 * and MPAC, made to agree in sign (TPAGREE), into the noun's double word;
 * then LOADLV. */
void hmsin(void)
{
    tc_at(HMSIN);
    if (!all3dec_returns()) return;
    tc_at(HMSIN + 1u);
    dmp(); /* ADRES WHOLECON */
    tc_at(HMSIN + 3u);
    if (!rnd_tst_returns()) return; /* "ROUND OFF TO WHOLE HRS IN MPAC+1." */
    set_A(ZERO);                    /* "ALARM IF MPAC NON ZERO (G/ 16383)." */
    ts(ADDRESS_OF(MPAC) + 2u);
    set_A(HRCON);
    ts(ADDRESS_OF(MPAC));
    set_A(HRCON_LO);
    xch(ADDRESS_OF(MPAC) + 1u);
    tc_at(HMSIN + 10u);
    shortmp();
    tc_at(HMSIN + 11u);
    if (!mpactst_returns()) return; /* "ALARM IF MPAC NON ZERO (G/ 745)" */
    dxch(ADDRESS_OF(MPAC) + 1u);    /* "STORE HOURS CONTRIBUTION" */
    dxch(ADDRESS_OF(HITEMIN));
    ca(ADDRESS_OF(YREG)); /* "PUT YREG, YREGLP INTO MPAC, +1." */
    lxch(ADDRESS_OF(YREGLP));
    dxch(ADDRESS_OF(MPAC));
    tc_at(HMSIN + 17u);
    dmp(); /* ADRES WHOLECON */
    tc_at(HMSIN + 19u);
    if (!rnd_tst_returns()) return; /* "ROUND OFF TO WHOLE MIN IN MPAC+1" */
    set_A(negate(MIN59));           /* "ALARM IF MPAC NON ZERO (G/16383)" */
    tc_at(HMSIN + 21u);
    if (!sizetst_returns()) return; /* "ALARM IF MPAC+1 G/ 59MIN" */
    xch(ADDRESS_OF(MPAC) + 1u);
    mp(MINCON);               /* "LEAVES MINUTES CONTRIBUTION IN A,L" */
    das(ADDRESS_OF(HITEMIN)); /* "ADD IN MINUTES CONTRIBUTION" */
    if (!A_is_zero()) {       /* EXTEND; BZF +2: "IF THIS DAS OVEFLOWS, G/ 745 HR, 39MIN" */
        tc(HMSIN + 28u, ALMCYCLE);
        return;
    }
    ca(ADDRESS_OF(ZREG)); /* "PUT ZREG, ZREGLP INTO MPAC, +1." */
    lxch(ADDRESS_OF(ZREGLP));
    dxch(ADDRESS_OF(MPAC));
    tc_at(HMSIN + 32u);
    dmp(); /* ADRES WHOLECON */
    tc_at(HMSIN + 34u);
    if (!rnd_tst_returns()) return; /* "ROUND OFF TO WHOLE CENTI-SEC IN MPAC+1" */
    set_A(negate(SEC59_99));        /* "ALARM IF MPAC NON ZERO (G/163.83 SEC)" */
    tc_at(HMSIN + 36u);
    if (!sizetst_returns()) return; /* "ALARM IF MPAC+1 G/59.99 SEC" */
    dxch(ADDRESS_OF(HITEMIN));      /* "ADD IN SECONDS CONTRIBUTION" */
    das(ADDRESS_OF(MPAC));
    if (!A_is_zero()) { /* "IF THIS DAS OVERFLOWS, G/ 745 HR, 39 MIN, 14.59 SEC." */
        tc(HMSIN + 41u, ALMCYCLE);
        return;
    }
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 2u);
    tc_at(HMSIN + 44u);
    tpagree();
    dxch(ADDRESS_OF(MPAC));
    dxch(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD)))); /* INDEX NOUNADD; DXCH 0 */
    tc(HMSIN + 48u, POSTJUMP);                              /* CADR LOADLV */
}

/* RND/TST and MPACTST -- LM p. 435, CM p. 353. "ROUNDS MPAC+2 INTO
 * MPAC+1. ALARMS IF MPAC NOT 0" (as DPINSF, each TS that overflows skips
 * its CAF ZERO, carrying on). True when it went back. */
static bool rnd_tst_returns(void)
{
    xch(ADDRESS_OF(MPAC) + 2u);
    double_A();
    if (!ts(ADDRESS_OF(MPAC) + 2u)) set_A(ZERO);
    ad_at(ADDRESS_OF(MPAC) + 1u);
    if (!ts(ADDRESS_OF(MPAC) + 1u)) set_A(ZERO);
    ad_at(ADDRESS_OF(MPAC)); /* "CANT OVFLOW" */
    xch(ADDRESS_OF(MPAC));
    return mpactst_returns();
}

void rnd_tst(void)
{
    (void)rnd_tst_returns();
}

static bool mpactst_returns(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: tc(MPACTST + 1u, ALMCYCLE); return false; /* "ALARM AND RECYCLE." */
    case NEGATIVE: tc(MPACTST + 3u, ALMCYCLE); return false;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    tc_return(Q_ADDRESS);
    return true;
}

void mpactst(void)
{
    (void)mpactst_returns();
}

/* SIZETST -- LM pp. 435-436, CM pp. 353-354. "CALLED WITH - CON IN A":
 * "MAG OF MPAC+1 G/ CON. ALARM AND RECYCLE.", "MAG OF MPAC+1 L/= CON"
 * goes back. */
static bool sizetst_returns(void)
{
    if (ts(ADDRESS_OF(MPAC) + 2u)) {
        ad(ONE); /* an overflow skips the CCS: AD ONE; TCF +2 */
    } else
        switch (ccs_at(ADDRESS_OF(MPAC) + 1u)) { /* "GET MAG OF MPAC+1" */
        case POSITIVE: ad(ONE); break;           /* TCF +2 */
        case IS_PLUS_ZERO: break;                /* TCF +2 */
        case NEGATIVE: ad(ONE); break;
        case IS_MINUS_ZERO: break;
        }
    ad_at(ADDRESS_OF(MPAC) + 2u); /* "MAG OF MPAC+1 - CON" */
    if (!A_is_zero_or_minus()) {  /* EXTEND; BZMF +2 */
        tc(SIZETST + 8u, ALMCYCLE);
        return false;
    }
    tc_return(Q_ADDRESS);
    return true;
}

void sizetst(void)
{
    (void)sizetst_returns();
}

/* ALL3DEC and FORCEV25 -- LM p. 436, CM p. 354. "ALL3DEC TESTS THAT ALL 3
 * WORDS ARE LOADED IN DEC (FOR HMSIN). ALARM IF NOT. (TEST THAT BITS 3,4,5
 * OF DECBRNCH ARE ALL = 1)." With all three, AD OCT34BAR leaves -0;
 * otherwise less. "FORCE VERB 25 TO BE EXECUTED BY RECYCLE IN CASE
 * OPERATOR EXECUTED A LOWER LOAD VERB." True when it went back. */
static bool all3dec_returns(void)
{
    set_A(negate(OCT34BAR)); /* "GET BITS 3,4,5 IN A" */
    mask_at(ADDRESS_OF(DECBRNCH));
    ad(OCT34BAR);
    switch (ccs_at(A_ADDRESS)) {
    case NEGATIVE:
        tc_at(ALL3DEC + 6u);
        forcev25();
        return false;
    case IS_MINUS_ZERO: break;
    case POSITIVE:
    case IS_PLUS_ZERO: not_modelled(); /* AD OCT34BAR leaves -0 or less */
    }
    tc_return(Q_ADDRESS);
    return true;
}

void all3dec(void)
{
    (void)all3dec_returns();
}

void forcev25(void)
{
    set_A(negate(OCT31));
    ts(ADDRESS_OF(VERBSAVE));
    tc(FORCEV25 + 2u, ALMCYCLE); /* "ALARM AND RECYCLE." */
}
