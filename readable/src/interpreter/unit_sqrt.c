/* SPDX-License-Identifier: MIT */
/* UNIT, the miscellaneous unary orders, the squaring subroutines and the
 * square root: INTERPRETER.agc LM pp. 1070-1081, CM pp. 1175-1186 (in bank
 * 0). The faithful C is c64-port/src/lm_interp_core.c (lmi_unit, unitdv,
 * lmi_dsq, lmi_abvalabs, sqrt_body, lmi_abs, lmi_vdef, lmi_vsq, lmi_push,
 * lmi_rvq, dsqsub, vsqsub, sqrtsub). */
#include "interpreter.h"
#include "../agc/names.h"

static void unit_plus22(void);
static void small(void);
static void small2(void);
static void large(void);
static void large3(void);
static void large2(void);
static void unitdv(void);
static void abval(void);
static void abval_plus1(void);
static void sqrtshft(void);
static void tpush(void);
static bool sqrtneg(void);
static void zeroans(void);
static void fixroot(void);
static bool sqrtabrt(void);
static void smpac_plus(void);
static void arghi(void);
static void arghi_plus4(void);
static void srtest(void);
static void arglo(void);
static void sqrtnm2(void);
static void sqrtnorm(void);

/* ======================================================================
 * UNIT
 * ====================================================================== */

/* UNIT -- LM pp. 1070-1073, CM pp. 1175-1178.
 * "THE FOLLOWING ROUTINE EXECUTES THE UNIT INSTRUCTION, WHICH TAKES THE
 * UNIT OF THE VECTOR IN MPAC." The square of the length goes to LVSQUARE
 * and the length to LV in the work area; each component is divided by the
 * length (UNITDV), with the vector and its length normalized alike. */
void unit(void)
{
    tc_at(UNIT);
    vecagree(); /* "FORCE SIGN AGREEMENT IN VECTOR" */
    unit_plus1();
}

void unit_plus1(void)
{
    tc_at(UNIT + 1u);
    mpacvbuf(); /* "SAVE ARGUMENT IN VBUF" */

    set_A(ZERO); /* "MUST SENSE OVERFLOW IN FOLLOWING DOT." */
    xch(ADDRESS_OF(OVFIND));
    ts(ADDRESS_OF(TEM1));
    tc_at(UNIT + 5u);
    vsqsub(); /* "DOT MPAC WITH ITSELF." */
    ca(ADDRESS_OF(TEM1));
    xch(ADDRESS_OF(OVFIND));
    if (!A_is_zero()) {
        dvovf();
        return;
    }

    dca(MPAC_AT(0u));                            /* "LEAVE THE SQUARE OF THE LENGTH OF THE */
    dxch(indexed(ADDRESS_OF(LVSQUARE), FIXLOC)); /* ARGUMENT IN LVSQUARE." */

    tc_at(UNIT + 15u);
    if (sqrtsub()) return; /* "GO TAKE THE NORMALIZED SQUARE ROOT." */
    unit_plus16();
}

/* UNIT +16: "CHECK FOR UNIT OVERFLOW." The root is normalized: not less
 * than .5 "UNLESS INPUT TO SQRTSUB WAS 0". (The -0 slot runs the DXCH
 * without its INDEX.) */
void unit_plus16(void)
{
    switch (ccs_at(MPAC_AT(0u))) {
    case POSITIVE: unit_plus22(); return;
    case IS_PLUS_ZERO:
        ts(L_ADDRESS);
        /* fall through */
    case NEGATIVE: dxch(indexed(ADDRESS_OF(LV), FIXLOC)); break;
    case IS_MINUS_ZERO: dxch(ADDRESS_OF(LV)); break;
    }
    dvovf();
}

/* UNIT +22: "SEE IF THE INPUT WAS SO SMALL THAT THE FIRST TWO REGISTERS OF
 * THE SQUARE WERE 0": SQRTSUB's right shift count MPTEMP against 14. */
static void unit_plus22(void)
{
    set_A(negate(FOURTEEN));
    ad_at(ADDRESS_OF(MPTEMP));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        com(); /* "IF SO, SAVE THE NEGATIVE OF THE SHIFT COUNT -15D." */
        /* fall through */
    case IS_PLUS_ZERO: small(); return;
    case NEGATIVE: large(); return; /* "(THIS IS USUALLY THE CASE.)" */
    case IS_MINUS_ZERO: break;
    }
    set_A(negate(THIRTEEN)); /* "IF THE SHIFT COUNT WAS EXACTLY 14, SET THE */
    ts(ADDRESS_OF(MPTEMP));  /* PRE-DIVIDE NORM COUNT TO -13D." */
    ca(MPAC_AT(0u));         /* "SHIFT THE LENGTH RIGHT 14 BEFORE STORING" */
    small2();
}

