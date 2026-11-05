/* SPDX-License-Identifier: MIT */
/* FINDCDUW--GUIDAP_INTERFACE.agc, pp. 919-924 (Luminary099, bank 30): the
 * subroutines. FLTRSUB, the thrust direction filter's step; UNWCTEST, the
 * window's test; NB2CDUSP, the gimbal angles of a direction cosine matrix,
 * through DVBYCOSM and ARCTRGSP; SPARCSIN, LIMITSUB and ONESTO2S. The
 * faithful C is c64-port/src/lm_findcduw.c. */
#include <stdbool.h>
#include "findcduw.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* TC Q: back to the caller (Z the return, Q 3). Not modelled: a word in Q
 * that is no TC to an address (bits 13-15 set, or below 010), which the
 * faithful port refuses. */
static void return_to_q(void)
{
    word to = (word)Q;

    if ((to & 070000u) || to < 010u) not_modelled();
    tc_through(Q_ADDRESS);
    continue_at((word)(to & 07777u));
}

/* TC to an erasable word that holds a return (FLTRSUB's TC TEM2,
 * ARCTRGSP's TC TEM4): the AGC runs the word as TC <address>, which the
 * faithful port takes as leaving the file. Not modelled: a word that is
 * no TC to an address, as above. */
void leave_through(unsigned address)
{
    word to = fetch(address);

    if ((to & 070000u) || to < 010u) not_modelled();
    tc_through(address);
}

/* ---- FLTRSUB -- p. 919 ---- */

/* FLTRSUB, "THRUST VECTOR FILTER SUBROUTINE": the filter's offset (in A)
 * moves toward the measured one (in L) by GAINFLTR of the difference
 * (.2, or .1 with the CSM docked), "INCR TO OFFSET, LIMITED" to DUNFVLIM,
 * "7 MR MAX CHG IN F DIR IN VEH IN 2 SECS". The return waits in TEM2.
 * ("ONE MCT, NO WDS, CAN BE SAVED IF NEG OF ORIG OFFSET ARRIVES IN A, BUT
 * IT'S NOT WORTH THE INCREASED OBSCURITY.") (An A that overflows skips the
 * step after its TS, as each test here has it; the offset is a word, the
 * product too.) */
void fltrsub(void)
{
    qxch(ADDRESS_OF(TEM2));
    if (!ts(ADDRESS_OF(TEM3))) com(); /* "SAVE ORIGINAL OFFSET" */
    ad_at(L_ADDRESS);
    mp(docked_constant(MP_CODE | GAINFLTR));
    if (!ts(L_ADDRESS)) set_A(DUNFVLIM); /* "SAME LIMIT FOR Y AND Z" */
    tc_at(FLTRSUB + 012u);
    limitsub();
    fltrsub_after_increment();
}

/* LIMITSUB's return: the "TOTAL OFFSET", limited to UNFVLIM, "129 MR MAX
 * THRUST OFFSET. 105 MR TRAVEL +10MR DEFL+5MR MECH MOUNT+9MR ABLATION."
 * (A sum that overflows skips the CA: LIMITSUB then limits L to +1 or
 * -1.) */
void fltrsub_after_increment(void)
{
    ad_at(ADDRESS_OF(TEM3));
    if (!ts(L_ADDRESS)) set_A(UNFVLIM);
    tc_at(FLTRSUB + 016u);
    limitsub();
    fltrsub_after_total();
}

/* LIMITSUB's return: back to the caller, through TEM2. */
void fltrsub_after_total(void)
{
    leave_through(ADDRESS_OF(TEM2));
}

/* ---- UNWCTEST -- p. 919 ---- */

/* UNWCTEST, the "SUBR TO TEST THE ANGLE BETWEEN THE PROPOSED WINDOW AND
 * THRUST CMD VCTS": the square of their dot product less DOTSWFMX ("LIM
 * COLNRTY OF UNWC/2 & UNFC/2 TO 85 DEG"); if below, on to DCMCL with the
 * window; else FLAGOODW zeroed and back, "FOR ALT CHOICE IF DOT MAGN TOO
 * LARGE". */
