/* SPDX-License-Identifier: MIT */
/* The trigonometric function package: INTERPRETER.agc LM pp. 1082-1086, CM
 * pp. 1187-1191 (in bank 0). The faithful C is c64-port/src/lm_interp_core.c
 * (lmi_cos, sine, arccos_body, acos0, escape, asinex).
 *
 * "THE FOLLOWING TRIGONOMETRIC FUNCTIONS ARE AVAIALABLE AS INTERPRETIVE
 * OPERATIONS:
 *	1. SIN	COMPUTES (1/2)SINE(2 PI MPAC).
 *	2. COS	COMPUTES (1/2)COSINE(2 PI MPAC).
 *	3. ASIN	COMPUTES (1/2PI)ARCSINE(2 MPAC).
 *	4. ACOS	COMPUTES (1/2PI)ARCCOSINE(2 MPAC).
 * SIN-ASIN AND COS-ACOS ARE MUTUALLY INVERSE, I.E., SIN(ASIN(X)) = X."
 *
 * Angles are in revolutions, so 2 PI MPAC is the angle: .25 is PI/2. Both
 * SIN and ACOS use polynomial approximations from Hastings. */
#include "interpreter.h"
#include "../agc/names.h"

static void cosine_plus4(void);
static void presine(void);
static void sine_plus6(void);
static void sn1(void);
static void acosst(void);
static void acos_eq_0(void);
static void acos_eq_0_plus1(void);
static void acosst2(void);
static void acos3(void);
static void acos3_plus1(void);
static void acos3_plus2(void);
static void subtr(void);
static void asinex(void);
static void acosshr(void);
static void acosovf(void);
static void acosabrt(void);
static void acoszero(void);
static void tc_escape(word site, unsigned escape);

/* ======================================================================
 * SIN and COS
 * ====================================================================== */

/* COSINE -- LM p. 1082, CM p. 1187. "FINDS COSINE USING THE IDENTITY
 * COS(X) = SIN(PI/2 - ABS(X))." (Here -ABS(X) + PI/2.) */
void cosine(void)
{
    tc_at(COSINE);
    if (branch() == BRANCH_POSITIVE)
        cosine_plus4();
    else
        presine();
}

static void cosine_plus4(void)
{
    dcs(MPAC_AT(0u));
    dxch(MPAC_AT(0u));
    presine();
}

static void presine(void)
{
    set_A(QUARTER); /* "PI/2 SCALED." */
    ads(MPAC_AT(0u));
    sine();
}

/* SINE -- LM p. 1082, CM p. 1187. "DOUBLE ARGUMENT." "IF SO, WE LOST (OR
 * GAINED) PI, SO COMPLEMENT MPAC USING THE IDENTITY SIN(X-(+)PI) =
 * SIN(-X)." Then "SEE IF ARGUMENT GREATER THAN .5 IN MAGNITUDE. IF SO,
 * REDUCE IT TO LESS THAN .5 (+-PI/2 SCALED) AS FOLLOWS: IF POSITIVE, FORM
 * PI - X, IF NEGATIVE USE -PI -X." */
void sine(void)
{
    dxch(MPAC_AT(0u));
    sine_plus1();
}

void sine_plus1(void)
{
    ddoubl();
    if (A_has_overflow()) dcom(); /* "SEE IF OVERFLOW PRESENT." */
    sine_plus6();
}

static void sine_plus6(void)
{
    dxch(MPAC_AT(0u));
    ca(MPAC_AT(0u));
    double_A();
    if (!ts(L_ADDRESS)) {
        sn1();
        return;
    }
    /* A is +1 or -1: NEG1/2 +1 indexed by it is +.5 or -.5, doubled PI or
     * -PI. */
    ca(indexed(ADDRESS_OF(NEG1_2) + 1u, a_word()));
    double_A();
    su_at(MPAC_AT(0u)); /* "GUARANTEED NO OVERFLOW." */
    ts(MPAC_AT(0u));
    cs(MPAC_AT(1u));
    ts(MPAC_AT(1u));
    sn1();
}

/* SN1 -- LM p. 1083, CM p. 1188. "SET UP TO EVALUATE HASTINGS POLYNOMIAL":
 * x (2 PI x) is kept in BUF2, the polynomial in x squared evaluated
 * ("FOURTH ORDER POLYNOMIAL", coefficients after the TC POLY), multiplied
 * by x and shifted left 2. */
static void sn1(void)
{
    dca(MPAC_AT(0u));
    dxch(ADDRESS_OF(BUF2));
    tc_at(SN1 + 3u);
    dsqsub(); /* "SQUARE MPAC." */

    tc_at(SN1 + 4u);
    poly(); /* "EVALUATE FOURTH ORDER POLYNOMIAL." It returns to SN1 +16. */
    sn1_plus16();
}

