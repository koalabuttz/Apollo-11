/* SPDX-License-Identifier: MIT */
/* Division: DDV and BDDV, the general double-precision divide, and V/SC:
 * INTERPRETER.agc LM pp. 1057-1069, CM pp. 1162-1174 (in bank 0). The
 * faithful C is c64-port/src/lm_interp_core.c (ddv_bddv, dvovf, ddv_chain,
 * maxtest, genddv, finaldv3, maxdv, vsc2, vscdv, vecagree).
 *
 * The AGC's DV divides a double-precision dividend by a single-precision
 * divisor, and needs the dividend smaller than the divisor. To divide by a
 * double-precision divisor, the routine first makes both positive and
 * normalizes the divisor (shifts it left until it is at least .5,
 * counting the shifts in DVNORMCT), shifts the dividend left as many
 * places (the MAXTEST prologues), then divides twice, for the major and
 * minor parts of the quotient. A quotient too large to hold sets MPAC to
 * the largest number of its sign, and OVFIND (DVOVF). */
#include "interpreter.h"
#include "../agc/names.h"

static void bufzero(void);
static void sgndvovf(void);
static void ovf_plus(void);
static void buf_minus(void);
static void buf_plus(void);
static void mpac_minus(void);
static void mpac_plus(void);
static void dvnorm(void);
static void bufpos(void);
static void bufpos_plus6(void);
static void bufneg(void);
static void maxdv(void);
static void genddv(void);
static void genddv_plus1(void);
static void plus_down(void);
static void minus_up(void);
static void endmaxdv(void);
static void finaldv(void);
static void slash_buf_minus(void);
static void slash_buf_plus(void);
static bool v_scdv(void);

/* ======================================================================
 * DDV and BDDV
 * ====================================================================== */

/* DDV/BDDV -- LM p. 1057, CM p. 1162.
 * "SCALAR DIVISION INSTRUCTIONS, DDV AND BDDV, ARE EXECUTED HERE. AT THIS
 * POINT, THE DIVIDEND IS IN MPAC AND THE DIVISOR IS IN BUF." DVSIGN starts
 * at -1, "+-1 FOR POSITIVE QUOTIENT -- -0 FOR NEG.": each operand made
 * positive adds one to it. Then "FORCE BUF POSITIVE WITH THE MAJOR PART
 * NON-ZERO." */
void ddv_bddv(void)
{
    set_A(negate(ONE)); /* "INITIALIZATION" */
    ts(ADDRESS_OF(DVSIGN));
    ts(ADDRESS_OF(DVNORMCT)); /* "DIVIDENT NORMALIZATION COUNT." */
    ts(ADDRESS_OF(MAXDVSW));  /* "NEAR-ONE DIVIDE FLAG." */

    switch (ccs_at(BUF_AT(0u))) {
    case POSITIVE: bufpos(); return;
    case NEGATIVE: bufneg(); return;
    default: bufzero(); return;
    }
}

/* BUFZERO: the divisor's major part is zero. "FORCE SIGN AGREEMENT BEFORE
 * OVERFLOW" test: a dividend with a major part now is too big. Otherwise
 * "SHIFT DIVIDEND AND DIVISOR LEFT 14" and "TRY AGAIN ON FORMER MINOR
 * PART." */
static void bufzero(void)
{
    ts(MPAC_AT(2u)); /* "ZERO THIS." */
    tc_at(BUFZERO + 1u);
    tpagree();

    switch (ccs_at(MPAC_AT(0u))) {            /* "TEST TO SEE IF MPAC NON-ZERO. (TOO BIG)" */
    case POSITIVE: ovf_plus(); return;        /* "MAJOR PART OF DIVIDEND IS POSITIVE NON-0" */
    case NEGATIVE: ovf_plus_minus1(); return; /* "... IS NEG. NON-ZERO" */
    default: break;
    }

    xch(BUF_AT(1u));
    xch(BUF_AT(0u));
    xch(MPAC_AT(1u));
    xch(MPAC_AT(0u));
    switch (ccs_at(BUF_AT(0u))) {
    case POSITIVE: buf_plus(); return;
    case NEGATIVE: buf_minus(); return;
    default: break; /* "OVERFLOW ON ZERO DIVISOR." */
    }

    cs(MPAC_AT(0u)); /* "SIGN OF MPAC DETERMINES SIGN OF RESULT." */
    sgndvovf();
}