static const word unwctest_words[] = {
    /* UNWCTEST */ ORDERS(DOT, DSQ),
    ADDRESS_PLUS_ONE(UNX_2),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(DOTSWFMX),
    ADDRESS(DCMCL),
    ORDERS(SSP, RVQ),
    ADDRESS_PLUS_ONE(FLAGOODW), /* "ZEROING WINDOW GOOD FLAG" */
    NUMBER(0),
};

const struct interpretive_code UNWCTEST_CODE = {030u, UNWCTEST, COUNT(unwctest_words),
                                                unwctest_words};

/* ---- NB2CDUSP -- p. 920 ---- */

/* NB2CDUSP "RETURNS THE 2'S COMPLEMENT, PI, SP CDU ANGLES X,Y,Z IN
 * MPAC,+1,+2 GIVEN THE MATRIX WHOSE ROW VECTORS ARE THE SEMI-UNIT NAV BASE
 * VECTORS X,Y,X EXPRESSED IN STABLE MEMBER COORDINATES, LOCATED AT 0 IN
 * THE PUSH LIST." The middle angle's sine is the matrix's word 2; its
 * cosine, the square root of 1/4 less the sine squared ("IN CASE SIN WAS
 * SLIGHTLY > 1/2", zero), "YIELDS COS(CDUZ) IN UNITS OF 2"; then the basic
 * code (EXIT). */