/* SMALL2: "(SMALL EXITS TO THIS POINT)." */
static void small2(void)
{
    ts(L_ADDRESS);
    set_A(ZERO);
    large2(); /* "GO TO STORE LENGTH AND PROCEED." */
}

/* LARGE: "MOST ALL CASES COME HERE." "SEE IF NO NORMALIZATION WAS REQUIRED
 * BY SQRT, AND IF SO, SET UP FOR A SHIFT RIGHT 1 BEFORE DIVIDING TO PRODUCE
 * THE DESIRED HALF UNIT VECTOR" (MPTEMP -20, for MAXTEST -21D). */
static void large(void)
{
    switch (ccs_at(ADDRESS_OF(MPTEMP))) {
    case POSITIVE: large3(); return;
    case IS_PLUS_ZERO:
        set_A(negate(SRDDV));
        /* fall through */
    case NEGATIVE: ts(ADDRESS_OF(MPTEMP)); break;
    case IS_MINUS_ZERO: break;
    }
    dca(MPAC_AT(0u));
    large2();
}

/* LARGE3 -- LM p. 1072, CM p. 1177. "LEAVE NEGATIVE OF SHIFT COUNT-1 FOR
 * PREDIVIDE LEFT SHIFT." "PICK UP REQUIRED SHIFTING BIT TO UNNORMALIZE THE
 * SQRT RESULT" "(UNNORMALIZE THE SQRT FOR LV)." */
static void large3(void)
{
    com();
    ts(ADDRESS_OF(MPTEMP));
    com();
    ca(indexed(ADDRESS_OF(BIT14), a_word()));
    ts(BUF_AT(0u));
    mp_at(MPAC_AT(1u));
    xch(BUF_AT(0u));
    mp_at(MPAC_AT(0u));
    xch(L_ADDRESS);
    ad_at(BUF_AT(0u));
    xch(L_ADDRESS);
    large2();
}

/* LARGE2: "LENGTH NOW STORED IN WORK AREA." "NO MAXDV CASES IN UNIT." Each
 * component is divided by the length, "IN USUAL FASHION SO WE CAN EXIT
 * THROUGH VROTATEX." */
static void large2(void)
{
    dxch(indexed(ADDRESS_OF(LV), FIXLOC));

    set_A(negate(ONE));
    ts(ADDRESS_OF(MAXDVSW));

    dxch(VBUF_AT(0u)); /* "PREPARE X COMPONENT FOR DIVIDE, SETTING */
    dxch(MPAC_AT(0u)); /* LENGTH OF VECTOR AS DIVISOR IN BUF." */
    dxch(BUF_AT(0u));
    tc_at(LARGE2 + 7u);
    unitdv();

    dxch(VBUF_AT(2u));
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(3u));
    tc_at(LARGE2 + 11u);
    unitdv();

    dxch(VBUF_AT(4u));
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(5u));
    tc_at(LARGE2 + 15u);
    unitdv();
    vrotatex(); /* "AND EXIT." */
}

/* SMALL -- LM p. 1073, CM p. 1178. "IF THE LENGTH OF THE ARGUMENT VECTOR
 * WAS LESS THAN 2(-28), EACH COMPONENT MUST BE SHIFTED LEFT AT LEAST 14
 * PLACES BEFORE THE DIVIDE. NOTE THAT IN THIS CASE, THE MAJOR PART OF EACH
 * COMPONENT IS ZERO." */
static void small(void)
{
    ts(ADDRESS_OF(MPTEMP)); /* "NEGATIVE OF PRE-DIVIDE SHIFT COUNT." */

    set_A(ZERO); /* "SHIFT EACH COMPONENT LEFT 14." */
    xch(VBUF_AT(1u));
    xch(VBUF_AT(0u));
    xch(VBUF_AT(3u));
    xch(VBUF_AT(2u));
    xch(VBUF_AT(5u));
    xch(VBUF_AT(4u));

    cs(ADDRESS_OF(MPTEMP));
    ca(indexed(ADDRESS_OF(BIT14), a_word()));
    mp_at(MPAC_AT(0u));
    small2();
}

