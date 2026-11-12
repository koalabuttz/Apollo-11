/* SPDX-License-Identifier: MIT */
/* AOSTASK_AND_AOSJOB.agc, the LM's: Luminary099 pages 1485-1506, bank 20,
 * its erasable words in EBANK 6 (EBANK= AOSQ) once 1/ACCS has set it. The
 * faithful C is c64-port/src/lm_aostask.c.
 *
 * A TS whose A holds an overflow stores the corrected word, leaves +1 or
 * -1 in A and skips the next instruction. Where an overflow can reach a
 * TS here, the code says so: if (!ts(X)) next instruction. */
#include "aostask.h"

#if LUMINARY099

/* An erasable word, by its address, at an index: INDEX index on it. */
static unsigned at_index(unsigned address, unsigned index)
{
    return indexed(address, fetch(index));
}

/* The MPAC words the file names by EQUALS (DOCKTEMP, EPSILON, UV and the
 * rest are MPAC +n) are named by ADDRESS_OF as the listing names them. */
#define MPAC_1 (ADDRESS_OF(MPAC) + 1u)
#define MPAC_2 (ADDRESS_OF(MPAC) + 2u)

/* ---- INVERT -- LM p. 1504 ---- */

/* INVERT's work: "ROUTINE TO INVERT -INPUT AT PI/2", 1 at 2(6) over the
 * word in A: "RESULT AT 2(7)/PI". "ZERO L FOR ACCURACY AND TO PREVENT
 * OVFLO". A TS HOLD that overflows skips CA BIT9. */
static void invert_work(void)
{
    if (!ts(ADDRESS_OF(HOLD))) set_A(BIT9);
    zl();
    dv_at(ADDRESS_OF(HOLD));
}

/* TC INVERT at at, from this file: back at at +1. */
static void tc_invert(word at)
{
    tc_at(at);
    invert_work();
    tc_through(Q_ADDRESS);
}

/* INVERT -- LM p. 1504, by TC from outside the run: its TC Q goes back to
 * the caller. Only the file's own TCs call INVERT, and those run here
 * inline, so a return into the file's code (1/ACCSET on) is not modelled;
 * nor one outside 010-07777. */
void invert(void)
{
    word to;

    invert_work();
    to = overflow_corrected(Q);
    if (to < 010u || to > 07777u || (to >= ONE_ACCSET && to < 04000u)) not_modelled();
    Q = 3u;
    correct_L();
    continue_at(to);
}

/* ---- DVOVSUB -- LM pp. 1492-1493 ---- */

/* DVOVSUB -- LM p. 1492, by TC at at, the ADRES of the divisor after the TC:
 * A,L over the divisor, "QUOTIENT WITH OVERFLOW PROTECTION". "RETURN POSMAX
 * FOR A ZERO DIVISOR." When "DIVISION IS NOT PROPER" (ABS(DIVISOR) not above
 * ABS(DIVIDEND)), MAKEMAX gives POSMAX for like signs and NEGMAX for unlike
 * ones (the dividend's high word and the divisor are not zero there).
 * Returns by TC Q to the word after the ADRES. */
static void dvovsub(word at)
{
    tc_at(at);
    ts(ADDRESS_OF(SCRATCHY)); /* "SAVE UPPER HALF OF DIVIDEND" */
    ts(ADDRESS_OF(SCRATCHX));
    ca(overflow_corrected(Q)); /* "OBTAIN ADDRESS OF DIVISOR." */
    incr(Q_ADDRESS);           /* "STEP Q FOR PROPER RETURN SEQUENCE." */
    ca(a_word());              /* "PICK UP THE DIVISOR." */
    if (A_is_zero()) {
        set_A(POSMAX); /* MAXPLUS */
    } else {
        ts(ADDRESS_OF(SCRATCHZ));
        (void)ccs_at(A_ADDRESS); /* "GET ABS(DIVISOR) IN THE A REGISTER." */
        ad(BIT1);
        xch(ADDRESS_OF(SCRATCHY));        /* ZEROPLUS: "STORE ABS(DIVISOR)." */
        if (!A_is_zero_or_minus()) com(); /* "GET -ABS(DIVIDEND)" */
        ad_at(ADDRESS_OF(SCRATCHY));      /* GOODNEG: "ABS(DIVISOR) - ABS(DIVIDEND)" */
        if (!A_is_zero_or_minus()) {
            ca(ADDRESS_OF(SCRATCHX)); /* "RE-ESTABLISH THE DIVIDEND" */
            dv_at(ADDRESS_OF(SCRATCHZ));
        } else {
            /* MAKEMAX: "DETERMINE THE SIGN OF THE QUOTIENT." */
            bool dividend_up = ccs_at(ADDRESS_OF(SCRATCHX)) == POSITIVE;
            bool divisor_up = ccs_at(ADDRESS_OF(SCRATCHZ)) == POSITIVE;
            set_A(dividend_up == divisor_up ? POSMAX : NEGMAX);
        }
    }
    tc_through(Q_ADDRESS);
}

/* ---- 1/ACCSET, 1/ACCJOB, 1/ACCS -- LM p. 1486 ---- */

/* 1/ACCJOB -- LM p. 1486: "1/ACCS ASSUMES ENTRY VIA BANKCALL." The CADR
 * after the TC names 1/ACCS +2: "SKIP EBANK SETTING." */
void one_accjob(void)
{
    tc_to(ONE_ACCJOB, BANKCALL);
}

/* 1/ACCSET -- LM p. 1486: "ENTRY FROM FRESH START/RESTART CODING." "NULL THE
 * OFFSET ESTIMATES FOR 1/ACCS." and "FOR DOWNLIST", then 1/ACCJOB. */
void one_accset(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(AOSQ));
    ts(ADDRESS_OF(AOSR));
    ts(ADDRESS_OF(ALPHAQ));
    ts(ADDRESS_OF(ALPHAR));
    one_accjob();
}

/* 1/ACCJOB +2 -- LM p. 1486, where BANKCALL brings 1/ACCS back: the job
 * ends. */
void one_accjob_return(void)
{
    tc_to(ONE_ACCJOB + 2u, ENDOFJOB);
}

/* 1/ACCS +2 -- LM p. 1486: TC MAKECADR, "SAVE RETURN SO THAT BUF2 MAY BE
 * USED". */
void one_accs_plus2(void)
{
    tc_to(ONE_ACCS + 2u, MAKECADR);
}

/* 1/ACCS -- LM p. 1486: "***** EBANK SET BUT NOT RESTORED *****" */
void one_accs(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    one_accs_plus2();
}

/* ---- 1/ACCRET, DOCKTEST, DOWNGTS -- LM pp. 1491, 1504 ---- */

/* 1/ACCRET +1 -- LM p. 1504: "SET BIT TO INDICATE DATA GOOD.", then "RETURN
 * TO CALLER" by BANKJUMP to the CADR MAKECADR made. */