static const word nb2cdusp_words[] = {
    /* NB2CDUSP */ ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(BDSU, BPL),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ADDRESS_AT(NB2CDUSP, 7), /* +3 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    ORDERS(SQRT, EXIT),
};

const struct interpretive_code NB2CDUSP_CODE = {030u, NB2CDUSP, COUNT(nb2cdusp_words),
                                                nb2cdusp_words};

static bool arctrgsp(void);
static void dvbycosm(void);

/* The cosine doubled, to units of 1, in TEM5 ("OVERFLOW. FETCH POSMAX,
 * MPAC ALWAYS POS"); CDUZ from its sine (the matrix's word 2) and cosine
 * (ARCTRGSP). ARCTRGSP returns through TEM4, which leaves the file; each
 * return is a key of its own, and only SINZERO's TC Q comes straight
 * back. */
void nb2cdusp_after_exit(void)
{
    dca(MPAC_AT(0u));
    ddoubl();
    if (ts(ADDRESS_OF(TEM5))) {
        set_A(POSMAX);
        ts(ADDRESS_OF(TEM5));
    }
    ca(indexed_operand(CA_CODE | 2u, FIXLOC)); /* INDEX FIXLOC; CA 2 */
    lxch(MPAC_AT(0u));
    tc_at(NB2CDUSP + 022u);
    if (arctrgsp()) nb2cdusp_store_cduz();
}

/* ARCTRGSP's return: "CDUZ". Then CDUY, from the matrix's words 0 and 4
 * divided by the cosine (DVBYCOSM). (An A that overflows skips the CA:
 * DVBYCOSM then takes the word one off FIXLOC.) */
void nb2cdusp_store_cduz(void)
{
    if (!ts(MPAC_AT(2u))) set_A(ZERO);
    tc_at(NB2CDUSP + 025u);
    dvbycosm();
    nb2cdusp_cduy_second();
}

void nb2cdusp_cduy_second(void)
{
    set_A(FOUR);
    tc_at(NB2CDUSP + 027u);
    dvbycosm();
    nb2cdusp_cduy_angle();
}

/* The first quotient negated (TEM1) is the sine, the second (in L) the
 * cosine. */
void nb2cdusp_cduy_angle(void)
{
    cs(ADDRESS_OF(TEM1));
    tc_at(NB2CDUSP + 031u);
    if (arctrgsp()) nb2cdusp_store_cduy();
}

/* ARCTRGSP's return: "CDUY". Then CDUX, from the matrix's words 8 and 14.
 */
void nb2cdusp_store_cduy(void)
{
    if (!ts(MPAC_AT(1u))) set_A(BIT4);
    tc_at(NB2CDUSP + 034u);
    dvbycosm();
    nb2cdusp_cdux_second();
}

void nb2cdusp_cdux_second(void)
{
    set_A(OCT_16);
    tc_at(NB2CDUSP + 036u);
    dvbycosm();
    nb2cdusp_cdux_angle();
}

void nb2cdusp_cdux_angle(void)
{
    cs(ADDRESS_OF(TEM1));
    tc_at(NB2CDUSP + 040u);
    if (arctrgsp()) nb2cdusp_store_cdux();
}

/* ARCTRGSP's return: "CDUX", and back (RVQ). Not modelled: an A that
 * overflows, whose TS would skip the TC INTPRET into the RVQ. */
void nb2cdusp_store_cdux(void)
{
    if (ts(MPAC_AT(0u))) not_modelled();
    tc_at(NB2CDUSP + 042u);
    intpret();
}

static const word nb2cdusp_rvq_words[] = {
    /* NB2CDUSP +043 */ ORDER(RVQ),
};

const struct interpretive_code NB2CDUSP_RVQ_CODE = {030u, NB2CDUSP + 043u,
                                                    COUNT(nb2cdusp_rvq_words), nb2cdusp_rvq_words};

/* ---- DVBYCOSM -- p. 921 ---- */

/* DVBYCOSM: the matrix's word at A (from FIXLOC) divided by the cosine in
 * TEM5, to TEM1. "THE ELEMENTS OF THE NAV BASE MATRIX WHICH WE MUST DIVIDE
 * BY COS(MGA) ALREADY CONTAIN COS(MGA)/2 AS A FACTOR. THEREFORE THE
 * QUOTIENT SHOULD ORDINARILY NEVER EXCEED 1/2 IN MAGNITUDE. BUT IF THE MGA
 * IS NEAR PI/2 THEN COS(MGA) IS NEAR ZERO, AND THERE MAY BE SOME CHAFF IN
 * THE OTHER ELEMENTS OF THE MATRIX WHICH WOULD PRODUCE CHAOS UNDER
 * DIVISION. BEFORE DIVIDING WE MAKE SURE COS(MGA) IS AT LEAST ONE BIT
 * LARGER THAN THE MAGNITUDE OF THE HIGH ORDER PART OF THE OPERAND." Else
 * TEM1 gets the difference, "ALWAYS SMALL IF BRANCH". (A small offset plus
 * FIXLOC never overflows: TS ADDRWD does not skip. A sum that overflows
 * skips the LXCH.) */
static void dvbycosm(void)
{
    ad_at(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(ADDRWD)); /* "ADRES OF OPERAND" */
    ca(indexed_operand(CA_CODE, ADDRWD));
    if (!A_is_zero_or_minus()) com(); /* "FETCH NEG ABS OF OPERAND, AD TEM5" */
    ad_at(ADDRESS_OF(TEM5));
    if (!A_is_zero_or_minus()) {
        dca(indexed_operand(DCA_CODE, ADDRWD) - 1u); /* "DIVISION WILL ALWAYS SUCCEED" */
        dv_at(ADDRESS_OF(TEM5));
    }
    if (!ts(L_ADDRESS)) lxch(ADDRESS_OF(TEM1)); /* TSL&TCQ */
    return_to_q();
}

/* ---- ARCTRGSP -- p. 922 ---- */

static void sinzero(void);
static void sparcsin(void);
static void usecos(void);
static void one_to2_tcq(void);

/* ARCTRGSP "RETURNS THE 2'S COMPLEMENT, PI, SP ANGLE IN THE A REGISTER
 * GIVEN ITS SINE IN A AND ITS COSINE IN L IN UNITS OF 2. THE RESULT IS AN
 * UNAMBIGUOUS ANGLE ANYWHERE IN THE CIRCLE, WITH A MAXIMUM ERROR OF +-4
 * BITS." A zero sine goes to SINZERO, "TO AVOID DIVIDING BY ZERO", which
 * returns by TC Q: true. Else the return waits in TEM4, the sine in TEM2,
 * the cosine in TEM3; the cosine divided by the sine is zero if "COS IS
 * SMALLER" (USECOS). "SIN IS SMALLER OR EQUAL": the angle is ARCSIN(SIN),
 * or with a negative cosine PI less it ("REVERSE SIGN OF SIN"; NEGMAX,
 * "PI, 2'S COMPLEMENT", in TEM3). CCS TEM3 takes four ways: above zero
 * and +0, no turn to add; below zero, the sine negated; -0, the sine
 * replaced by its +0. (A is a word at each TS: none skips. As each test
 * here has it, an overflow would skip the step after.) */
static bool arctrgsp(void)
{
    if (A_is_zero()) {
        sinzero();
        return true;
    }
    qxch(ADDRESS_OF(TEM4));
    if (!ts(ADDRESS_OF(TEM2))) ca(L_ADDRESS);
    if (!ts(ADDRESS_OF(TEM3))) set_A(ZERO);
    dv_at(ADDRESS_OF(TEM2));
    if (A_is_zero()) {
        usecos();
        return false;
    }
    switch (ccs_at(ADDRESS_OF(TEM3))) {
    case POSITIVE: set_A(ZERO); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: cs(ADDRESS_OF(TEM2)); /* fall through */
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(TEM2));
        set_A(NEGMAX);
        break;
    }
    ts(ADDRESS_OF(TEM3)); /* "WE NO LONGER NEED COS" */
    ca(ADDRESS_OF(TEM2));
    tc_at(ARCTRGSP + 024u);
    sparcsin_minus1();
    arctrgsp_after_sparcsin();
    return false;
}

