/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 3: the scale factor routines
 * for display -- LM pp. 417-424, CM pp. 334-341. The faithful C is the
 * port's Pinball modules (c64-port/src/lanec_pw_*.c, lanec_pin_head_*.c).
 *
 * DECDSP3 (display_verbs.c) sends each component of a decimal display to
 * its noun's output routine, by BANKJUMP through SFOUTABR, with the word
 * in MPAC and the scale factor constant in SFTEMP1, SFTEMP2. "THE
 * FOLLOWING IS A TYPICAL SF ROUTINE. IT USES MPAC. LEAVES RESULTS IN MPAC,
 * MPAC+1. ENDS WITH TC DSPDCEND": a fraction of the constant in decimal
 * (ARTOUTSF, AROUT1SF), degrees (DEGOUTSF), a double word (DP1OUTSF,
 * DP2OUTSF, DP3OUTSF, DPOUT), two small integers (2INTOUT), hours,
 * minutes and seconds (HMSOUT), minutes and seconds (M/SOUT). Luminary099
 * adds 360 - CDU angle (360-CDUO) and the landing radar position
 * (LRPOSOUT); Comanche055 the optics angle (OPDEGOUT) and a double
 * fraction (DPFRACOT).
 *
 * Bank 40, but HMSOUT, M/SOUT and their subroutines in bank 42. Each
 * function runs from a label, or from the word a call returns to, to a
 * call or jump to another routine or to the next label the faithful port
 * enters. The subroutines that come straight back (FIXRANGE, SETAUG,
 * DPOUT, READLO, SEPSEC, SEPSECNR, SEPMIN, and the arithmetic of other
 * files: SHORTMP, DMP, TPAGREE) are C calls. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define MPAC_PLUS(n) (ADDRESS_OF(MPAC) + (n))

/* SCOUTEND -- LM p. 419, CM p. 336. Back to DSPDCEND in bank 41. */
static void scoutend(void)
{
    tc(SCOUTEND, POSTJUMP); /* CADR DSPDCEND */
}

/* ---- Degrees (LM pp. 418-419, CM pp. 335-336) ---- */

static void degoutsf_plus1(void);

/* DEGOUTSF -- "DEGOUTSF SCALES BY .18 THE LOW 14 BITS OF ANGLE, ADDING .18
 * FOR NUMBERS IN THE NEGATIVE (AGC) RANGE." MPAC +2 is the index into
 * DEGTAB: 0 for .18, "SET INDEX FOR FULL SCALE." */
void degoutsf(void)
{
    set_A(ZERO);
    degoutsf_plus1();
}

/* FIXRANGE -- "IF MPAC IS + RETURN TO L+1. IF MPAC IS - RETURN TO L+2
 * AFTER MASKING OUT THE SIGN BIT". True for L+2. */
static bool fixrange(void)
{
    word back;

    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE:
    case IS_PLUS_ZERO: tc_return(Q_ADDRESS); return false;
    case NEGATIVE: break; /* TCF +1 */
    case IS_MINUS_ZERO: break;
    }
    set_A(negate(BIT15));
    mask_at(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(MPAC));
    back = (word)(Q + 1u);
    tc(FIXRANGE + 8u, back); /* INDEX Q; TC 1 */
    return true;
}

/* SETAUG -- "LOADS SFTEMP1 AND SFTEMP2 WITH THE DP AUGMENTER CONSTANT"
 * (DEGTAB +MPAC +2). */
static void setaug(void)
{
    dca(indexed_by(ADDRESS_OF(DEGTAB), fetch(MPAC_PLUS(2u))));
    dxch(ADDRESS_OF(SFTEMP1));
    tc_return(Q_ADDRESS);
}

/* DEGCOM -- "LOADS MULTIPLIER, DOES SHORTMP, AND ADDS AUGMENTER." DEGTAB
 * holds .18 and .45, double words. */
static void degcom(void)
{
    dca(indexed_by(ADDRESS_OF(DEGTAB), fetch(MPAC_PLUS(2u))));
    dxch(ADDRESS_OF(MPAC)); /* "ADJUSTED ANGLE IN A" */
    tc_at(DEGCOM + 4u);
    shortmp();
    dxch(ADDRESS_OF(SFTEMP1));
    das(ADDRESS_OF(MPAC));
    tc_at(DEGCOM + 7u);
    scoutend();
}