static void one_accret_plus1(void)
{
    cs(ADDRESS_OF(DAPBOOLS));
    mask(ACCSOKAY);
    ads(ADDRESS_OF(DAPBOOLS));
    relint();
    ca(ADDRESS_OF(ACCRETRN));
    tc_to(ONE_ACCRET + 6u, BANKJUMP);
}

static void one_accret(void)
{
    inhint();
    one_accret_plus1();
}

static void one_accont(void);

/* DOCKTEST -- LM p. 1491: "BYPASS 1/ACCONT WHEN DOCKED." DOCKTEMP holds the
 * CSMDOCKD bit or +0 (1/ACCS's own TS DOCKTEMP); a negative one would fall
 * into DVOVSUB, which is not modelled. */
void docktest(void)
{
    switch (ccs_at(ADDRESS_OF(DOCKTEMP))) {
    case POSITIVE: one_accret(); return;
    case IS_PLUS_ZERO: one_accont(); return;
    default: not_modelled(); return;
    }
}

/* DOWNGTS -- LM p. 1504: "ZERO SWITCHES WHEN USEQRJTS BIT IS UP OR DAP IS
 * OFF". */
static void downgts(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(ALLOWGTS));
    ts(ADDRESS_OF(INGTS));
    docktest();
}

/* ---- SPSCONT -- LM pp. 1490-1491 ---- */

/* SPSCONT -- LM p. 1490, A ACCDOTQ: KQ and KRDAP, ".3ACCDOTQ" and
 * ".3ACCDOTR", then "THE SIGNED JERK TERMS" QACCDOT and RACCDOT from the
 * gimbal drive bits of channel 12 (LOOP3: RACCDOT, then QACCDOT): zero when
 * "NONE", minus ACCDOT(Q,R) for "POSITIVE ROTATION", plus it for negative.
 * Then the gimbal switches: down ("BE SURE THE GIMBAL SWITCHES ARE DOWN")
 * when USEQRJTS is set or "IS THE DAP RUNNINT?" (T5ADR not PAXIS) is no.
 * Else DOCKTEST when "GTS IN CONTROL", or TIMEGMBL by IBNKCALL: "NULL OFFSET
 * AND FIND ALLOWGTS". */
static void spscont(void)
{
    ts(ADDRESS_OF(ACCDOTQ));
    mp(DGBF);
    ts(ADDRESS_OF(KQ));
    ca(ADDRESS_OF(ACCDOTR));
    mp(DGBF);
    ts(ADDRESS_OF(KRDAP));
    read_channel(CHAN12);
    ts(MPAC_1);
    set_A(BIT2);
    for (;;) {
        ts(ADDRESS_OF(MPAC)); /* LOOP3 */
        ca(MPAC_1);
        mask_at(at_index(GIMBLBTS, ADDRESS_OF(MPAC)));
        if (A_is_zero()) {
            set_A(ZERO); /* ZACCDOT */
        } else {
            ca(MPAC_1);
            mask_at(at_index(GIMBLBTS + 1u, ADDRESS_OF(MPAC)));
            if (A_is_zero())
                ca(at_index(ADDRESS_OF(ACCDOTQ), ADDRESS_OF(MPAC))); /* FRSTZERO */
            else
                cs(at_index(ADDRESS_OF(ACCDOTQ), ADDRESS_OF(MPAC)));
        }
        ts(at_index(ADDRESS_OF(QACCDOT), ADDRESS_OF(MPAC))); /* STACCDOT */
        if (ccs_at(ADDRESS_OF(MPAC)) != POSITIVE) break;
        set_A(ZERO); /* "NOW DO QACCDOT." */
    }
    cs(ADDRESS_OF(DAPBOOLS)); /* "IS GIMBAL USABLE?" */
    mask(USEQRJTS);
    if (A_is_zero()) {
        downgts();
        return;
    }
    cs(ADDRESS_OF(T5ADR));
    ad(PAXISADR);
    if (!A_is_zero()) {
        downgts();
        return;
    }
    switch (ccs_at(ADDRESS_OF(INGTS))) {
    case POSITIVE: docktest(); return;
    case IS_PLUS_ZERO: tc_to(DOCKTEST - 2u, IBNKCALL); return; /* CADR TIMEGMBL */
    case NEGATIVE: not_modelled(); return;                     /* the CADR as an instruction */
    case IS_MINUS_ZERO: docktest(); return;
    }
}

/* ---- LRESC -- LM pp. 1489-1490 ---- */

/* LRESC -- LM p. 1489, in descent: "GET QUOTIENT WITH OVERFLOW PROTECTION"
 * three times, for the engine's thrust (ABDELV times MASS over GFACTM) times
 * L,PVT-CG, the gimbal's torque, over TORKJET1 times the R and Q inertias:
 * ACCDOTR and ACCDOTQ, "SCALED AT PI/2(7)". Then SPSCONT. */
static void lresc(void)
{
    ca(ADDRESS_OF(ABDELV));
    mp_at(ADDRESS_OF(MASS));
    dvovsub(LRESC + 3u); /* ADRES GFACTM */
    mp_at(ADDRESS_OF(L_PVT_CG));
    inhint();
    ts(ADDRESS_OF(MPAC));
    mp_at(ADDRESS_OF(1JACCR));
    dvovsub(LRESC + 013u); /* ADRES TORKJET1 */
    ts(ADDRESS_OF(ACCDOTR));
    ca(ADDRESS_OF(MPAC));
    mp_at(ADDRESS_OF(1JACCQ));
    dvovsub(LRESC + 021u); /* ADRES TORKJET1 */
    spscont();
}

/* ---- COMMEQS, the inertias -- LM pp. 1487-1489 ---- */

static void one_accont_minus1(void);

/* The coupling of the Q and R axes: "EPSILON IS A MEASURE OF COUPLING",
 * 1 - IQ/IR when "IR GREATER THAN IQ" (COMMEQS; or BIGIQ, -EPSILON from
 * 1 - IR/IQ), "LIMITED TO A MAX. OF .42265". COEFFQ and COEFFR (GOODEPS1,
 * GOODEPS2) resolve the Q and R accelerations into the U and V ones,
 * 1JACCU and 1JACCV (JACCUV). A TS COEFFR or COEFFQ that overflows skips
 * the CS after it. */