/* SN1 +16: "MULTIPLY BY ARGUMENT AND SHIFT LEFT 2." "NEITHER SHIFT
 * OVERFLOWS." */
void sn1_plus16(void)
{
    unsigned shift;

    set_A(LBUF2);
    tc_at(SN1 + 17u);
    dmpsub_minus1();

    for (shift = 0; shift < 2u; ++shift) {
        dca(MPAC_AT(1u));
        das(MPAC_AT(1u));
        ad_at(MPAC_AT(0u));
        ads(MPAC_AT(0u));
    }
    continue_at(DANZIG);
}

/* ======================================================================
 * ASIN and ACOS
 * ====================================================================== */

/* ARCSIN, ARCCOS -- LM p. 1084, CM p. 1189. "COMPUTE ARCSIN BY USING THE
 * IDENTITY ARCSIN(X) = PI/2 - ARCCOS(X)." Both compute ARCCOS, then leave
 * through the TCF kept in ESCAPE: to ASINEX for ARCSIN, to DANZIG for
 * ARCCOS ("EXITS IMMEDIATELY"). A negative input goes through SUBTR first
 * ("ARCCOS(X) = PI - ARCCOS(-X)"), which then leaves through ESCAPE2. */
void arcsin(void)
{
    set_A(LASINEX);
    arccos_plus1();
}

void arccos(void)
{
    set_A(LDANZIG);
    arccos_plus1();
}

void arccos_plus1(void)
{
    ts(ADDRESS_OF(ESCAPE));
    tc_at(ARCCOS + 2u);
    switch (branch()) {                     /* "TEST SIGN OF INPUT." */
    case BRANCH_POSITIVE: acosst(); return; /* "START IMMEDIATELY IF POSITIVE." */
    case BRANCH_ZERO: acoszero(); return;   /* "ARCCOS(0) = PI/2 = .25." */
    case BRANCH_NEGATIVE: break;
    }
    dcs(MPAC_AT(0u)); /* "FORCING ARGUMENT POSITIVE." */
    dxch(MPAC_AT(0u));
    set_A(TCSUBTR);          /* "SET EXIT TO DO ABOVE BEFORE ARCSIN/ARCCOS */
    xch(ADDRESS_OF(ESCAPE)); /* CONSIDERATIONS." */
    ts(ADDRESS_OF(ESCAPE2));
    acosst();
}

/* ACOSST: "TEST MAGNITUDE OF INPUT." Over .5 is "PROBABLY AN OVERFLOW
 * CASE"; exactly .5 in the major part (which the add makes -0) is "ANSWER
 * 0 UNLESS MINOR PART NEGATIVE". (The +0 slot never comes: it is the word
 * LASINEX, TCF ASINEX, that ARCSIN keeps in ESCAPE.) */
static void acosst(void)
{
    set_A(negate(HALF));
    ad_at(MPAC_AT(0u));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: acosovf(); return;
    case IS_PLUS_ZERO: asinex(); return;
    case NEGATIVE: acosst2(); return; /* "NO OVERFLOW -- PROCEED." */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(MPAC_AT(1u))) {
    case POSITIVE:
        set_A(ZERO);
        acos_eq_0();
        return;
    case NEGATIVE: acosst2(); return;
    default: acos_eq_0(); return;
    }
}

/* ACOS=0: the answer, A (+0, or .25 from ACOSZERO), to MPAC, "SET MPAC AND
 * EXIT VIA ESCAPE." */
static void acos_eq_0(void)
{
    ts(MPAC_AT(1u));
    acos_eq_0_plus1();
}

static void acos_eq_0_plus1(void)
{
    ts(MPAC_AT(0u));
    tc_escape(ACOS_EQ_0 + 2u, ADDRESS_OF(ESCAPE));
}

/* ACOSST2: "NOW THAT ARGUMENT IS IN PROPER RANGE, BEGIN COMPUTATION. USE
 * HASTINGS APPROXIMATION ARCCOS(X) = SQRT(1-X)P(X) IN A SCALED VERSION
 * WHERE P(X) IS A SEVENTH ORDER POLYNOMIAL." X is kept in BUF2 while
 * SQRTSUB takes the root of 1 - X. "SEE IF UN-NORMALIZATION REQUIRED."
 * (SQRTSUB's count is never negative: the other slots enter ACOS3 one and
 * two words in.) */