/* SPARCSIN -1's return: the arcsine in two's complement (ONESTO2S), less
 * the turn in TEM3 (MSU), in two's complement again (1TO2&TCQ), and back
 * through TEM4. */
void arctrgsp_after_sparcsin(void)
{
    tc_at(ARCTRGSP + 025u);
    onesto2s();
    arctrgsp_after_onesto2s();
}

void arctrgsp_after_onesto2s(void)
{
    msu_at(ADDRESS_OF(TEM3));
    one_to2_tcq();
}

static void one_to2_tcq(void)
{
    tc_at(ONE_TO2_TCQ);
    onesto2s();
    one_to2_tcq_plus1();
}

void one_to2_tcq_plus1(void)
{
    leave_through(ADDRESS_OF(TEM4));
}

/* USECOS: "ANGLE = SIGN(SIN)(FI/2-ARCSIN(COS))": the arcsine of the
 * cosine negated, plus HALF (a quarter turn at this scale), in TEM3 ("WE
 * NO LONGER NEED COS"). */
static void usecos(void)
{
    cs(ADDRESS_OF(TEM3));
    tc_at(USECOS + 1u);
    sparcsin_minus1();
    usecos_after_sparcsin();
}

/* SPARCSIN -1's return: the angle's sign the sine's (CCS TEM2: above zero
 * as it is, below zero negated, either zero +0); an A that overflows skips
 * the CCS and keeps it as it is. Then 1TO2&TCQ. */
void usecos_after_sparcsin(void)
{
    ad(HALF);
    switch (ts(ADDRESS_OF(TEM3)) ? POSITIVE : ccs_at(ADDRESS_OF(TEM2))) {
    case POSITIVE: ca(ADDRESS_OF(TEM3)); break;
    case NEGATIVE: cs(ADDRESS_OF(TEM3)); break;
    default: break;
    }
    one_to2_tcq();
}