static void commeqs(void)
{
    cs(ADDRESS_OF(1JACCR));
    ad_at(ADDRESS_OF(1JACCQ));
    if (!A_is_zero_or_minus()) {
        dv_at(ADDRESS_OF(1JACCQ));
        ts(ADDRESS_OF(EPSILON));
        ad(MINUS_EPSMAX);
        if (!A_is_zero_or_minus()) {
            set_A(negate(MINUS_EPSMAX));
            ts(ADDRESS_OF(EPSILON));
        }
        ca(ADDRESS_OF(EPSILON)); /* GOODEPS1 */
        mp(ZERO_35356);
        ad(POINT7071);
        if (!ts(ADDRESS_OF(COEFFR))) set_A(negate(POSMAX));
        ad_at(ADDRESS_OF(EPSILON));
        mp_at(ADDRESS_OF(COEFFR));
        ts(ADDRESS_OF(COEFFQ));
    } else {
        dv_at(ADDRESS_OF(1JACCR)); /* BIGIQ */
        ts(ADDRESS_OF(MEPSILON));
        cs(ADDRESS_OF(MEPSILON));
        ad(MINUS_EPSMAX);
        if (!A_is_zero_or_minus()) {
            set_A(MINUS_EPSMAX);
            ts(ADDRESS_OF(MEPSILON));
        }
        ca(ADDRESS_OF(MEPSILON)); /* GOODEPS2 */
        mp(ZERO_35356);
        ad(MINUS_POINT7071);
        if (!ts(ADDRESS_OF(COEFFQ))) cs(ADDRESS_OF(MEPSILON));
        ad(NEGMAX);
        mp_at(ADDRESS_OF(COEFFQ));
        ts(ADDRESS_OF(COEFFR));
    }
    cs(ADDRESS_OF(COEFFQ)); /* JACCUV */
    mp_at(ADDRESS_OF(1JACCQ));
    ts(ADDRESS_OF(1JACCU));
    ca(ADDRESS_OF(COEFFR));
    mp_at(ADDRESS_OF(1JACCR));
    ad_at(ADDRESS_OF(1JACCU));
    mp(BIT14); /* "SCALING CHANGED FROM PI/4 TO PI/2" */
    ts(ADDRESS_OF(1JACCU));
    ts(ADDRESS_OF(1JACCV));
}

/* The inertia terms from STCTR (skip_stctr false) or STCTR1, MPAC the
 * index JX and MPAC +1 the term J: "1JACC(J)=A(JX)/(MASS+C(JX) + B(JX)",
 * for 1JACCR, 1JACCQ and 1JACC (J 2, 1, 0; JX "10,8,6 OR 4,2,0" in ascent
 * or descent), and with J -1 "1JACC(-1)=L,PVT-CG". A TS MPAC +2 that
 * overflows skips the EXTEND: the DCA runs as a CA of INERCONA's low
 * word. A TS 1JACC that overflows skips the CCS to TCF STCTR. Then COMMEQS
 * and JACCUV (J +0), LRESC (J negative). "COMPUTE L,PVT-CG IF IN DESCENT":
 * JACCUV's CCS MPAC, 6 in ascent and -0 in descent (2 less 2), goes on in
 * descent with J -1, in ascent to 1/ACCONT -1, "ZERO SWITCHES AND GO TO
 * 1/ACCONT IN ASCENT". */
static void inertias(bool skip_stctr)
{
    for (;;) {
        if (!skip_stctr) {
            ts(MPAC_1); /* STCTR */
            set_A(negate(TWO));
            ads(ADDRESS_OF(MPAC));
        }
        skip_stctr = false;
        ca(ADDRESS_OF(LEMMASS)); /* STCTR1 */
        ad_at(at_index(INERCONC, ADDRESS_OF(MPAC)));
        if (ts(MPAC_2))
            ca(at_index(INERCONA + 1u, ADDRESS_OF(MPAC)));
        else
            dca(at_index(INERCONA, ADDRESS_OF(MPAC)));
        dv_at(MPAC_2);
        ad_at(at_index(INERCONB, ADDRESS_OF(MPAC)));
        if (ts(at_index(ADDRESS_OF(1JACC), MPAC_1))) continue;
        switch (ccs_at(MPAC_1)) {
        case POSITIVE: continue;
        case IS_PLUS_ZERO: break;
        case NEGATIVE: lresc(); return;
        case IS_MINUS_ZERO: break;
        }
        commeqs();
        switch (ccs_at(ADDRESS_OF(MPAC))) {
        case POSITIVE: set_A(ZERO);                  /* fall through */
        case IS_PLUS_ZERO: ts(ADDRESS_OF(ALLOWGTS)); /* fall through */
        case NEGATIVE: one_accont_minus1(); return;
        case IS_MINUS_ZERO: break;
        }
        set_A(negate(TWO));
        ts(ADDRESS_OF(MPAC));
        set_A(negate(ONE));
        ts(MPAC_1);
        skip_stctr = true;
    }
}

/* ---- DOCKED -- LM pp. 1494-1496 ---- */

/* SPSLOOP1 -- LM p. 1494 and SPSLOOP2, A COEFCTR: 1 for the inertia's
 * coefficients (C, F, B, A, E, D), 7 for the CG's. "LET X = CSMMASS AND Y =
 * LEMMASS": "MPAC = C X Y + F", then "MPAC = MPAC + (A X +D)X + (B Y +E)Y",
 * MASSCTR 1 then 0 (INDEX MASSCTR on LEMMASS reads CSMMASS, the word after).
 * A TS MPAC that overflows skips the TCF +4 into SPSLOOP2. DIM takes COEFCTR
 * to 0 or 6, and +1 less 1 is -0: so CCS COEFCTR goes on at CA MPAC for the
 * inertia (the result is true) and leaves the loop for the CG: "IF COEFCTR
 * IS POS, EXIT FROM LOOP" (as a negative MASSCTR does). +0 and a negative
 * COEFCTR, or a -0 MASSCTR, run TORQCONS as instructions: not modelled. */
static bool docked_polynomial(void)
{
    bool spsloop2;

    ts(ADDRESS_OF(COEFCTR)); /* SPSLOOP1 */
    set_A(ONE);
    ts(ADDRESS_OF(MASSCTR));
    ca(at_index(COEFF - 1u, ADDRESS_OF(COEFCTR))); /* "COEFF -1 = C" */
    mp_at(ADDRESS_OF(LEMMASS));
    mp_at(ADDRESS_OF(CSMMASS));
    ad_at(at_index(COEFF, ADDRESS_OF(COEFCTR))); /* "COEFF = F" */
    spsloop2 = ts(ADDRESS_OF(MPAC));
    for (;;) {
        if (spsloop2) {
            ts(ADDRESS_OF(MASSCTR)); /* SPSLOOP2 */
            dim(ADDRESS_OF(COEFCTR));
        }
        ca(at_index(COEFF + 2u, ADDRESS_OF(COEFCTR))); /* "COEFF +2 = A OR B" */
        mp_at(at_index(ADDRESS_OF(LEMMASS), ADDRESS_OF(MASSCTR)));
        ad_at(at_index(COEFF + 4u, ADDRESS_OF(COEFCTR))); /* "COEFF +4 = E OR D" */
        mp_at(at_index(ADDRESS_OF(LEMMASS), ADDRESS_OF(MASSCTR)));
        ads(ADDRESS_OF(MPAC));
        switch (ccs_at(ADDRESS_OF(MASSCTR))) {
        case POSITIVE: spsloop2 = true; continue;
        case IS_PLUS_ZERO: break;
        case NEGATIVE: return false;
        case IS_MINUS_ZERO: not_modelled(); return false;
        }
        switch (ccs_at(ADDRESS_OF(COEFCTR))) {
        case POSITIVE: return false;
        case IS_MINUS_ZERO: return true;
        default: not_modelled(); return false;
        }
    }
}