/* UNITDV -- LM p. 1074, CM p. 1179. "THE FOLLOWING ROUTINE SETS UP THE
 * CALL TO THE DIVIDE ROUTINES." "FORCE MPAC POSITIVE IF POSSIBLE, SETTING
 * DVSIGN ACCORDING TO THE SIGN OF MPAC SINCE THE DIVISOR IS ALWAYS
 * POSITIVE HERE." "EXIT IMMEDIATELY ON ZERO." The divide starts at the
 * prologue MPTEMP selects (INDEX MPTEMP; TCF MAXTEST -1) and returns to
 * UNITDV's caller. */
static void unitdv(void)
{
    enum sign major = ccs_at(MPAC_AT(0u));

    if (major != POSITIVE && major != NEGATIVE) {
        major = ccs_at(MPAC_AT(1u));
        if (major != POSITIVE && major != NEGATIVE) {
            tc_through(Q_ADDRESS);
            return;
        }
    }
    if (major == NEGATIVE) {    /* UMPAC- */
        set_A(negate(ZERO));    /* "IF NEGATIVE, SET -0 IN DVSIGN FOR FINAL */
        ts(ADDRESS_OF(DVSIGN)); /* COMPLEMENT." */
        dcs(MPAC_AT(0u));       /* "PICK UP ABSOLUTE VALUE OF ARG AND JUMP." */
    } else {                    /* UMPAC+ */
        ts(ADDRESS_OF(DVSIGN)); /* "SET DVSIGN FOR POSITIVE QUOTIENT." */
        dxch(MPAC_AT(0u));
    }
    into_maxtest(MAXTEST - 1u, ADDRESS_OF(MPTEMP));
}

/* ======================================================================
 * Miscellaneous unary orders
 * ====================================================================== */

/* DSQ -- LM p. 1075, CM p. 1180. "SQUARE THE DP CONTENTS OF MPAC." */
void dsq(void)
{
    tc_at(DSQ);
    dsqsub();
    continue_at(DANZIG);
}

/* ABVALABS -- LM p. 1075, CM p. 1180. "ABVAL OR ABS INSTRUCTION": ABS on a
 * scalar, ABVAL on a vector, which leaves the square of the length in
 * LVSQUARE and runs on into SQRT. (A MODE of -0 would skip ABVAL's TC.) */
void abvalabs(void)
{
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: abs_(); return; /* "DO ABS ON SCALAR." */
    case NEGATIVE: abval(); return;
    case IS_MINUS_ZERO: abval_plus1(); return;
    }
}

static void abval(void)
{
    tc_at(ABVAL);
    vsqsub(); /* "DOT MPAC WITH ITSELF." */
    abval_plus1();
}

static void abval_plus1(void)
{
    lxch(ADDRESS_OF(MODE)); /* "MODE IS NOW DP (L ZERO AFTER DAS)." */
    dca(MPAC_AT(0u));       /* "STORE SQUARE OF LENGTH IN WORK AREA." */
    dxch(indexed(ADDRESS_OF(LVSQUARE), FIXLOC));
    sqrt_();
}

/* SQRT -- LM p. 1076, CM p. 1181. "DOUBLE PRECISION SQUARE ROOT ROUTINE":
 * "THIS PROGRAM TAKES THE SQUARE ROOT OF THE 27 OR 28 MOST SIGNIFICANT
 * BITS IN THE TRIPLE PRECISION SET OF NUMBERS -- MPAC, MPAC+1, AND MPAC+2.
 * THE ROOT IS RETURNED DOUBLE PRECISION IN MPAC AND MPAC+1." "ABORTS ON
 * NEGATIVE INPUT -1.2X10E-4 (77775 OCTAL) OR LESS. DISPLAYS ERROR CODE
 * 1302". SQRTSUB returns the root normalized, with the right shift that
 * undoes the normalization in MPTEMP: "SEE IF ANY UN-NORMALIZATION
 * REQUIRED AND EXIT IF NOT." */