/* DEGOUTSF +1: the DEGTAB index in A. A negative angle has its sign bit
 * masked out and the augmenter added; a positive one adds nothing
 * ("SFTEMP1 AND 2 ARE 0"). */
static void degoutsf_plus1(void)
{
    ts(MPAC_PLUS(2u));
    tc_at(DEGOUTSF + 2u);
    if (fixrange()) {
        tc_at(DEGOUTSF + 4u);
        setaug(); /* "SET AUGMENTER ACCORDING TO C(MPAC +2)" */
    } else {
        tc_at(DEGOUTSF + 3u); /* TC +2: "NO AUGMENT NEEDED" */
    }
    tc_at(DEGOUTSF + 5u);
    degcom();
}

#if LUMINARY099
/* 360-CDUO and 360-CDU -- LM p. 418. "360-CDUD COMPUTES 360 - CDU ANGLE IN
 * MPAC, STORES RESULT IN MPAC AND GOES TO DEGOUTSF." "IF ANGLE IS 0 OR
 * 180 DEGREES, DO NOTHING". */
void three_sixty_cduo(void)
{
    tc_at(THREE_SIXTY_CDUO);
    three_sixty_cdu();
    tc(THREE_SIXTY_CDUO + 1u, DEGOUTSF);
}

void three_sixty_cdu(void)
{
    ca(ADDRESS_OF(MPAC));
    mask(POSMAX);
    if (!A_is_zero()) {       /* EXTEND; BZF 360-CDUE */
        cs(ADDRESS_OF(MPAC)); /* "COMPUTE 360 DEGREES MINUS ANGLE" */
        ad(ONE);
        if (ts(ADDRESS_OF(MPAC))) not_modelled(); /* an overflow would skip TC Q */
    }
    three_sixty_cdue();
}

void three_sixty_cdue(void)
{
    tc_return(Q_ADDRESS);
}

/* LRPOSOUT -- LM p. 418. "LRPOSOUT DISPLAYS +0,1,2, OR 3 (WHOLE) FOR
 * CHANNEL 33, BITS 7-6 = 11,10, 01,00 RESPECTIVELY", "AS WHOLE" through
 * ARTOUTSF. */
void lrposout(void)
{
    read_channel(CHAN33);
    mp(BIT10); /* "BITS 7-6 TO BITS 2-1" */
    com();
    mask(THREE);
    ts(ADDRESS_OF(MPAC));
    tc(LRPOSOUT + 7u, ARTOUTSF);
}
#else
/* OPDEGOUT, BIASCOM and NEGOPT -- CM p. 335. "OPDEGOUT SCALES BY .45 (THE
 * RANGE IS 90 DEGREES) AND ADDS A 20 DEG BIAS." The bias added in one's
 * complement: a negative angle's sum less one when it comes out positive
 * ("BECAUSE OF 2SCOM"), an overflow put back with BIT15, and DEGOUTSF +1
 * with the index 2, for .45. */
static void biascom(void)
{
    if (!ts(ADDRESS_OF(MPAC))) {
        tc_at(BIASCOM + 1u); /* TC +3: "NO OVFLOW" */
    } else {
        set_A(BIT15); /* "IF OVFLOW" */
        ads(ADDRESS_OF(MPAC));
    }
    set_A(TWO); /* "SET MULTIPLIER TO .45" */
    tc_at(BIASCOM + 5u);
    degoutsf_plus1();
}

static void negopt(void)
{
    xch(ADDRESS_OF(MPAC)); /* "NEGATIVE CASE" */
    ad(TWENTY_BIAS);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_at(NEGOPT + 3u); break; /* "IF POS THEN SUBTRACT 1 BECASUE OF 2SCOM" */
    case IS_PLUS_ZERO: tc(NEGOPT + 4u, CCSHOLE); return;
    case NEGATIVE:
        ad(ONE); /* "IF NEG RESTORE SUM" */
        /* fall through */
    case IS_MINUS_ZERO:
        com(); /* "IF NEG 0 LEAVE NEG 0" */
        tc_at(NEGOPT + 7u);
        break;
    }
    biascom();
}