/* DOCKED -- LM p. 1494: "USE SEPARATE COMPUTATION." The inertia, "(MPAC +1)
 * X 2(38) KG-CM(2)", and the CG's offset times ABDELV ("CG X DELDOT = MPAC X
 * 4 PI RAD-CM/SEC") from the polynomials; "1JACC=1JACCCON/MASS"; the inverse
 * jet accelerations "TO POSMAX, WHICH CORRESPONDS TO ACCEL. OF 1.4 D/SS.";
 * 1JACCQ and 1JACCR, TORQCONS ("CORRESPONDS TO 500 LB-FT") over the inertia;
 * COEFFQ and COEFFR "TO MAKE U- AND V-AXES ORTHOGONAL FOR DOCKED CASE";
 * ACCDOTR with DVOVSUB's protection. Then SPSCONT: "CONTINUE K, KSQ
 * CALCULATIONS". */
static void docked(void)
{
    set_A(ONE);
    while (docked_polynomial()) {
        ca(ADDRESS_OF(MPAC));
        ts(MPAC_1);
        set_A(SEVEN);
    }
    set_A(ONE_JACCCON);
    zl();
    dvovsub(DOCKED + 050u); /* ADRES MASS */
    ts(ADDRESS_OF(1JACC));
    set_A(POSMAX);
    ts(ADDRESS_OF(1_ANETP));
    ts(ADDRESS_OF(1_ANET2) + 1u);
    ts(ADDRESS_OF(1_ANET2) + 2u);
    ts(ADDRESS_OF(1_ANET2) + 17u);
    ts(ADDRESS_OF(1_ANET2) + 18u);
    set_L(TORQCONS[1]);
    set_A(TORQCONS[0]);
    dv_at(MPAC_1);
    inhint();
    ts(ADDRESS_OF(1JACCQ));
    ts(ADDRESS_OF(1JACCR));
    set_A(MINUS_POINT7071);
    ts(ADDRESS_OF(COEFFQ));
    set_A(POINT7071);
    ts(ADDRESS_OF(COEFFR));
    ca(ADDRESS_OF(MASS));
    mp_at(ADDRESS_OF(MPAC));
    mp_at(ADDRESS_OF(ABDELV));
    dvovsub(DOCKED + 0101u); /* ADRES MPAC +1 */
    ts(ADDRESS_OF(ACCDOTR));
    spscont();
}

/* ---- 1/ACCS +3: the mass -- LM pp. 1486-1487 ---- */

/* F(MASS) -- LM p. 1487: "DOCKED:  USE SEPARATE COMPUTATION.", else the
 * inertia terms
 * from J 2 ("J=2,1,0 FOR 1JACCR,1JACCQ,1JACC"). DOCKTEMP is the CSMDOCKD
 * bit or +0. */
static void f_mass(void)
{
    relint();
    if (ccs_at(ADDRESS_OF(DOCKTEMP)) == POSITIVE) {
        docked();
        return;
    }
    set_A(TWO);
    inertias(false);
}

/* 1/ACCS +3 -- LM p. 1486, where MAKECADR returns the caller's CADR:
 * ACCRETRN keeps it.
 * "IS THE CSM DOCKED": DOCKTEMP, and "DOCKED:  LEMMAS = MASS - CSMMASS",
 * "LEM ALONE:  LEMMASS = MASS". "DETERMINE WHETHER STAGED.": in ascent
 * "ALWAYS 2 JETS FOR P-AXIS RATE COMMAND", the index at 12; in descent
 * "FOUR JETS FOR P-AXIS RATE COMMAND ERRORS EXCEEDING 1.4 DEG/SEC", the
 * index at 6. A LEMMASS above the stage's high limit or below its low one
 * ("CATCH STAGING.") goes to MASSFIX: "STORE THE VIOLATED LIMIT AS
 * LEMMASS.", "ALSO CORRECT TOTAL MASS, ZEROING THE LOW-ORDER WORD." Then
 * F(MASS). A TS ACCRETRN that overflows skips CA DAPBOOLS, a TS LEMMASS
 * the INHINT. */