/* SGNDVOVF: the quotient's sign from A's. */
static void sgndvovf(void)
{
    if (!A_is_zero_or_minus()) incr(ADDRESS_OF(DVSIGN)); /* "NEGMAX IN MPAC PERHAPS." */
    dvovf();
}

/* DVOVF -- LM p. 1057, CM p. 1162.
 * "ON DIVISION OVERFLOW OF ANY SORT, SET DP MPAC TO +-POSMAX." (FINALDV +3
 * sets MPAC +1 and attaches the sign.) "SET OVERFLOW INDICATOR AND EXIT."
 * The exit is a TC DANZIG. */
void dvovf(void)
{
    set_A(POSMAX);
    ts(MPAC_AT(0u));
    tc_at(DVOVF + 2u);
    finaldv_plus3();
    set_A(ONE);
    ts(ADDRESS_OF(OVFIND));
    tc_at(DVOVF + 5u);
    continue_at(DANZIG);
}

/* OVF+ -1, OVF+: the dividend's major part is not zero. "LOAD LOWER ORDER
 * PART OF DIVISOR" and "GET SIGN OF RESULT." */
void ovf_plus_minus1(void)
{
    incr(ADDRESS_OF(DVSIGN));
    ovf_plus();
}

static void ovf_plus(void)
{
    cs(BUF_AT(1u));
    sgndvovf();
}

/* BUF-: "IF BUF IS NEGATIVE, COMPLEMENT IT AND MAINTAIN DVSIGN FOR FINAL
 * QUOTIENT SIGN." */
static void buf_minus(void)
{
    dcs(BUF_AT(0u));
    dxch(BUF_AT(0u));
    incr(ADDRESS_OF(DVSIGN)); /* "NOW -0." */
    buf_plus();
}

/* BUF+ -- LM p. 1058, CM p. 1163. "FORCE MPAC POSITIVE, CHECKING FOR ZERO
 * DIVIDEND IN THE PROCESS." "EXIT IMMEDIATELY ON ZERO DIVIDEND." */
static void buf_plus(void)
{
    enum sign major = ccs_at(MPAC_AT(0u));
    if (major == POSITIVE) {
        mpac_plus();
        return;
    }
    if (major == NEGATIVE) {
        mpac_minus();
        return;
    }
    switch (ccs_at(MPAC_AT(1u))) {
    case POSITIVE: mpac_plus(); return;
    case NEGATIVE: mpac_minus(); return;
    default: continue_at(DANZIG); return;
    }
}

/* MPAC-: "FORCE MPAC POSITIVE AS BUF IN BUF-." */
static void mpac_minus(void)
{
    dcs(MPAC_AT(0u));
    dxch(MPAC_AT(0u));
    incr(ADDRESS_OF(DVSIGN)); /* "NOW +1 OR -0." */
    mpac_plus();
}

/* MPAC+ -- LM p. 1059, CM p. 1164.
 * "CHECK FOR DIVISION OVERFLOW. IF THE MAJOR PART OF THE DIVIDEND IS LESS
 * THAN THE MAJOR PART OF THE DIVISOR BY AT LEAST TWO, WE CAN PROCEED
 * IMMEDIATELY WITHOUT NORMALIZATION PRODUCING A DVMAX." "IF THE ABOVE DOES
 * NOT HOLD, FORCE SIGN AGREEMENT IN NUMERATOR AND DENOMINATOR TO
 * FACILITATE OVERFLOW AND NEAR-ONE CHECKING", and compare again. "IF THE
 * MAJOR PARTS OF THE DIVIDEND AND DIVISOR ARE EQUAL, A SPECIAL
 * APPROXIMATION IS USED" (MAXDVSW +0). */