void opdegout(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) { /* "RANGE IS 90 DEG" */
    case POSITIVE:
        xch(ADDRESS_OF(MPAC)); /* "IF POS OR POS 0 THEN ADD BIAS" */
        /* fall through */
    case IS_PLUS_ZERO: tc_at(OPDEGOUT + 2u); break; /* TC +3 */
    case NEGATIVE:
        tc_at(OPDEGOUT + 3u);
        negopt();
        return;                          /* "IF NEG NON ZERO" */
    case IS_MINUS_ZERO: ad(NEG1); break; /* "IF NEG ZERO SUBTRACT 1" */
    }
    ad(TWENTY_BIAS);
    biascom();
}
#endif

/* ---- A fraction of the constant (LM p. 419, CM p. 336) ---- */

static void l14_out(void);

/* ARTOUTSF -- "ASSUMES POINT AT LEFT OF DP SFCON": the word times the
 * constant, by PRSHRTMP ("IF C(A) = -0, SHORTMP FAILS TO GIVE -0."). */
void artoutsf(void)
{
    dxch(ADDRESS_OF(SFTEMP1));
    dxch(ADDRESS_OF(MPAC));
    tc_at(ARTOUTSF + 2u);
    prshrtmp();
    scoutend();
}

/* AROUT1SF -- "ASSUMES POINT BETWEEN HI AND LO PARTS OF DP SFCON. SHIFTS
 * RESULTS LEFT 14, BY TAKING RESULTS FROM MPAC+1, MPAC+2." */
void arout1sf(void)
{
    dxch(ADDRESS_OF(SFTEMP1));
    dxch(ADDRESS_OF(MPAC));
    tc_at(AROUT1SF + 2u);
    prshrtmp();
    tc_at(AROUT1SF + 3u);
    l14_out();
}

/* ---- A double word (LM pp. 419-420, CM pp. 336-337) ---- */

/* DPOUT -- LM p. 420, CM p. 337. MPAC, MPAC +1 times the double constant
 * in SFTEMP1, SFTEMP2: the word read fresh, high and low (READLO), "MAKE
 * DP DATA AGREE" (TPAGREE), and DMP. "USE MPAC +6 INSTEAD OF OVFIND" for
 * the return. */
static void dpout(void)
{
    xch(Q_ADDRESS);
    if (ts(MPAC_PLUS(6u))) not_modelled(); /* an overflow in Q would skip TC READLO */
    tc_at(DPOUT + 2u);
    readlo();
    tc_at(DPOUT + 3u);
    tpagree();
    tc_at(DPOUT + 4u);
    dmp(); /* ADRES SFTEMP1 */
    tc_return(MPAC_PLUS(6u));
}

/* DP1OUTSF and L14/OUT -- "SCALES MPAC, MPAC +1 BY DP SCALE FACTOR IN
 * SFTEMP1, SFTEMP2. THEN SCALE RESULT BY B14". */
void dp1outsf(void)
{
    tc_at(DP1OUTSF);
    dpout();
    l14_out();
}

static void l14_out(void)
{
    xch(MPAC_PLUS(2u));
    xch(MPAC_PLUS(1u));
    ts(ADDRESS_OF(MPAC));
    tc_at(L14_OUT + 3u);
    scoutend();
}

/* DP2OUTSF -- "SCALES MPAC, MPAC +1 BY DP SCALE FACTOR". */
void dp2outsf(void)
{
    tc_at(DP2OUTSF);
    dpout();
    tc_at(DP2OUTSF + 1u);
    scoutend();
}

/* DP3OUTSF -- "ASSUMES POINT BETWEEN BITS 7-8 OF HIGH ... LEFT BY 7, ROUNDS
 * MPAC+2 INTO MPAC+1.": the product shifted left 7 (TPLEFTN with 6). */
void dp3outsf(void)
{
    tc_at(DP3OUTSF);
    dpout();
    set_A(SIX);
    tc_at(DP3OUTSF + 2u);
    tpleftn(); /* "SHIFT LEFT 7." */
    tc_at(DP3OUTSF + 3u);
    scoutend();
}

#if !LUMINARY099
/* DPFRACOT -- CM p. 337. "DP FRACTION TO MPAC,+1", unscaled. */
void dpfracot(void)
{
    tc_at(DPFRACOT);
    readlo();
    tc_at(DPFRACOT + 1u);
    scoutend();
}
#endif

/* ---- Two integers (LM p. 420, CM p. 337) ---- */