void one_accs_plus3(void)
{
    bool massfix;

    if (!ts(ADDRESS_OF(ACCRETRN))) ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    ts(ADDRESS_OF(DOCKTEMP)); /* "STORE RECORD OF STATE IN TEMP" */
    if (ccs_at(A_ADDRESS) == POSITIVE) cs(ADDRESS_OF(CSMMASS));
    ad_at(ADDRESS_OF(MASS));
    if (!ts(ADDRESS_OF(LEMMASS))) inhint();
    ca(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    if (!A_is_zero()) {
        set_A(negate(POSMAX)); /* "ASCENT (OR ON LUNAR SURFACE)" */
        ts(ADDRESS_OF(M2JETLIM));
        set_A(OCT14);
        ts(ADDRESS_OF(MPAC));
        cs(ADDRESS_OF(LEMMASS)); /* "CHECK IF MASS TOO HIGH." */
        ad_at(ADDRESS_OF(HIASCENT));
        massfix = A_is_zero_or_minus();
        if (!massfix) {
            cs(ADDRESS_OF(LEMMASS)); /* "CHECK IF MASS TOO LOW." */
            ad(LOASCENT);
            massfix = !A_is_zero_or_minus();
        }
    } else {
        set_A(negate(BIT10)); /* DPSFLITE */
        ts(ADDRESS_OF(M2JETLIM));
        set_A(SIX);
        ts(ADDRESS_OF(MPAC));
        cs(ADDRESS_OF(LEMMASS));
        ad(HIDESCNT);
        massfix = A_is_zero_or_minus();
        if (!massfix) {
            cs(ADDRESS_OF(LEMMASS));
            ad(LODESCNT);
            ad_at(ADDRESS_OF(HIASCENT));
            massfix = !A_is_zero_or_minus();
        }
    }
    if (massfix) {
        ads(ADDRESS_OF(LEMMASS)); /* MASSFIX */
        zl();
        if (ccs_at(ADDRESS_OF(DOCKTEMP)) == POSITIVE) ca(ADDRESS_OF(CSMMASS));
        ad_at(ADDRESS_OF(LEMMASS));
        dxch(ADDRESS_OF(MASS));
    }
    f_mass();
}

/* ---- 1/ACCONT, GETAOSUV -- LM p. 1497 ---- */

/* 1/ACCONT -- LM p. 1497: "INITIALIZE DBVAL1,2,3", DB, "0.50 DB" and "-.75
 * DB". Then GETAOSUV: "COMPUTE ASOU AND AOSV BY ROTATING AOSQ AND AOSR." by
 * ROT-TOUV (IBNKCALL), which returns at GETAOSUV +6. */
static void one_accont(void)
{
    ca(ADDRESS_OF(DB));
    mp(BIT13);
    ts(L_ADDRESS); /* "0.25 DB" */
    ad_at(A_ADDRESS);
    ts(ADDRESS_OF(DBVAL3));
    cs(ADDRESS_OF(DBVAL1));
    ad_at(L_ADDRESS);
    ts(ADDRESS_OF(DBVAL2));
    inhint(); /* GETAOSUV */
    ca(ADDRESS_OF(AOSR));
    ts(L_ADDRESS);
    ca(ADDRESS_OF(AOSQ));
    tc_to(GETAOSUV + 4u, IBNKCALL); /* CADR ROT-TOUV */
}

/* 1/ACCONT -1 -- LM p. 1497: "ZERO INGTS IN ASCENT" (A +0, or what JACCUV's
 * CCS left). */
static void one_accont_minus1(void)
{
    ts(ADDRESS_OF(INGTS));
    one_accont();
}

/* ---- 1/ANET-, 1/NETMIN, DO1/NET+ -- LM pp. 1504-1505 ---- */

/* DOACCFUN -- LM p. 1504, A -1 at 2(6) (or -1 at 2(7)): "ACCFUN AT 2(7)/PI",
 * -1/ANET over ANET, then TC ARET: "RETURN". */
static void doaccfun(void)
{
    mp_at(ADDRESS_OF(1_ANET)); /* "-1/ANET AT 2(13)/PI" */
    dv_at(ADDRESS_OF(ANET));
    tc_through(ADDRESS_OF(ARET));
}

/* DO1/NET+ -- LM p. 1504, A ANET over the coast acceleration: "1 +
 * ANET/ACOAST AT 2(6)" into ANET, the return into ARET ("SAVE RETURN"),
 * 1/ANET "AT 2(7)/PI" by INVERT, and ACCFUN. */
static void do1_net_plus(void)
{
    ad(BIT9);
    xch(ADDRESS_OF(ANET)); /* "SAVE AND PICK UP ANET" */
    qxch(ADDRESS_OF(ARET));
    tc_invert(DO1_NET_PLUS + 4u);
    ts(ADDRESS_OF(1_ANET));
    set_A(negate(BIT9));
    doaccfun();
}

/* 1/NETMIN +1 -- LM p. 1504, A ANET: "ANETNEG(POS)/ACOASTPOS(NEG) AT
 * 2(6)". */
static void one_netmin_plus1(void)
{
    mp_at(at_index(ADDRESS_OF(1_ACOSTT) + 1u, ADDRESS_OF(MSIGNAOS)));
    do1_net_plus();
}

/* TC 1/NETMIN at at. */
static void tc_one_netmin(word at)
{
    tc_at(at);
    ca(ADDRESS_OF(ANET));
    one_netmin_plus1();
}

/* TC DO1/NET+ at at, A ANET. */
static void tc_do1_net_plus(word at)
{
    tc_at(at);
    do1_net_plus();
}

/* TC 1/ANET- at at, A ANET: "ZERO ACCSW" (L gets the old one); "ANET LESS
 * THAN AMIN, SO FAKE IT" (NETNEG: "SET EQUAL TO AMIN", "CONTINUE AS IF
 * NOTHING HAPPENED."). A TS ANET that overflows skips the AD. */
static void tc_one_anet_minus(word at)
{
    tc_at(at);
    zl();
    lxch(ADDRESS_OF(ACCSW));
    if (!ts(ADDRESS_OF(ANET))) ad(MINUS_POINT03R_S2); /* "TEST FOR MIN VALUE" */
    if (A_is_zero_or_minus()) {
        set_A(negate(MINUS_POINT03R_S2)); /* NETNEG */
        ts(ADDRESS_OF(ANET));
        one_netmin_plus1();
        return;
    }
    ca(ADDRESS_OF(ANET)); /* 1/NETMIN */
    one_netmin_plus1();
}

/* ---- BOTHAXES, the U and V axes -- LM pp. 1498-1503 ---- */

/* SKIPDB2 -- LM p. 1500: "ANETPOS(NEG) MAX = 2 JACC + ABS(AOS)", "CANNOT
 * OVERFLOW HERE" (if it does, the TS skips the TC to ACCTHERE), then
 * CL1/NET+: "COMPUTE 1/ANET, ACCFUN". */
static void skipdb2(void)
{
    ca(ADDRESS_OF(ABSAOS));
    ad_at(ADDRESS_OF(1JACCU));
    ad_at(ADDRESS_OF(1JACCU));
    if (ts(ADDRESS_OF(ANET))) return;
    tc_do1_net_plus(CL1_NET_PLUS);
}

/* SKIPDB1 -- LM p. 1499: "ABS(AOS) GREATER THAN AMIN, SO IT IS", at its TC
 * INVERT
 * "ALL RIGHT TO DIVIDE":
 * "1/ACOASTPOS(NET) = 1/ABS(AOS)" from "(9/8) ABSAOS.", the other "1/AIN".
 * "2 JACC + ABS(AOS)" with BIT9 is the "MAXIMUM VALUE IN COMPUTATIONS":
 * TS A tests it, and an overflow "RESCALE TO PI TO PREVENT OVERFLOW":
 * ANET and 1/ANET at PI, ACCFUN by DOACCFUN, which returns by ACCHERE. */
static void skipdb1(void)
{
    ca(ADDRESS_OF(ABSAOS));
    mp(BIT12);
    ad_at(ADDRESS_OF(ABSAOS));
    tc_invert(SKIPDB1 + 4u);
    ts(at_index(ADDRESS_OF(1_ACOSTT) + 1u, ADDRESS_OF(MSIGNAOS)));
    set_A(ONE_POINT03);
    ts(at_index(ADDRESS_OF(1_ACOSTT), ADDRESS_OF(SIGNAOS)));
    ca(ADDRESS_OF(ABSAOS));
    ad_at(ADDRESS_OF(1JACCU));
    ad_at(ADDRESS_OF(1JACCU));
    ad(BIT9);
    if (!ts(A_ADDRESS)) { /* "TEST FOR OVERFLOW" */
        skipdb2();
        return;
    }
    ca(ADDRESS_OF(ABSAOS));
    mp(BIT14);
    ad_at(ADDRESS_OF(1JACCU));           /* "1 JACC AT PI/2 = 2JACC AT PI" */
    if (!ts(ADDRESS_OF(ANET))) ad(BIT8); /* "1 + ANETPOS/ACOASTNEG AT 2(7)" */
    xch(ADDRESS_OF(ANET));
    tc_invert(ACCHERE - 010u);
    mp(BIT14); /* "SCALE 1/ANET AT 2(7)/PI" */
    ts(ADDRESS_OF(1_ANET));
    set_A(ACCHERE); /* "SET UP RETURN FROM COMPUTATION ROUTINE" */
    ts(ADDRESS_OF(ARET));
    set_A(negate(BIT8)); /* "-1 AT 2(7)" */
    doaccfun();          /* ACCHERE: TCF ACCTHERE */
}

/* BIGAOS -- LM p. 1498: "AGS(AOS) GREATER THAN AMIN". "I DRIFT OR GTS, DO
 * NOT COMPUTE
 * DB" (FLATEMP positive); else "DB2(1) = 2 DB", "DB4(3) = 1 DB",
 * "DB1(2)=(1-8 ABSAOS) DB.  IF ABSAOS IS GREATER THAN .1875 THEN DB1(2) =
 * -.5 DB", "DB3(4) = -.75 DB". CCS FLATEMP's other exits run on into the
 * ADS. A TS DBB4 that overflows skips CA -.1875. */
static void bigaos(void)
{
    switch (ccs_at(ADDRESS_OF(FLATEMP))) {
    case POSITIVE: skipdb1(); return;
    case IS_PLUS_ZERO: ca(ADDRESS_OF(DBVAL1)); /* fall through */
    case NEGATIVE: ads(at_index(ADDRESS_OF(DBB2), ADDRESS_OF(MSIGNAOS))); break;
    case IS_MINUS_ZERO: ads(ADDRESS_OF(DBB2)); break;
    }
    if (!ts(at_index(ADDRESS_OF(DBB4), ADDRESS_OF(SIGNAOS)))) set_A(MINUS_POINT1875);
    ad_at(ADDRESS_OF(ABSAOS));
    if (A_is_zero_or_minus()) {
        cs(ADDRESS_OF(ABSAOS));
        double_A();
        double_A();
        ad(BIT14);
        double_A(); /* "1-8 ABSAOS." */
        mp_at(ADDRESS_OF(DB));
    } else {
        cs(ADDRESS_OF(DBVAL3)); /* "-.5 DB" */
    }
    ts(at_index(ADDRESS_OF(DBB1), ADDRESS_OF(SIGNAOS))); /* DBONE */
    ca(ADDRESS_OF(DBVAL2));
    ts(at_index(ADDRESS_OF(DBB3), ADDRESS_OF(MSIGNAOS)));
    skipdb1();
}

/* NOTMUCH -- LM p. 1500: "ABS(AOS) LESS THAN AMIN", "ACOASTPOS,NEG = AMIN".
 * "DO NOT COMPUTE DB IF DRIFT OR GTS" (FLATEMP positive); else "AMIN/2 LT
 * ABS(AOS) LT AMIN" (SOMEAOS: "DB3(4) = DB/2", "DB4(3) = DB"; a TS DBB4 that
 * overflows skips to NOAOS), or "ABS(AOS) LESS THAN AMIN/2" (NOAOS: "DB3,4 =
 * DB"). CCS FLATEMP's other exits run on into the AD L and the BZMF. */
static void notmuch(void)
{
    bool noaos;

    ts(L_ADDRESS); /* "L HAS ABS(AOS) - AMIN" */
    set_A(ONE_POINT03);
    ts(ADDRESS_OF(1_ACOSTT));
    ts(ADDRESS_OF(1_ACOSTT) + 1u);
    switch (ccs_at(ADDRESS_OF(FLATEMP))) {
    case POSITIVE: skipdb2(); return;
    case IS_PLUS_ZERO: set_A(POINT0125RS); /* "AMIN/2" */ /* fall through */
    case NEGATIVE: ad_at(L_ADDRESS);                      /* fall through */
    case IS_MINUS_ZERO: break;
    }
    noaos = A_is_zero_or_minus();
    if (!noaos) {
        ca(ADDRESS_OF(DBVAL3)); /* SOMEAOS */
        ts(at_index(ADDRESS_OF(DBB3), ADDRESS_OF(MSIGNAOS)));
        ad_at(A_ADDRESS);
        noaos = ts(at_index(ADDRESS_OF(DBB4), ADDRESS_OF(SIGNAOS)));
    }
    if (noaos) {
        ca(ADDRESS_OF(DBVAL1)); /* NOAOS */
        ts(ADDRESS_OF(DBB3));
        ts(ADDRESS_OF(DBB4));
    }
    skipdb2();
}

/* FIXMIN -- LM p. 1505, A ANET less AMIN, not positive: "IF ANET LESS THAN
 * AMIN, STORE MAX JET VALUES FOR MIN JETS AND SET ACCSW". "IF AOS NEG, ACCSW
 * = +1", "IF AOS POS, ACCSW = -1", and the failed-jet test of that side:
 * "JET FAILURE -- CANNOT USE 2-JET VALUES", "CALCULATE FUNCTIONS USING
 * AMIN"; else "L HAS ACCFUN". On to STMIN- either way. */
static void fixmin(void)
{
    if (ccs_at(ADDRESS_OF(SIGNAOS)) == POSITIVE) set_A(TWO);
    ad(NEGONE);
    ts(ADDRESS_OF(ACCSW));
    ad_at(ADDRESS_OF(UV));
    ca(indexed(MINUS_UMASK + 1u, a_word()));
    mask_at(ADDRESS_OF(CH5MASK));
    if (A_is_zero()) {
        ca(L_ADDRESS);
        return;
    }
    set_A(negate(MINUS_POINT03R_S2)); /* "ANET = AMIN" */
    ts(ADDRESS_OF(ANET));
    tc_one_netmin(STMIN_MINUS - 1u);
}

/* BOTHAXES -- LM p. 1498, A +0: "CODING COMMON TO U,V AXES", UV 0 or 1.
 * ABSAOS and
 * SIGNAOS ("SET SIGNAOS TO ONE TO SHOW AOS NEG"), the deadbands DB1-DB4
 * (BIGAOS or NOTMUCH), then ACCTHERE: ACCFUN and 1/ANET for 2 jets (max)
 * and 1 jet (min) toward the offset (into Z5TEM and 1/ATEM2), then away
 * from it ("NOW DO NEG(POS) CASES", into Z1TEM and 1/ATEM1; FIXMIN when
 * "ANET LESS THAN AMIN"). "TEST FOR +U (+V) JET FAILURES" and -U (-V):
 * "REPLACE FUNCTION VALUES DEPENDING ON THE", the "FAILED JET PAIR",
 * with the one-jet (or AMIN) ones. DBFUN: "COMPUTE AXISDIST". Then the
 * axis's values go by GENTRAN ("TRANSFER 10 WORDS"):
 * the U axis's into 1/ANET1, the V axis's (STORV) into 1/ANET1 +16D. TS
 * ANET, TS AXDSTEM or TS AXDSTEM +1 that overflows skips the next
 * instruction. */
static void bothaxes(void)
{
    ts(ADDRESS_OF(SIGNAOS));
    switch (ccs_at(at_index(ADDRESS_OF(AOSU), ADDRESS_OF(UV)))) {
    case POSITIVE: ad(ONE); break; /* "RESTORE TO PROPER VALUE" */
    case IS_PLUS_ZERO: break;
    case NEGATIVE: ad(ONE); /* fall through */
    case IS_MINUS_ZERO: incr(ADDRESS_OF(SIGNAOS)); break;
    }
    ts(ADDRESS_OF(ABSAOS));
    cs(ADDRESS_OF(SIGNAOS));
    ts(ADDRESS_OF(MSIGNAOS)); /* "USED AS AN INDEX" */
    ca(ADDRESS_OF(DBVAL1));   /* "SET DB1, DB2 TO DBVAL1 (= DB)" */
    ts(ADDRESS_OF(DBB1));
    ts(ADDRESS_OF(DBB2));
    ca(ADDRESS_OF(ABSAOS));
    ad(MINUS_POINT03R_S2);
    if (A_is_zero_or_minus())
        notmuch();
    else
        bigaos();

    ts(at_index(ADDRESS_OF(Z5TEM) + 2u, ADDRESS_OF(MSIGNAOS))); /* ACCTHERE */
    ca(ADDRESS_OF(1_ANET));
    ts(at_index(ADDRESS_OF(1_ATEM2) + 2u, ADDRESS_OF(MSIGNAOS)));
    ca(ADDRESS_OF(ABSAOS)); /* "SEE IF OVERFLOW IN MIN CASE" */
    ad_at(ADDRESS_OF(1JACCU));
    ad(BIT9);
    if (ts(A_ADDRESS)) set_A(POSMAX); /* "IF OVERFLOW, TRUNCATE TO PI/2" */
    ad(MINUS_POINT03R_S2);
    ts(ADDRESS_OF(ANET));
    tc_do1_net_plus(ACCTHERE + 015u);
    ts(at_index(ADDRESS_OF(Z5TEM), ADDRESS_OF(MSIGNAOS)));
    ca(ADDRESS_OF(1_ANET));
    ts(at_index(ADDRESS_OF(1_ATEM2), ADDRESS_OF(MSIGNAOS)));
    cs(ADDRESS_OF(ABSAOS));
    ad_at(ADDRESS_OF(1JACCU));
    ad_at(ADDRESS_OF(1JACCU));
    tc_one_anet_minus(ACCTHERE + 026u);
    ts(at_index(ADDRESS_OF(Z1TEM) + 2u, ADDRESS_OF(SIGNAOS)));
    ts(L_ADDRESS); /* "SAVE IN L FOR POSSIBLE FUTURE USE" */
    ca(ADDRESS_OF(1_ANET));
    ts(at_index(ADDRESS_OF(1_ATEM1) + 2u, ADDRESS_OF(SIGNAOS)));
    cs(ADDRESS_OF(ABSAOS));
    ad_at(ADDRESS_OF(1JACCU));
    if (!ts(ADDRESS_OF(ANET))) ad(MINUS_POINT03R_S2); /* "TEST FOR AMIN" */
    if (A_is_zero_or_minus())
        fixmin();
    else
        tc_one_netmin(STMIN_MINUS - 1u);

    ts(at_index(ADDRESS_OF(Z1TEM), ADDRESS_OF(SIGNAOS))); /* STMIN- */
    ca(ADDRESS_OF(1_ANET));
    ts(at_index(ADDRESS_OF(1_ATEM1), ADDRESS_OF(SIGNAOS)));
    ca(at_index(PLUS_UMASK, ADDRESS_OF(UV)));
    mask_at(ADDRESS_OF(CH5MASK));
    if (!A_is_zero()) {
        ca(ADDRESS_OF(1_ATEM2));
        ts(ADDRESS_OF(1_ATEM2) + 2u);
        ca(ADDRESS_OF(Z5TEM));
        ts(ADDRESS_OF(Z5TEM) + 2u);
    }
    ca(at_index(MINUS_UMASK, ADDRESS_OF(UV))); /* FAIL- */
    mask_at(ADDRESS_OF(CH5MASK));
    if (!A_is_zero()) {
        ca(ADDRESS_OF(1_ATEM1));
        ts(ADDRESS_OF(1_ATEM1) + 2u);
        ca(ADDRESS_OF(Z1TEM));
        ts(ADDRESS_OF(Z1TEM) + 2u);
    }
    cs(ADDRESS_OF(DBB3)); /* DBFUN */
    ad_at(ADDRESS_OF(DBB1));
    ad_at(ADDRESS_OF(FLATEMP));
    if (!ts(ADDRESS_OF(AXDSTEM))) cs(ADDRESS_OF(DBB4));
    ad_at(ADDRESS_OF(DBB2));
    ad_at(ADDRESS_OF(FLATEMP));
    if (!ts(ADDRESS_OF(AXDSTEM) + 1u)) inhint();
    if (ccs_at(ADDRESS_OF(UV)) == POSITIVE) {
        ca(ADDRESS_OF(ACCSW)); /* STORV */
        ts(ADDRESS_OF(ACCSWV));
        set_A(NINE);
        tc_to(STORV + 3u, GENTRAN + 1u); /* ADRES 1/ATEM1, ADRES 1/ANET1 +16D */
    } else {
        ca(ADDRESS_OF(ACCSW));
        ts(ADDRESS_OF(ACCSWU));
        set_A(NINE);
        tc_to(STORV - 016u, GENTRAN + 1u); /* ADRES 1/ATEM1, ADRES 1/ANET1 */
    }
}

/* ---- GETAOSUV +6, DOPAXIS -- LM pp. 1497-1498 ---- */

/* GETAOSUV +6 -- LM p. 1497, A,L AOSU and AOSV from ROT-TOUV. DRIFTER: "IF
 * DRIFTING LIGHT, SET DRIFTER TO 1". FLATEMP: "DRIFTING FLIGHT, STORE .8 IN
 * FLAT", "IN POWERED FLIGHT, STORE ZERO IN FLAT"; "IF POWERED AND NO GTS,
 * START P AXIS, OTHERWISE SET ZONE3LIM" ("17.5 MS" when drifting). CCS's
 * negative exits store no FLATEMP. DOPAXIS: "P AXIS DATA MUST BE
 * CONSISTENT": 1/ANETP, PACCFUN ("-1/(ANET + ANET**2/ACOAST) AT 2(7)/PI")
 * and 1/ACOSTP, "NO AOS FOR P AXIS, ACOAST = AMIN". "ZERO AOSU,V IF IN
 * DRIFT, JUST TO BE SURE", then the U axis (UAXIS: UV 0). A TS FUNTEM that
 * overflows skips CA 1JACC. */
void getaosuv_plus6(void)
{
    bool dopaxis;

    dxch(ADDRESS_OF(AOSU));
    relint();
    ca(ADDRESS_OF(DAPBOOLS));
    mask(DRIFTBIT); /* "ZERO DURING ULLAGE AND POWERED FLIGHT." */
    if (ccs_at(A_ADDRESS) == POSITIVE) set_A(ONE);
    ts(ADDRESS_OF(DRIFTER));
    ad_at(ADDRESS_OF(ALLOWGTS)); /* "NON-ZERO IF DRIFT OR GTS NEAR" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: set_A(FLATVAL);              /* fall through */
    case IS_PLUS_ZERO: ts(ADDRESS_OF(FLATEMP)); /* fall through */
    case NEGATIVE: dopaxis = A_is_zero(); break;
    default: dopaxis = true; break; /* BZF without its EXTEND: TCF DOPAXIS */
    }
    if (!dopaxis) {
        if (ccs_at(ADDRESS_OF(DRIFTER)) == POSITIVE) set_A(ZONE3MAX);
        ts(ADDRESS_OF(Z3TEM));
    }
    ca(ADDRESS_OF(1JACC)); /* DOPAXIS */
    ad(BIT9);              /* "1 + ANET/ACOAST AT 2(6)" */
    if (!ts(ADDRESS_OF(FUNTEM))) ca(ADDRESS_OF(1JACC));
    tc_invert(DOPAXIS + 4u);
    inhint();
    ts(ADDRESS_OF(1_ANETP)); /* "SCALED AT 2(7)/PI." */
    ts(ADDRESS_OF(1_ANETP) + 1u);
    set_A(negate(BIT9));
    mp_at(ADDRESS_OF(1_ANETP));
    dv_at(ADDRESS_OF(FUNTEM));
    ts(ADDRESS_OF(PACCFUN));
    ts(ADDRESS_OF(PACCFUN) + 1u);
    set_A(ONE_POINT03);
    ts(ADDRESS_OF(1_ACOSTP));
    ts(ADDRESS_OF(1_ACOSTP) + 1u);
    relint();
    zl();
    if (ccs_at(ADDRESS_OF(DRIFTER)) == POSITIVE) dxch(ADDRESS_OF(AOSU));
    set_A(ZERO); /* UAXIS */
    ts(ADDRESS_OF(UV));
    bothaxes();
}