static void mpac_plus(void)
{
    cs(MPAC_AT(0u));
    ad(NEGONE);
    ad_at(BUF_AT(0u));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: dvnorm(); return;
    case IS_PLUS_ZERO:
        /* The next word, -1/2+2 ("USED IN SQRTSUB"), is OCT 60001: it runs
         * as AD L, and the TCF +1 after it goes on. */
        ad_at(L_ADDRESS);
        break;
    default: break;
    }

    set_A(HALF);
    double_A();
    ad_at(MPAC_AT(1u));
    if (!ts(MPAC_AT(1u))) set_A(ZERO);
    ad(POSMAX);
    ads(MPAC_AT(0u));

    set_A(HALF); /* "SAME FOR BUF." */
    double_A();
    ad_at(BUF_AT(1u));
    if (!ts(BUF_AT(1u))) set_A(ZERO);
    ad(POSMAX);
    ads(BUF_AT(0u));

    cs(MPAC_AT(0u)); /* "CHECK MAGNITUDE OF SIGN-CORRECTED OPERANDS." */
    ad_at(BUF_AT(0u));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: dvnorm(); return; /* "DIVIDE OK -- WILL NOT BECOME MAXOV CASE." */
    case IS_PLUS_ZERO:
        /* (never: the majors differed by one before the agreement. The next
         * word, LBUF2, ADRES BUF2, would run as TC BUF2.) */
        tc_at(ADDRESS_OF(LBUF2));
        continue_at(LBUF2);
        return;
    case NEGATIVE: dvovf(); return; /* "DIVISOR NOT LESS THAN DIVIDEND -- OVF." */
    case IS_MINUS_ZERO: break;
    }

    ts(ADDRESS_OF(MAXDVSW)); /* the majors are equal: MAXDVSW +0 */
    cs(MPAC_AT(1u));
    ad_at(BUF_AT(1u));
    if (A_is_zero_or_minus())
        dvovf();
    else
        dvnorm(); /* "IF NO OVERFLOW." */
}

/* DVNORM, BUFNORM -- LM p. 1060, CM p. 1165.
 * "SEE IF DIVISOR NORMALIZED YET": "NO -- SHIFT LEFT ONE AND TRY AGAIN"
 * ("ADD -1 TO AUGMENT SHIFT COUNT"). Then "CALL DIVIDEND NORMALIZATION
 * SEQUENCE PRIOR TO DOING THE DIVIDE", which "RETURNS WITH DIVISION DONE
 * AND C(A) = 0." */
static void dvnorm(void)
{
    for (;;) {
        ca(BUF_AT(0u));
        double_A();
        if (A_has_overflow()) break;

        aug(ADDRESS_OF(DVNORMCT)); /* BUFNORM */
        dca(BUF_AT(0u));
        das(BUF_AT(0u));
    }

    dxch(MPAC_AT(0u));
    tc_at(DVNORM + 6u);
    into_maxtest(MAXTEST, ADDRESS_OF(DVNORMCT)); /* INDEX DVNORMCT; TC MAXTEST */

    ts(MPAC_AT(2u));
    continue_at(DANZIG);
}

/* BUFPOS, BUFNEG: BUF's major part is +1 or -1 (A, from the CCS, is its
 * magnitude less one, so is never negative here): "FORCING SIGN AGREEMENT
 * MAY CAUSE BUF TO BECOME ZERO." */
static void bufpos(void)
{
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        buf_plus(); /* "TO BUF+ IF BUF IS GREATER THAN +1." */
        return;
    }
    cs(BUF_AT(1u));
    if (A_is_zero_or_minus()) {
        buf_plus(); /* "BRANCH IF SIGNS AGREE." */
        return;
    }
    set_A(HALF); /* "SIGNS DISAGREE. FORCE AGREEMENT." */
    bufpos_plus6();
}

static void bufpos_plus6(void)
{
    double_A();
    ads(BUF_AT(1u));
    set_A(ZERO);
    ts(BUF_AT(0u));
    bufzero();
}

static void bufneg(void)
{
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        buf_minus(); /* "TO BUF- IF BUF IS LESS THAN -1." */
        return;
    }
    ca(BUF_AT(1u));
    if (A_is_zero_or_minus()) {
        buf_minus(); /* "BRANCH IF SIGNS AGREE." */
        return;
    }
    set_A(negate(HALF)); /* "SIGNS DISAGREE. FORCE AGREEMENT." */
    bufpos_plus6();
}