/* 2INTOUT -- "THE FOLLOWING ROUTINE DISPLAYS TWO CONTIGUOUS SP POSITIVE
 * INTEGERS AS TWO POSITIVE DECIMAL INTEGERS IN RXD1-RXD2 AND RXD4-RXD5
 * (RXD3 IS BLANKED). THE INTEGER IN THE LOWER NUMBERED ADDRESS IS
 * DISPLAYED IN RXD1-RXD2." Each step is a call that comes back to the
 * next: 5BLANK "TO BLANK RXD3", +ON "TURN ON + SIGN", DSPDECVN for each
 * integer "(LIKE VERB AND NOUN)". */
void two_intout(void)
{
    tc(TWO_INTOUT, FIVE_BLANK);
}

void two_intout_plus1(void)
{
    tc(TWO_INTOUT + 1u, PLUS_ON);
}

void two_intout_plus2(void)
{
    ca(ADDRESS_OF(MPAC));
    tc(TWO_INTOUT + 3u, DSPDECVN); /* "DISPLAY 1ST INTEGER" */
}

void two_intout_plus4(void)
{
    set_A(negate(THREE));
    ad_at(indexed_by(ADDRESS_OF(R1D1), fetch(ADDRESS_OF(DECOUNT)))); /* "RXD4" */
    ts(ADDRESS_OF(DSPCOUNT));
    tc_at(TWO_INTOUT + 8u);
    readlo(); /* "GET 2ND INTEGER" */
    ca(MPAC_PLUS(1u));
    tc(TWO_INTOUT + 10u, DSPDECVN); /* "DISPLAY 2ND INTEGER" */
}

void two_intout_plus11(void)
{
    tc(TWO_INTOUT + 11u, POSTJUMP); /* CADR DSPDCEND +2 */
}

/* ---- READLO (LM pp. 420-421, CM pp. 337-338) ---- */

/* READLO, READLO1 and RDLONOR -- "READLO PICKS UP FRESHDATA FOR BOTH HI AND
 * LO AND LEAVES IT IN MPAC, MPAC+1. THIS IS NEEDED FOR TIME DISPLAY. IT
 * ZEROES MPAC+2, BUT DOES NOT FORCE TPAGREE." The word at NOUNADD for a
 * normal noun; for a mixed one the component's own address, an ECADR from
 * IDAD1TEM +DECOUNT ("E SUBK", SETEBANK). The return in TEM4. */
void readlo(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(TEM4))) not_modelled(); /* an overflow in Q would skip the INDEX */
    if (mixed_noun(READLO + 3u)) {
        ca(indexed_by(ADDRESS_OF(IDAD1TEM), fetch(ADDRESS_OF(DECOUNT))));
        mask(LOW11); /* "E SUBK" */
        tc_at(READLO + 8u);
        setebank(); /* "SET EB, LEAVE EADRES IN A." */
    } else {
        tc_at(READLO + 4u);
        ca(ADDRESS_OF(NOUNADD)); /* RDLONOR: "E" */
        tc_at(ENDRDLO);
    }
    /* READLO1 */
    dca(indexed_by(0u, a_word())); /* EXTEND; INDEX A; DCA 0 */
    dxch(ADDRESS_OF(MPAC));
    set_A(ZERO);
    ts(MPAC_PLUS(2u));
    tc_return(ADDRESS_OF(TEM4));
}

/* ---- Hours, minutes and seconds (LM pp. 421-424, CM pp. 338-341) ---- */

/* SEPSECNR -- LM pp. 423-424, CM pp. 340-341. "THIS ENTRY AVOIDS ROUNDING
 * BY .5 SEC". The time, in centiseconds, times 2EXP12/6000 (SECON1): the
 * whole minutes and hours kept in HITEMOUT, LOTEMOUT ("SAVE MINUTES AND
 * HOURS"), and the product shifted left 2 (TPSL1 twice) "GIVES FRACT
 * SEC/60 IN MPAC+1, MPAC+2", which is moved up: "LEAVE FRACT SEC/60 IN
 * MPAC, MPAC+1." The return in SEPSCRET. SEPSECNR +1 takes it in A. */
static void sepsecnr_plus1(void)
{
    if (ts(ADDRESS_OF(SEPSCRET))) not_modelled(); /* an overflow would skip TC DMP */
    tc_at(SEPSECNR + 2u);
    dmp(); /* ADRES SECON1: "MULT BY 2EXP12/6000" */
    dca(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(HITEMOUT)); /* "SAVE MINUTES AND HOURS" */
    tc_at(SEPSECNR + 7u);
    tpsl1();
    tc_at(SEPSECNR + 8u);
    tpsl1();
    set_A(ZERO);
    xch(MPAC_PLUS(2u));
    xch(MPAC_PLUS(1u));
    xch(ADDRESS_OF(MPAC));
    tc_return(ADDRESS_OF(SEPSCRET));
}