/* The U axis's values sent: its deadbands into UDB1-UDB4 and UAXDIST,
 * "SAVE U DBS FOR LATER STORING", then "NOW DO V AXIS" (UV 1). */
void u_axis_stored(void)
{
    relint();
    dxch(ADDRESS_OF(DBB1));
    dxch(ADDRESS_OF(UDB1));
    dxch(ADDRESS_OF(DBB4));
    dxch(ADDRESS_OF(UDB4));
    dxch(ADDRESS_OF(AXDSTEM));
    dxch(ADDRESS_OF(UAXDIST));
    set_A(ONE);
    ts(ADDRESS_OF(UV));
    set_A(ZERO);
    bothaxes(); /* "AND DO IT AGAIN" */
}

/* ---- The deadbands, DRFDB -- LM pp. 1502-1503 ---- */

/* DRFDB -- LM p. 1503: "DRIFT DEADBANDS" for the U and V axes: FIREDB DB,
 * COASTDB DB + FLAT, AXISDIST 0. A TS COASTDB that overflows skips TS
 * COASTDB +1. */
static void drfdb(void)
{
    ca(ADDRESS_OF(DBVAL1));
    ts(ADDRESS_OF(FIREDB));
    ts(ADDRESS_OF(FIREDB) + 1u);
    ts(ADDRESS_OF(FIREDB) + 16u);
    ts(ADDRESS_OF(FIREDB) + 17u);
    ad_at(ADDRESS_OF(FLAT));
    if (!ts(ADDRESS_OF(COASTDB))) ts(ADDRESS_OF(COASTDB) + 1u);
    ts(ADDRESS_OF(COASTDB) + 16u);
    ts(ADDRESS_OF(COASTDB) + 17u);
    set_A(ZERO);
    ts(ADDRESS_OF(AXISDIST));
    ts(ADDRESS_OF(AXISDIST) + 1u);
    ts(ADDRESS_OF(AXISDIST) + 16u);
    ts(ADDRESS_OF(AXISDIST) + 17u);
}