void sqrt_(void)
{
    tc_at(SQRT);
    if (sqrtsub()) return;

    switch (ccs_at(ADDRESS_OF(MPTEMP))) {
    case IS_PLUS_ZERO: continue_at(DANZIG); return;
    case IS_MINUS_ZERO: sqrtshft(); return; /* (A +0 passes the BZMF) */
    default: break;
    }
    ad(NEG12); /* "A RIGHT SHIFT OF MORE THAN 13 COULD BE REQUIRED IF INPUT
                * WAS ZERO IN MPAC,+1." */
    if (A_is_zero_or_minus()) {
        sqrtshft(); /* "GOES HERE IN MOST CASES." */
        return;
    }
    zl();                     /* "IF A LONG SHIFT IS REQUIRED, GO TO GENERAL RIGHT SHIFT */
    lxch(ADDRESS_OF(ADDRWD)); /* ROUTINES." */
    genscr_plus4();           /* "ADDRWD WAS ZERO TO PREVENT ROUND." */
}

/* SQRTSHFT: "SELECT SHIFTING BIT AND EXIT THROUGH SHIFT ROUTINES." */
static void sqrtshft(void)
{
    ca(indexed(ADDRESS_OF(BIT15), MPTEMP));
    ts(ADDRESS_OF(MPTEMP));
    set_A(ZERO); /* "TO ZERO MPAC +2 IN THE PROCESS." */
    mpacshr_plus3();
}

/* ABS -- LM p. 1076, CM p. 1181. "TEST SIGN OF MPAC AND COMPLEMENT IF"
 * negative. */
void abs_(void)
{
    tc_at(ABS);
    if (branch() == BRANCH_NEGATIVE)
        comp();
    else
        continue_at(DANZIG);
}

/* VDEF -- LM p. 1077, CM p. 1182. "VECTOR DEFINE -- ESSENTIALLY TREATS
 * SCALAR IN MPAC AS X COMPONENT, PUSHES UP FOR Y AND THEN AGAIN FOR Z." */
void vdef(void)
{
    set_A(negate(FOUR));
    ads(ADDRESS_OF(PUSHLOC));
    dca(indexed(2u, a_word()));
    dxch(MPAC_AT(3u));
    dca(indexed(0u, PUSHLOC));
    dxch(MPAC_AT(5u));
    vmode(); /* "MODE IS NON VECTOR." */
}

/* VSQ -- LM p. 1077, CM p. 1182. "DOT MPAC WITH ITSELF." "MODE IS NOW DP." */
void vsq(void)
{
    tc_at(VSQ);
    vsqsub();
    dmode();
}

/* PUSH -- LM p. 1077, CM p. 1182. "PUSH DOWN MPAC LEAVING IT LOADED."
 * "PUSH DOWN FIRST TWO REGISTERS IN EACH" case, "INCREMENT PUSHDOWN
 * POINTER" by the mode's size (NO.WDS), then the rest: MPAC +2 for triple
 * precision, the Y and Z components for a vector. (A MODE of -0 would skip
 * the vector's EXTEND: its DCA MPAC +3 would run as CA MPAC +4.) */
void push(void)
{
    dca(MPAC_AT(0u));
    dxch(indexed(0u, PUSHLOC));

    ca(indexed(ADDRESS_OF(NO_WDS), MODE));
    ads(ADDRESS_OF(PUSHLOC));

    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE: tpush(); return;                 /* "PUSH DOWN MPAC +2." */
    case IS_PLUS_ZERO: continue_at(DANZIG); return; /* "DONE FOR DP." */
    case NEGATIVE: dca(MPAC_AT(3u)); break;         /* "ON VECTOR, PUSH DOWN Y AND Z */
    case IS_MINUS_ZERO: ca(MPAC_AT(4u)); break;     /* COMPONENTS." */
    }
    dxch(PUSHED(4u));
    dca(MPAC_AT(5u));
    dxch(PUSHED(2u));
    continue_at(DANZIG);
}

static void tpush(void)
{
    ca(MPAC_AT(2u));
    endtpush_plus2();
}

/* RVQ -- LM p. 1077, CM p. 1182. "RETURN VIA QPRET." "(ASSUME QPRET POINTS
 * TO FIXED ONLY.)" */
void rvq(void)
{
    ca(indexed(ADDRESS_OF(QPRET), FIXLOC));
    ts(ADDRESS_OF(POLISH));
    goto_plus4();
}

/* ======================================================================
 * Squaring
 * ====================================================================== */