static void sepsecnr(void)
{
    xch(Q_ADDRESS);
    sepsecnr_plus1();
}

/* SEPSEC -- LM p. 423, CM p. 340. "IF +, ROUND BY ADDING .5 SEC", "IF -,
 * ROUND BY SUBTRACING .5 SEC" (RNDCON -1, RNDCON: the double word .5
 * SEC), then SEPSECNR. "FINDS TIME IN MPAC, MPAC+1", "LEAVES WHOLE MIN IN
 * BIT13 OF LOTEMOUT AND ABOVE." */
static void sepsec(void)
{
    switch (ccs_at(MPAC_PLUS(1u))) {
    case POSITIVE:
    case IS_PLUS_ZERO:
        dca(ADDRESS_OF(RNDCON) - 1u); /* POSEC */
        break;
    case NEGATIVE: /* TCF +1 */
    case IS_MINUS_ZERO: dcs(ADDRESS_OF(RNDCON) - 1u); break;
    }
    das(ADDRESS_OF(MPAC)); /* SEPSEC1 */
    sepsecnr();            /* TCF SEPSECNR */
}

/* SEPMIN -- LM p. 424, CM p. 341. "FIND WHOLE MINUTES IN BIT13 OF LOTEMOUT
 * AND ABOVE. REMOVES REST OF SECONDS. LEAVES FRACT MIN/60 IN MPAC+1.
 * LEAVES WHOLE HOURS IN MPAC.": LOTEMOUT shifted right 12 and back left 12
 * ("SR 12, THROW AWAY LP." "SR 2, TAKE FROM LP. = SL 12."), with
 * HITEMOUT, times 1/15 (MINCON1). The return in SEPMNRET. */
static void sepmin(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(SEPMNRET))) not_modelled(); /* an overflow would skip CA LOTEMOUT */
    ca(ADDRESS_OF(LOTEMOUT));
    mp(BIT3);
    mp(BIT13);
    lxch(MPAC_PLUS(1u)); /* "THIS FORCES BITS 12-1 TO 0 IF +." */
    ca(ADDRESS_OF(HITEMOUT));
    ts(ADDRESS_OF(MPAC));
    tc_at(SEPMIN + 10u);
    dmp();                           /* ADRES MINCON1: "MULT BY 1/15" */
    tc_return(ADDRESS_OF(SEPMNRET)); /* "GIVES WHOLE HOURS IN MPAC." */
}

/* HMSOUT -- LM p. 421, CM p. 338. Hours in R1, minutes in R2, seconds in
 * R3, from a double word of centiseconds, each by DSPDECWD (by BANKCALL,
 * from bank 42): "READ FRESH DATA FOR HI AND LO INTO MPAC, MPAC+1." */
void hmsout(void)
{
    tc(HMSOUT, BANKCALL); /* CADR READLO */
}

/* HMSOUT +2: "MAKE DP DATA AGREE.", "LEAVE FRACT SEC/60 IN MPAC, MPAC+1.
 * LEAVE WHOLE MIN IN BIT13 OF LOTEMOUT AND ABOVE" (SEPSECNR, which comes
 * back to HMSOUT +4). */
void hmsout_plus2(void)
{
    tc_at(HMSOUT + 2u);
    tpagree();
    tc_at(HMSOUT + 3u);
    sepsecnr();
}

/* HMSOUT +4: "USE ONLY FRACT SEC/60 MOD 60", "MULT BY .06" (SECON2):
 * "GIVES CENTI-SEC/10EXP5 MOD 60", for R3. */
void hmsout_plus4(void)
{
    tc_at(HMSOUT + 4u);
    dmp(); /* ADRES SECON2 */
    set_A(R3D1);
    ts(ADDRESS_OF(DSPCOUNT));
    continue_at(HMSOUT + 8u);
}

void hmsout_plus8(void)
{
    tc(HMSOUT + 8u, BANKCALL); /* CADR DSPDECWD: "DISPLAY SEC MOD 60" */
}