/* SINZERO: a zero sine, so the angle is 0, or PI if the cosine is below
 * zero ("PI, 2'S COMP"); CCS L leaves +0 for the others. */
static void sinzero(void)
{
    if (ccs_register(L_ADDRESS) == NEGATIVE)
        set_A(NEGMAX);
    else
        set_A(ZERO);
    return_to_q();
}

/* ---- SPARCSIN -- p. 923 ---- */

/* "SPARCSIN TAKES AN ARGUMENT SCALED UNITY IN A AND RETURNS AN ANGLE
 * SCALED 180 DEGREES IN A. IT HAS BEEN UNIT TESTED IN THE REGION +-.94
 * (+-70 DEGREES) AND THE MAXIMUM ERROR IS +-5 BITS WITH AN AVERAGE TIME OF
 * 450 MICROSECONDS. SPARCSIN -1 TAKES THE ARGUMENT SCALED TWO. (BOB
 * CRISP)" SPARCSIN -1 doubles it (DOUBLE) and goes on. */
void sparcsin_minus1(void)
{
    double_A();
    sparcsin();
}

/* The argument x to SR (an argument of 1 or more overflows: the limit of
 * its sign, CS LIMITS +1 or -1), its square to TEM1, and
 *     x (DPL1 + x^2 (DPL3 + x^2 (DPL5 + x^2 (DPL7 + x^2 DPL9))))
 * by TC Q. (A square is a word: TS TEM1 does not skip.) */
static void sparcsin(void)
{
    if (ts(ADDRESS_OF(SR))) {
        cs(indexed(ADDRESS_OF(LIMITS), a_word()));
        ts(ADDRESS_OF(SR));
    }
    square();
    ts(ADDRESS_OF(TEM1));
    mp(DPL9);
    ad(DPL7);
    mp_at(ADDRESS_OF(TEM1));
    ad(DPL5);
    mp_at(ADDRESS_OF(TEM1));
    ad(DPL3);
    mp_at(ADDRESS_OF(TEM1));
    ad(DPL1);
    mp_at(ADDRESS_OF(SR));
    return_to_q();
}

/* ---- LIMITSUB and ONESTO2S -- p. 924 ---- */

/* LIMITSUB "LIMITS THE MAGNITUDE OF THE POSITIVE OR NEGATIVE VARIABLE
 * ARRIVING IN L TO THE POSITIVE LIMIT ARRIVING IN A. THE SIGNED LIMITED
 * VARIABLE IS RETURNED IN A." ("VERSION COUTESY HUGH BLAIR-SMITH".) The
 * variable divided by the limit (DV of +0,L) is zero only if it is
 * within the limit: then the variable itself; else the limit, with its
 * sign (LXCH TEM1 puts the limit in L). (A limit that overflows skips the
 * CA ZERO: the divide then takes A's +1 or -1.) */
void limitsub(void)
{
    if (!ts(ADDRESS_OF(TEM1))) set_A(ZERO);
    dv_at(ADDRESS_OF(TEM1));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        lxch(ADDRESS_OF(TEM1));
        ca(L_ADDRESS);
        break;
    case NEGATIVE: cs(ADDRESS_OF(TEM1)); break;
    default: ca(L_ADDRESS); break;
    }
    return_to_q();
}

/* ONESTO2S, the "SUBROUTINE TO CONVERT 1'S COMP SP TO 2'S COMP": a
 * negative number one more (CCS leaves its magnitude less one, CS A its
 * negative), -0 to +0, a positive one as it is (AD ONE puts back the one
 * CCS took off). */
void onesto2s(void)
{
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: ad(ONE); break;
    case NEGATIVE: cs(A_ADDRESS); break;
    default: break;
    }
    return_to_q();
}

#endif