/* DSQSUB -- LM p. 1078, CM p. 1183. "THE FOLLOWING SUBROUTINES ARE USED IN
 * SQUARING MPAC, IN BOTH THE SCALAR AND VECTOR SENSE. THEY ARE SPECIAL
 * CASES OF DMPSUB AND DOTSUB, PUT IN TO SAVE SOME TIME." DSQSUB "SQUARES
 * THE SCALAR CONTENTS OF MPAC". */
void dsqsub(void)
{
    ca(MPAC_AT(1u));
    square();
    ts(MPAC_AT(2u));
    set_A(ZERO); /* "FORM 2(CROSS TERM)." */
    xch(MPAC_AT(1u));
    mp_at(MPAC_AT(0u));
    ddoubl();         /* "AND MAYBE OVEFLOW." */
    das(MPAC_AT(1u)); /* "AND SET A TO NET OVERFLOW." */
    xch(MPAC_AT(0u));
    square();
    das(MPAC_AT(0u));
    tc_through(Q_ADDRESS);
}

/* VSQSUB: "DOTS THE VECTOR IN MPAC WITH ITSELF", ending "AS IN DOTSUB". */
void vsqsub(void)
{
    qxch(ADDRESS_OF(DOTRET));
    tc_at(VSQSUB + 2u);
    dsqsub(); /* "SQUARE THE X COMPONENT." */
    dxch(MPAC_AT(3u));
    dxch(MPAC_AT(0u));
    dxch(BUF_AT(0u)); /* "SO WE CAN END IN DOTSUB." */
    ca(MPAC_AT(2u));
    ts(BUF_AT(2u));

    tc_at(VSQSUB + 8u);
    dsqsub(); /* "SQUARE Y COMPONENT." */
    dxch(MPAC_AT(1u));
    das(BUF_AT(1u));
    ad_at(MPAC_AT(0u));
    ad_at(BUF_AT(0u));
    if (ts(BUF_AT(0u))) ts(ADDRESS_OF(OVFIND)); /* "IF OVERFLOW." */

    dxch(MPAC_AT(5u));
    dxch(MPAC_AT(0u));
    tc_at(VSQSUB + 18u);
    dsqsub(); /* "SQUARE Z COMPONENT." */
    enddot(); /* "END AS IN DOTSUB." */
}

/* ======================================================================
 * The square root
 * ====================================================================== */

/* SQRTSUB -- LM pp. 1079-1081, CM pp. 1184-1186.
 * "TAKE THE SQUARE ROOT OF THE TRIPLE PRECISION (MPAC +2 USED ONLY IN
 * NORMALIZATION) CONTENTS OF MPAC AND LEAVE THE NORMALIZED RESULT IN MPAC
 * (C(MPAC) GREATER THAN OR EQUAL TO .5). THE RIGHT SHIFT COUNT (TC
 * UNNORMALIZE) IS LEFT IN MPTEMP."
 *
 * Normalize the argument to between .125 and .5 (counting the shifts), take
 * a straight-line first guess for that range (SLOPEHI and BIASHI, SLOPELO
 * and BIASLO), and improve it by two Newton steps, the second in double
 * precision. Its result says whether it aborted (alarm 1302) instead of
 * returning. */
bool sqrtsub(void)
{
    set_A(ZERO); /* "START BY ZEROING RIGHT SHIFT COUNT." */
    ts(ADDRESS_OF(MPTEMP));

    /* "CHECK FOR POSITIVE ARGUMENT, SHIFTING FIRST SIGNIFICANT MPAC
     * REGISTER INTO MPAC ITSELF." */
    switch (ccs_at(MPAC_AT(0u))) {
    case POSITIVE: smpac_plus(); return false;
    case NEGATIVE:
        return sqrtneg(); /* "SEE IF MAG OF ARGUMENT LESS THAN
                                      * 10(-4)." */
    default: break;
    }

    xch(MPAC_AT(2u)); /* "MPAC IS ZERO -- SHIFT LEFT 14." */
    xch(MPAC_AT(1u));
    ts(MPAC_AT(0u));
    set_A(SEVEN); /* "AUGMENT RIGHT SHIFT COUNTER." */
    ts(ADDRESS_OF(MPTEMP));

    switch (ccs_at(MPAC_AT(0u))) { /* "SEE IF MPAC NOW PNZ." */
    case POSITIVE: smpac_plus(); return false;
    case NEGATIVE:
        zeroans();
        return false; /* "NEGATIVE BUT LESS THAN 10(-4)
                                             * IN MAG." */
    default: break;
    }

    xch(MPAC_AT(1u)); /* "XERO -- SHIFT LEFT 14 AGAIN." */
    ts(MPAC_AT(0u));
    set_A(SEVEN); /* "AUGMENT RIGHT SHIFT COUNTER." */
    ads(ADDRESS_OF(MPTEMP));

    switch (ccs_at(MPAC_AT(0u))) {
    case POSITIVE: smpac_plus(); break;
    case IS_PLUS_ZERO: tc_through(Q_ADDRESS); break; /* "SQRT(0) = 0." */
    case NEGATIVE: zeroans(); break;
    case IS_MINUS_ZERO:
        fixroot();
        break; /* "DO NOT LEAVE SQRTSUB WITH -0 IN
                                           * MPAC." */
    }
    return false;
}