static void acosst2(void)
{
    dcs(MPAC_AT(0u));
    ad(HALF);
    dxch(MPAC_AT(0u));
    dxch(ADDRESS_OF(BUF2));

    tc_at(ACOSST2 + 5u);
    if (sqrtsub()) return; /* "RETURNS WITH NORMALIZED SQUARE ROOT." */

    switch (ccs_at(ADDRESS_OF(MPTEMP))) {
    case POSITIVE: acosshr(); return; /* "IF SO." */
    case IS_PLUS_ZERO: acos3(); return;
    case NEGATIVE: acos3_plus1(); return;
    case IS_MINUS_ZERO: acos3_plus2(); return;
    }
}

/* ACOS3 -- LM p. 1085, CM p. 1190. "SET UP FOR POLYNOMIAL EVALUATION": X
 * into MPAC and the root into BUF2; then the polynomial's "COEFFICIENTS
 * ARE C 2(+I)/PISQRT(2) ... WHERE C STANDS FOR ORIGINAL COEFFS.", follow
 * the TC POLY. "DO FINAL MULTIPLY AND GO TO ANY EPILOGUE SEQUENCES." */
static void acos3(void)
{
    dxch(MPAC_AT(0u));
    acos3_plus1();
}

static void acos3_plus1(void)
{
    dxch(ADDRESS_OF(BUF2));
    acos3_plus2();
}

static void acos3_plus2(void)
{
    dxch(MPAC_AT(0u));

    tc_at(ACOS3 + 3u);
    poly(); /* (it returns to ACOS3 +21) */

    set_A(LBUF2);
    tc_at(ACOS3 + 22u);
    dmpsub_minus1();
    tc_escape(ACOS3 + 23u, ADDRESS_OF(ESCAPE));
}

/* SUBTR: "EPILOGUE FOR NEGATIVE INPUTS TO ARCCOS." "FORMS PI - ARCCOS(-X) =
 * ARCCOS(X)." "GO TO POSSIBLE ARCSIN EPILOGUE." */
static void subtr(void)
{
    dcs(MPAC_AT(0u));
    ad(HALF);
    dxch(MPAC_AT(0u));
    tc_escape(SUBTR + 4u, ADDRESS_OF(ESCAPE2));
}

/* ASINEX: "ARCSIN EPILOGUE -- GET ARCSIN(X) = PI/2 - ARCCOS(X)." Then
 * LDANZIG, TCF DANZIG. */
static void asinex(void)
{
    dcs(MPAC_AT(0u));
    ad(QUARTER);
    dxch(MPAC_AT(0u));
    continue_at(DANZIG);
}

/* TC ESCAPE (or ESCAPE2) at site: the erasable word holds the TCF that
 * ARCSIN or ARCCOS put there (LASINEX, LDANZIG or TCSUBTR), and the TC runs
 * it. Any other word (never) the AGC would run as it found it. */
static void tc_escape(word site, unsigned escape)
{
    word instruction = fetch(escape);

    tc_at(site);
    if (instruction == TCSUBTR)
        subtr();
    else if (instruction == LASINEX)
        asinex();
    else if (instruction == LDANZIG)
        continue_at(DANZIG);
    else
        continue_at((word)escape);
}

/* ACOSSHR -- LM p. 1086, CM p. 1191. "THE SHIFT RIGHT IS LESS THAN 14 SINCE
 * THE INPUT WAS NON-ZERO DP." "DP SHIFT RIGHT AND ROUND." */
static void acosshr(void)
{
    ca(indexed(ADDRESS_OF(BIT14), a_word()));
    ts(ADDRESS_OF(MPTEMP));
    tc_at(ACOSSHR + 3u);
    vshrrnd();
    acos3(); /* "PROCEED." */
}

/* ACOSOVF: "IF MAJOR PART WAS ONLY 1 MORE THAN .5, CALL ANSWER ZERO." */
static void acosovf(void)
{
    if (A_is_zero())
        acos_eq_0();
    else
        acosabrt();
}

/* ACOSABRT: "IF OVERFLOW, CALL ANSWER ZERO BUT SOUND AN ALARM": alarm 1301
 * (the OCT after the TC). The LM's ALARM1 takes the interpretive return
 * from LOC in A,L; the CM's ALARM the plain one. Both return after the OCT,
 * to CAF ZERO; TCF ACOS=0, in basic code. */
static void acosabrt(void)
{
#if LUMINARY099
    dca(ADDRESS_OF(LOC));
    tc_at(ACOSABRT + 2u);
    continue_at(ALARM1);
#else
    tc_at(ACOSABRT);
    continue_at(ALARM);
#endif
}

/* ACOSZERO: "ACOS(0) = PI/2." "SET MPAC AND EXIT VIA ESCAPE." */
static void acoszero(void)
{
    set_A(QUARTER);
    acos_eq_0_plus1();
}