/* ======================================================================
 * The dividend's normalization: MAXTEST and its prologues
 * ====================================================================== */

/* INDEX k; TC (or TCF) base: k holds minus the number of words before
 * base to enter at. DVNORM and V/SCDV enter before MAXTEST by the shift
 * count DVNORMCT (-1 to -14), UNIT before MAXTEST -1 by MPTEMP (to -20,
 * which is MAXTEST -21D). */
void into_maxtest(word base, unsigned k)
{
    unsigned at = indexed(base, fetch(k));
    unsigned before = (unsigned)MAXTEST - at; /* the n of MAXTEST -n */

    if (at <= MAXTEST && before <= 14u)
        maxtest_minus(before);
    else if (before == 21u)
        maxtest_minus(21u);
    else
        continue_at(at); /* (never: no count goes outside the prologues) */
}

/* MAXTEST -n -- LM p. 1061, CM p. 1166. "THE FOLLOWING ARE PROLOGUES TO
 * SHIFT THE DIVIDEND ARRIVING IN A AND L BEFORE THE DIVIDE."
 *
 * MAXTEST -21D: "SPECIAL PROLOGUE FOR UNIT WHEN THE LENGTH OF THE ARGUMENT
 * WAS NOT LESS THAN .5. IN THIS CASE, EACH COMPONENT MUST BE SHIFTED RIGHT
 * ONE TO PRODUCE A HALF-UNIT VECTOR."
 *
 * MAXTEST -14 to -1: thirteen DDOUBLs and a DXCH MPAC, the "PROLOGUE WHICH
 * NORMALIZES THE DIVIDEND WHEN IT IS KNOWN THAT NO DIVISION OVEFLOW WILL
 * OCCUR": entered n words back, it doubles the dividend n - 1 times. */
void maxtest_minus(unsigned n)
{
    if (n == 21u) {
        lxch(SR_ADDRESS);
        mp(HALF);
        xch(L_ADDRESS);
        ad_at(SR_ADDRESS);
        xch(L_ADDRESS);
        genddv_plus1(); /* "WITH DP DIVIDEND IN A,L." */
        return;
    }
    for (; n > 1u; --n) ddoubl();
    if (n == 1u) dxch(MPAC_AT(0u));
    maxtest();
}

/* MAXTEST: MAXDVSW is "0 IF MAJORS MIGHT BE =, -1 OTHERWISE": MAXDV, or
 * GENDDV. (It is never positive; the next word, BIASHI, a SQRT constant,
 * would run as TC 6552.) */
void maxtest(void)
{
    switch (ccs_at(ADDRESS_OF(MAXDVSW))) {
    case POSITIVE:
        tc_at(ADDRESS_OF(BIASHI));
        continue_at(BIASHI);
        return;
    case IS_PLUS_ZERO: maxdv(); return; /* "CHECK TO SEE IF THAY ARE NOW EQUAL." */
    case NEGATIVE: genddv(); return;
    case IS_MINUS_ZERO: genddv_plus1(); return;
    }
}

/* ======================================================================
 * The general divide
 * ====================================================================== */

/* GENDDV -- LM pp. 1062-1064, CM pp. 1167-1169.
 * "A GENERAL PURPOSE DOUBLE PRECISION DIVISION ROUTINE. IT DIVIDES MPAC BY
 * BUF AND LEAVES THE RESULT IN MPAC." "THE DIVISOR (BUF) MUST BE POSITIVE
 * AND NOT LESS THAN .5." "THE DIVIDEND (MPAC) MUST BE POSITIVE WITH THE
 * MAJOR PART OF MPAC STRICTLY LESS THAN THAT OF BUF."
 *
 * With A + SB over C + SD (S = 2^-14), Q and R the quotient and remainder
 * of (A + SB) / C: "A + SB / C + SD = Q + S((R - QD) / C)". The first DV
 * gives Q and R, the second the minor part from R - QD, brought within
 * range of C first (with a compensating change to Q). */
static void genddv(void)
{
    dxch(MPAC_AT(0u)); /* "WE NEED A AND B ONLY FOR FIRST DV." */
    genddv_plus1();
}