/* HMSOUT +10: "REMOVE REST OF SECONDS" (SEPMIN). "LEAVE FRACT MIN/60 IN
 * MPAC+1. LEAVE WHOLE HOURS IN MPAC.": the hours saved in HITEMOUT, and
 * the minutes "MULT BY .0006" (MINCON2): "GIVE MIN/10EXP5 MOD 60", for
 * R2. */
void hmsout_plus10(void)
{
    tc_at(HMSOUT + 10u);
    sepmin();
    set_A(MINCON2);
    xch(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(HITEMOUT)); /* "SAVE WHOLE HOURS." */
    set_A(MINCON2_LO);
    xch(MPAC_PLUS(1u)); /* "USE ONLY FRACT MIN/60 MOD 60" */
    tc_at(HMSOUT + 16u);
    prshrtmp();
    set_A(R2D1);
    ts(ADDRESS_OF(DSPCOUNT));
    continue_at(HMSOUT + 19u);
}

void hmsout_plus19(void)
{
    tc(HMSOUT + 19u, BANKCALL); /* CADR DSPDECWD: "DISPLAY MIN MOD 60" */
}

/* HMSOUT +21: "MINUTES, SECONDS HAVE BEEN REMOVED": the whole hours "MULT
 * BY .16384" (HRCON1): "GIVES HOURS/10EXP5", for R1. */
void hmsout_plus21(void)
{
    dca(ADDRESS_OF(HRCON1));
    dxch(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(HITEMOUT)); /* "USE WHOLE HOURS" */
    tc_at(HMSOUT + 25u);
    prshrtmp();
    set_A(R1D1);
    ts(ADDRESS_OF(DSPCOUNT));
    continue_at(HMSOUT + 28u);
}

void hmsout_plus28(void)
{
    tc(HMSOUT + 28u, BANKCALL); /* CADR DSPDECWD: "USE REGULAR DSPDECWD, WITH ROUND OFF." */
}

void hmsout_plus30(void)
{
    tc(HMSOUT + 30u, ADDRESS_OF(ENTRET)); /* TC ENTEXIT */
}

/* M/SOUT -- LM p. 422, CM p. 339. Minutes and seconds, MMBSS in one
 * register: "READ FRESH DATA FOR HI AND LO INTO MPAC. MPAC+1." */
void m_sout(void)
{
    tc(M_SOUT, BANKCALL); /* CADR READLO */
}

/* TC M/SNORM at at, and M/SNORM's SEPSEC: "LEAVE FRACT SEC/60 IN
 * MPAC,MPAC+1. LEAVE WHOLE MIN IN BIT13 OF LOTEMOUT AND ABOVE", back at
 * M/SNORM +1. */
static void to_m_snorm(word at)
{
    tc_at(at);
    tc_at(M_SNORM);
    sepsec();
}

/* M/SLIMIT, +LIMIT and LIMITCOM: "FORCE MPAC, MPAC+1 TO +/- 59M59.5S"
 * ("WILL DISPLAY 59M59S IN DSPDECNR"), and SEPSECNR without the rounding,
 * back at M/SNORM +1 (NORMADR). */
static void m_slimit(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) { /* "MPAC CANNOT BE +/- 0 AT THIS POINT." */
    case POSITIVE: set_A(M_SCON3);      /* fall through */
    case IS_PLUS_ZERO:
        tc_at(M_SLIMIT + 2u); /* +LIMIT */
        ts(ADDRESS_OF(MPAC));
        set_A(M_SCON3_LO);
        tc_at(PLUS_LIMIT + 2u); /* TC LIMITCOM */
        break;
    case NEGATIVE: set_A(negate(M_SCON3)); /* fall through */
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(MPAC));
        set_A(negate(M_SCON3_LO));
        break;
    }
    /* LIMITCOM */
    ts(MPAC_PLUS(1u));
    set_A(NORMADR); /* "SET RETURN TO M/SNORM+1." */
    tc_at(LIMITCOM + 2u);
    sepsecnr_plus1();
}

/* The CCS A of a limit test at M/SOUT +b +4: the address of the TC it
 * takes, or 0 for -0 ("= 59M58.5S"). */
static word limit_ccs(word b)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: return M_SOUT + b + 5u; /* "G/ 59M58.5S": TC M/SLIMIT */
    case IS_PLUS_ZERO: return M_SOUT + b + 6u;
    case NEGATIVE: return M_SOUT + b + 7u; /* "L/ 59M58.5S" */
    case IS_MINUS_ZERO: break;
    }
    return 0u;
}