/* The V axis's values sent: "NOW STORE DEADBANDS FOR ALL AXES". FLAT and
 * ZONE3LIM; "COMPUTE P AXIS DEADBANDS"; then "DRIFT OR GTS -- COMPUTE DBS"
 * (DRFDB), or the U and V deadbands kept, by DXCH: "CANNOT USE GENTRAN
 * BECAUSE OF RELINT". CCS FLAT's other exits run on into the DXCHs.
 * "ALL DONE": 1/ACCRET. A TS PDB3 that overflows skips TS PDB4. */
void v_axis_stored(void)
{
    dxch(ADDRESS_OF(FLATEMP)); /* "FLAT AND ZONE3LIM" */
    dxch(ADDRESS_OF(FLAT));
    ca(ADDRESS_OF(DBVAL1));
    ts(ADDRESS_OF(PDB1));
    ts(ADDRESS_OF(PDB2));
    ad_at(ADDRESS_OF(FLAT));
    if (!ts(ADDRESS_OF(PDB3))) ts(ADDRESS_OF(PDB4));
    set_A(ZERO);
    ts(ADDRESS_OF(PAXDIST));
    ts(ADDRESS_OF(PAXDIST) + 1u);
    switch (ccs_at(ADDRESS_OF(FLAT))) {
    case POSITIVE:
        drfdb();
        one_accret();
        return;
    case IS_PLUS_ZERO: dxch(ADDRESS_OF(UDB1)); /* "STORE U DEADBANDS" */ /* fall through */
    case NEGATIVE: dxch(ADDRESS_OF(FIREDB));                             /* fall through */
    case IS_MINUS_ZERO: break;
    }
    dxch(ADDRESS_OF(UDB4));
    dxch(ADDRESS_OF(COASTDB));
    dxch(ADDRESS_OF(UAXDIST));
    dxch(ADDRESS_OF(AXISDIST));
    dxch(ADDRESS_OF(DBB1)); /* "STORE V AXIS DEADBANDS" */
    dxch(ADDRESS_OF(FIREDB) + 16u);
    dxch(ADDRESS_OF(DBB4));
    dxch(ADDRESS_OF(COASTDB) + 16u);
    dxch(ADDRESS_OF(AXDSTEM));
    dxch(ADDRESS_OF(AXISDIST) + 16u);
    one_accret_plus1();
}

#endif