/* SQRTNEG: "ARGUMENT IS NEGATIVE, BUT SEE IF SIGN-CORRECTED ARGUMENT IS LESS
 * THAN 10(-4) IN MAGNITUDE. IF SO, CALL ANSWER ZERO." A is MPAC's magnitude
 * less one. */
static bool sqrtneg(void)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: return sqrtabrt();
    case NEGATIVE: zeroans(); return false;
    case IS_MINUS_ZERO: fixroot(); return false;
    case IS_PLUS_ZERO: break;
    }
    switch (ccs_at(MPAC_AT(1u))) {
    case POSITIVE: zeroans(); return false;
    case NEGATIVE: return sqrtabrt();
    default: fixroot(); return false;
    }
}

/* ZEROANS: "FORCE ANSWER TO ZERO HERE." */
static void zeroans(void)
{
    set_A(ZERO);
    fixroot();
}

/* SQRTABRT: the alarm 1302 ("DISPLAYS ERROR CODE 1302"): the LM's
 * POODOO1 takes the interpretive return from LOC in A,L (DXCH LOC); the
 * CM's POODOO the plain one. The OCT 1302 after the TC is the code. */
static bool sqrtabrt(void)
{
#if LUMINARY099
    dxch(ADDRESS_OF(LOC));
    tc_at(SQRTABRT + 1u);
    continue_at(POODOO1);
#else
    tc_at(SQRTABRT);
    continue_at(POODOO);
#endif
    return true;
}

/* SMPAC+ -- LM p. 1080, CM p. 1185. "SEE IF ARGUMENT GREATER THAN OR EQUAL
 * TO .5." (A is MPAC less one, plus -1/2+2: MPAC - .5.) "WE WILL TAKE THE
 * SQUARE ROOT OF MPAC/2. SHIFT RIGHT 1 AND GO TO THE SQRT ROUTINE." */
static void smpac_plus(void)
{
    ad(MINUS_1_2_PLUS_2);
    if (A_is_zero_or_minus()) {
        srtest(); /* "IF SO, SEE IF LESS THAN .25." */
        return;
    }
    dxch(MPAC_AT(0u));
    lxch(SR_ADDRESS);
    mp(HALF);
    dxch(MPAC_AT(0u));
    xch(SR_ADDRESS);
    ads(MPAC_AT(1u)); /* "GUARANTEED NO OVERFLOW." */
    arghi();
}

/* ARGHI: "ARGUMENT BETWEEN .25 AND .5, GET A LINEAR APPROXIMATION FOR
 * THIS RANGE": "X0/2 = (MPAC/2)(SLOPHI) + BIASHI/2." */
static void arghi(void)
{
    set_A(SLOPEHI);
    mp_at(MPAC_AT(0u));
    ad(BIASHI);
    arghi_plus4();
}

/* ARGHI +4: "X0/2 (ARGLO ENTERS HERE)." Two Newton steps: "X1 = X0/2 +
 * .5(MPAX/2)/(X0/2)" in single precision, then "X2 = X1/2 + (MPAC/2)X1" in
 * double, which "OVERFLOWS IF ARG. NEAR POSMAX". */