/* GENDDV +1: "(SPECIAL UNIT PROLOGUE ENTERS HERE)." */
static void genddv_plus1(void)
{
    dv_at(BUF_AT(0u)); /* "A NOW CONTAINS Q AND L, R." */
    dxch(MPAC_AT(0u));

    cs(MPAC_AT(0u)); /* "FORM DIVIDEND FOR MINOR PART OF RESULT." */
    mp_at(BUF_AT(1u));
    ad_at(MPAC_AT(1u));
    if (A_has_overflow()) {
        /* "OVERFLOW AT THIS POINT IS POSITIVE SINCE R IS POSITIVE IN EVERY
         * CASE." "OVERFLOW CAN BE REMOVED BY SUBTRACTING C (BUF) ONCE SINCE
         * R IS ALWAYS LESS THAN C IN THIS CASE. INCR COMPENSATES
         * SUBTRACT." */
        su_at(BUF_AT(0u));
        incr(MPAC_AT(0u));
        plus_down(); /* "(SINCE C(A) IS STILL POSITIVE)." */
        return;
    }
    /* "C(A) CAN BE MADE LESS THAN C IN MAGNITUDE BY DIMINISHING IT BY C
     * (SINCE C IS NOT LESS THAN .5) UNLESS C(A) = 0." */
    if (A_is_zero_or_minus())
        minus_up();
    else
        plus_down();
}

/* +DOWN: "IF POSITIVE, REDUCE ONLY IF NECESSARY SINCE THE COMPENSATING
 * INCR MIGHT CAUSE OVERFLOW." "DON'T SUBTRACT UNLESS RESULT IS POSITIVE OR
 * ZERO." */
static void plus_down(void)
{
    su_at(BUF_AT(0u));
    if (!A_is_zero() && A_is_zero_or_minus()) {
        endmaxdv();
        return;
    }
    incr(MPAC_AT(0u)); /* "KEEP SUBTRACT HERE AND COMPENSATE." */
    finaldv();
}

/* -UP: "IF ZERO, SET MINOR PART OF RESULT TO ZERO." "IF NEGATIVE, ADD C TO
 * A, SUBTRACTING ONE TO COMPENSATE. DIM IS OK HERE SINCE THE MAJOR PART
 * NEVER GOES NEGATIVE." */
static void minus_up(void)
{
    if (A_is_zero()) {
        finaldv_plus3();
        return;
    }
    dim(MPAC_AT(0u));
    endmaxdv();
}

static void endmaxdv(void)
{
    ad_at(BUF_AT(0u));
    finaldv();
}

/* FINALDV -- LM p. 1064, CM p. 1169. "DO DV TO OBTAIN MINOR PART OF
 * RESULT." */
static void finaldv(void)
{
    zl();
    dv_at(BUF_AT(0u));
    finaldv_plus3();
}

/* FINALDV +3: "LEAVE RESULT POSITIVE UNLESS C(DVSIGN) = -0." "SO WE ALWAYS
 * RETURN WITH C(A) = 0." (An overflowing TS skips the CCS, to the first
 * TC Q: DVOVF's quotient.) */
void finaldv_plus3(void)
{
    if (ts(MPAC_AT(1u)) || ccs_at(ADDRESS_OF(DVSIGN)) != IS_MINUS_ZERO) {
        tc_through(Q_ADDRESS);
        return;
    }
    dcs(MPAC_AT(0u));
    dxch(MPAC_AT(0u));
    set_A(ZERO);
    tc_through(Q_ADDRESS);
}

/* MAXDV -- LM p. 1065, CM p. 1170.
 * "IF THE MAJOR PARTS OF THE DIVISOR AND DIVIDEND ARE EQUAL, BUT THE MINOR
 * PARTS ARE SUCH THAT THE DIVIDEND IS STRICTLY LESS THAN THE DIVISOR IN
 * MAGNITUDE, THE FOLLOWING APPROXIMATION IS USED":
 * "C + SB / C + SD = 37777 + S((C + B - D) / C)".
 * "SEE IF MAXDV CASE STILL HOLDS AFTER NORMALIZATION." */