/* One word of MPAC, MPAC +1 against the same word of 59M58.5S, from the
 * CCS of the word at M/SOUT +b: CCS; TC +2; TC M/SNORM; AD con; CCS A;
 * TC M/SLIMIT; TC M/SNORM; TC M/SNORM. con is "- HI (or LO) PART OF
 * (59M58.5S) +1 FOR CCS". The address of the TC taken, or 0 when the
 * words are the same ("MAG OF MPAC = HI PART OF 59M58.5S"). */
static word limit_test(unsigned word_address, word b, word con)
{
    switch (ccs_at(word_address)) {
    case POSITIVE: tc_at(M_SOUT + b + 1u); break; /* TC +2 */
    case IS_PLUS_ZERO: return M_SOUT + b + 2u;    /* "MPAC = +0. L/ 59M58.5S" */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: return limit_ccs(b); /* on to CCS A, with +0 */
    }
    ad(con);
    return limit_ccs(b);
}

/* M/SOUT +2: "MAKE DP DATA AGREE", and "IF MAG OF (MPAC, MPAC+1) G/ 59 M
 * 59 S. DISPLAY 59B59, WITH PROPER SIGN.": the high word against
 * M/SCON1 and, if the same, the low word against M/SCON2. Above or at
 * the limit, M/SLIMIT; below it, M/SNORM. */
void m_sout_plus2(void)
{
    word at;

    tc_at(M_SOUT + 2u);
    tpagree();
    at = limit_test(ADDRESS_OF(MPAC), 3u, M_SCON1);
    if (at == 0u) at = limit_test(MPAC_PLUS(1u), 11u, M_SCON2);
    if (at == 0u) { /* "= 59M58.5S LIMIT" */
        m_slimit();
        return;
    }
    if (at == M_SOUT + 8u || at == M_SOUT + 16u) { /* "G/ 59M58.5S" */
        tc_at(at);
        m_slimit();
        return;
    }
    to_m_snorm(at); /* "L/ 59M58.5S" */
}

/* M/SNORM +1: "USE ONLY FRACT SEC/60 MOD 60", "MULT BY .6 + 2EXP-14"
 * (HISECON): "GIVES SEC/100 MOD 60", for the register's last two digits
 * ("DSPCOUNT ALREADY SET TO RXD1"). */
void m_snorm_plus1(void)
{
    set_A(HISECON);
    tc_at(M_SNORM + 2u);
    shortmp();
    set_A(negate(THREE));
    ads(ADDRESS_OF(DSPCOUNT));
    continue_at(M_SNORM + 5u);
}

void m_snorm_plus5(void)
{
    tc(M_SNORM + 5u, BANKCALL); /* CADR DSPDC2NR: "DISPLAY SEC MOD 60 IN D4D5." */
}

/* M/SNORM +7: "BLANK MIDDLE CHAR" (RXD3) by DSPIN. */
void m_snorm_plus7(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CODE));
    set_A(negate(TWO));
    ad_at(indexed_by(ADDRESS_OF(R1D1), fetch(ADDRESS_OF(DECOUNT)))); /* "RXD3" */
    ts(ADDRESS_OF(COUNT));
    tc(M_SNORM + 13u, BANKCALL); /* CADR DSPIN */
}

/* M/SNORM +15: "REMOVE REST OF SECONDS" (SEPMIN), "LEAVE FRACT MIN/60 IN
 * MPAC+1", "MULT BY .6 + 2EXP-7" (HIMINCON): "GIVES MIN/100 MOD 60", for
 * the first two digits (RXD1). */
void m_snorm_plus15(void)
{
    tc_at(M_SNORM + 15u);
    sepmin();
    xch(MPAC_PLUS(1u));
    mp(HIMINCON);
    dxch(ADDRESS_OF(MPAC));
    ca(indexed_by(ADDRESS_OF(R1D1), fetch(ADDRESS_OF(DECOUNT)))); /* "RXD1" */
    ts(ADDRESS_OF(DSPCOUNT));
    continue_at(M_SNORM + 23u);
}

void m_snorm_plus23(void)
{
    tc(M_SNORM + 23u, BANKCALL); /* CADR DSPDC2NR: "DISPLAY MIN MOD 60 IN D1D2." */
}

void m_snorm_plus25(void)
{
    tc(M_SNORM + 25u, POSTJUMP); /* CADR DSPDCEND +2 */
}