static void arghi_plus4(void)
{
    if (!ts(BUF_AT(0u))) ca(MPAC_AT(0u)); /* "SINGLE-PRECISION THROUGHOUT." */
    zl();
    dv_at(BUF_AT(0u)); /* "(MPAC/2)/(X0/2)" */
    mp(HALF);
    ads(BUF_AT(0u));

    mp(HALF);                         /* "FORM UP X1/2." */
    dxch(MPAC_AT(0u));                /* "SAVE AND BRING OUT ARGUMENT." */
    dv_at(BUF_AT(0u));                /* "TAKE DP QUOTIENT WITH X1." */
    if (!ts(BUF_AT(1u))) set_A(ZERO); /* "SAVE MAJOR PART OF QUOTIENT." */
    xch(L_ADDRESS);                   /* "FORM MINOR PART OF QUOTIENT USING (REMAINDER,0)." */
    dv_at(BUF_AT(0u));
    if (!ts(L_ADDRESS)) ca(BUF_AT(1u)); /* "IN PREPARATION FOR DAS." */
    das(MPAC_AT(0u));

    if (A_is_zero()) {
        tc_through(Q_ADDRESS); /* TCQBNK00 */
        return;
    }
    set_A(POSMAX);
    fixroot();
}

/* FIXROOT, TCQBNK00: "RETURN TO CALLER TO UNNORMALIZE, ETC." */
static void fixroot(void)
{
    if (!ts(MPAC_AT(0u))) ts(MPAC_AT(1u));
    tc_through(Q_ADDRESS);
}

/* SRTEST -- LM p. 1081, CM p. 1186. "ARGUMENT WAS LESS THAN .5, SEE IF LESS
 * THAN .25." "IF BETWEEN .5 AND .25, SHIFT RIGHT 1 AND START AT ARGLO." */
static void srtest(void)
{
    ad(QUARTER);
    if (A_is_zero_or_minus()) {
        sqrtnorm(); /* "IF SO, BEGIN NORMALIZATION." */
        return;
    }
    dxch(MPAC_AT(0u));
    lxch(SR_ADDRESS);
    mp(HALF);
    dxch(MPAC_AT(0u));
    xch(SR_ADDRESS);
    ads(MPAC_AT(1u)); /* "NO OVERFLOW." */
    arglo();
}

/* ARGLO: "(NORMALIZED) ARGUMENT BETWEEN .125 AND .25". */
static void arglo(void)
{
    set_A(SLOPELO);
    mp_at(MPAC_AT(0u));
    ad(BIASLO);
    arghi_plus4(); /* "BEGIN SQUARE ROOT." */
}

/* SQRTNM2: "SHIFT LEFT 2 AND INCREMENT RIGHT SHIFT COUNT (FOR TERMINAL
 * UNNORMALIZATION)." */
static void sqrtnm2(void)
{
    dca(MPAC_AT(1u));
    das(MPAC_AT(1u));
    ad_at(MPAC_AT(0u));
    ads(MPAC_AT(0u)); /* "(NO OVERFLOW)." */
    sqrtnorm();
}

/* SQRTNORM: "FIRST TIME THROUGH, JUST SHIFT LEFT 1 (PUTS IN EFFECTIVE RIGHT
 * SHIFT SINCE WE WANT MPAC/2)." NORMTEST: "SEE IF ARGUMENT NOW NORMALIZED
 * AT GREATER THAN .125": the doubled major part goes into CYL, and each
 * CCS CYL (which cycles it left again) tests the next bit down. */
static void sqrtnorm(void)
{
    enum sign test;

    incr(ADDRESS_OF(MPTEMP));
    dca(MPAC_AT(1u));
    das(MPAC_AT(1u));
    ad_at(MPAC_AT(0u));
    ads(MPAC_AT(0u)); /* "(AGAIN NO OVERFLOW)." */
    double_A();

    /* TS CYL; NORMTEST CCS CYL; CCS CYL. An overflowing TS skips the first
     * CCS. */
    test = ts(CYL_ADDRESS) ? POSITIVE : ccs_at(CYL_ADDRESS);
    switch (test) {
    case POSITIVE: break;
    case IS_PLUS_ZERO: sqrtnm2(); return;
    case NEGATIVE: arghi(); return;
    case IS_MINUS_ZERO: arglo(); return;
    }
    switch (ccs_at(CYL_ADDRESS)) {
    case POSITIVE: sqrtnm2(); return;     /* "NO -- SHIFT LEFT 2 MORE AND TRY AGAIN." */
    case IS_PLUS_ZERO: arghi(); return;   /* "YES -- NOW BETWEEN .5 AND .25." */
    case NEGATIVE: arglo(); return;       /* "ARGUMENT NOW BETWEEN .25 AND .125." */
    case IS_MINUS_ZERO: cosine(); return; /* (never: runs on into COSINE) */
    }
}