static void maxdv(void)
{
    cs(MPAC_AT(0u));
    ad_at(BUF_AT(0u));
    if (!A_is_zero()) {
        genddv(); /* "MPAC NOW LESS THAN BUFF -- DIVIDE AS USUAL." */
        return;
    }
    set_A(POSMAX); /* "SET MAJOR PART OF RESULT." */
    ts(MPAC_AT(0u));

    cs(BUF_AT(1u)); /* "FORM DIVIDEND OF MINOR PART OF RESULT." */
    ad_at(MPAC_AT(1u));
    endmaxdv(); /* "GO ADD C AND DO DIVIDE, ATTACHING SIGN BEFORE EXITING." */
}

/* ======================================================================
 * V/SC
 * ====================================================================== */

/* V/SC2 -- LM p. 1066, CM p. 1171.
 * "VECTOR DIVIDED BY SCALAR, V/SC, IS EXECUTED HERE. THE VECTOR IS NOW IN
 * MPAC WITH SCALAR IN BUF." The sign of the scalar is kept in VBUF +5;
 * each component then divides as DDV does. */
void v_sc2(void)
{
    set_A(negate(ONE));       /* "INITIALIZE DIVIDEND NORMALIZATION COUNT */
    ts(ADDRESS_OF(DVNORMCT)); /* AND DIVISION SIGN REGISTER." */
    ts(ADDRESS_OF(VBUF) + 5u);

    tc_at(V_SC2 + 3u);
    vecagree(); /* "FORCE SIGN AGREEMENT IN VECTOR" */

    dxch(BUF_AT(0u));
    tc_at(V_SC2 + 5u);
    alsignag(); /* "SIGN AGREE BUF" */
    dxch(BUF_AT(0u));

    switch (ccs_at(BUF_AT(0u))) { /* "FORCE DIVISOR POSITIVE WITH MAJOR PART
                                   * NON-ZERO (IF POSSIBLE)." */
    case POSITIVE: slash_buf_plus(); return;
    case NEGATIVE: slash_buf_minus(); return;
    default: break;
    }

    /* "SHIFT VECTOR AND SCALAR LEFT 14." "CHECK FOR OVERFLOW IN EACH
     * CASE." */
    xch(BUF_AT(1u));
    xch(BUF_AT(0u));
    xch(MPAC_AT(1u));
    xch(MPAC_AT(0u));
    if (!A_is_zero()) {
        dvovf();
        return;
    }
    xch(MPAC_AT(4u));
    xch(MPAC_AT(3u));
    if (!A_is_zero()) {
        dvovf();
        return;
    }
    xch(MPAC_AT(6u));
    xch(MPAC_AT(5u));
    if (!A_is_zero()) {
        dvovf();
        return;
    }

    switch (ccs_at(BUF_AT(0u))) {
    case POSITIVE: slash_buf_plus(); return;
    case NEGATIVE: slash_buf_minus(); return;
    default: dvovf(); return; /* "ZERO DIVISOR - OVERFLOW." */
    }
}

/* /BUF-: "ON NEGATIVE, COMPLEMENT BUF AND MAINTAIN DVSIGN IN VBUF +5." */
static void slash_buf_minus(void)
{
    dcs(BUF_AT(0u));
    dxch(BUF_AT(0u));
    incr(ADDRESS_OF(VBUF) + 5u);
    slash_buf_plus();
}

/* /BUF+ -- LM p. 1067, CM p. 1172. "LEAVE ABS(ORIG DIVISOR) IN BUF2 FOR
 * OVERFLOW TESTING", "NORMALIZE DIVISOR IN BUF" (/NORM, /NORM2: "IF LESS
 * THAN .5, AUGMENT DVNORMCT AND DOUBLE DIVISOR"), and divide each
 * component, rotating it into MPAC. */
static void slash_buf_plus(void)
{
    dca(BUF_AT(0u));
    dxch(BUF2_AT(0u));

    for (;;) {
        ca(BUF_AT(0u)); /* /NORM: "SEE IF DIVISOR NORMALIZED." */
        double_A();
        if (A_has_overflow()) break;

        aug(ADDRESS_OF(DVNORMCT)); /* /NORM2 */
        dca(BUF_AT(0u));
        das(BUF_AT(0u));
    }

    tc_at(SLASH_NORM + 4u);
    if (v_scdv()) return; /* "DO X COMPONENT DIVIDE." */
    dxch(MPAC_AT(3u));    /* "SUPPLY ARGUMENTS IN USUAL SEQUENCE." */
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(3u));

    tc_at(SLASH_NORM + 8u);
    if (v_scdv()) return; /* "Y COMPONENT." */
    dxch(MPAC_AT(5u));
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(5u));

    tc_at(SLASH_NORM + 12u);
    if (v_scdv()) return; /* "Z COMPONENT." */
    vrotatex();           /* "GO RE-ARRANGE COMPONENTS BEFORE EXIT." */
}

/* V/SCDV -- LM p. 1068, CM p. 1173.
 * "SUBROUTINE USED BY V/SC TO DIVIDE VECTOR COMPONENT IN MPAC,+1 BY THE
 * SCALAR GIVEN IN BUF." "FORCE MPAC POSITIVE, EXITING ON ZERO." Its
 * result says whether it ended the order at DVOVF instead of returning. */
static bool v_scdv(void)
{
    enum sign major;

    ca(ADDRESS_OF(VBUF) + 5u); /* "REFLECTS SIGN OF SCALAR." */
    ts(ADDRESS_OF(DVSIGN));

    major = ccs_at(MPAC_AT(0u));
    if (major != POSITIVE && major != NEGATIVE) {
        major = ccs_at(MPAC_AT(1u));
        if (major != POSITIVE && major != NEGATIVE) {
            tc_through(Q_ADDRESS);
            return false;
        }
    }
    if (major == NEGATIVE) { /* /MPAC-: "USUAL COMPLEMENTING AND SETTING OF SIGN." */
        dcs(MPAC_AT(0u));
        dxch(MPAC_AT(0u));
        incr(ADDRESS_OF(DVSIGN));
    }

    set_A(negate(ONE)); /* /MPAC+: "INITIALIZE NEAR-ONE SWITCH." */
    ts(ADDRESS_OF(MAXDVSW));

    cs(MPAC_AT(0u));    /* "CHECK POSSIBLE OVERFLOW." */
    ad_at(BUF2_AT(0u)); /* "UNNORMALIZED INPUT DIVISOR." */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: break;                /* "NOT NEAR-ONE" */
    case NEGATIVE: dvovf(); return true; /* "NO HOPE" */
    default:                             /* "+0 IS JUST POSSIBLE" */
        ts(ADDRESS_OF(MAXDVSW));         /* "SIGNAL POSSIBLE NEAR-ONE CASE" */
        cs(MPAC_AT(1u));                 /* "SEE IF DIVISION CAN BE DONE" */
        ad_at(BUF2_AT(1u));
        if (A_is_zero_or_minus()) {
            dvovf();
            return true;
        }
        break;
    }

    /* DDVCALL: "CALL PRE-DIVIDE NORMALIZATION." The TCF leaves Q at the
     * caller's return, which the divide returns to. */
    dxch(MPAC_AT(0u));
    into_maxtest(MAXTEST, ADDRESS_OF(DVNORMCT));
    return false;
}

/* VECAGREE -- LM p. 1069, CM p. 1174.
 * Sign agreement for each component of the vector in MPAC (ALSIGNAG). The
 * return is kept in A ("SAVE Q IN A"), which the DXCHs carry through MPAC
 * while ALSIGNAG works on A and L; TC A returns. */
void vecagree(void)
{
    xch(Q_ADDRESS);
    dxch(MPAC_AT(0u));
    tc_at(VECAGREE + 2u);
    alsignag(); /* "SIGNAGREE MPAC" */
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(3u));
    tc_at(VECAGREE + 5u);
    alsignag(); /* "SIGN AGREE MPAC +3" */
    dxch(MPAC_AT(3u));
    dxch(MPAC_AT(5u));
    tc_at(VECAGREE + 8u);
    alsignag(); /* "SIGNAGREE MPAC +5" */
    dxch(MPAC_AT(5u));
    tc_through(A_ADDRESS);
}
