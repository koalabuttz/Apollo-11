/* SPDX-License-Identifier: MIT */
/* TRIM_GIMBAL_CONTROL_SYSTEM.agc, the LM's: Luminary099 pages 1472-1484,
 * bank 21 in EBANK 6 (the listing's EBANK= QDIFF); ACDT+C12 in bank 16. The
 * faithful C is c64-port/src/lm_trim_gimbal.c. */
#include "trim_gimbal.h"

#if LUMINARY099

/* The CA order's operation code. */
#define CA_CODE 3u

/* RETURN: TC Q. */
static void tc_q(void)
{
    tc_through(Q_ADDRESS);
}

/* INDEX QRCNTR: the Q axis's word (QRCNTR 0), or the R axis's two on. */
static unsigned by_axis(unsigned address)
{
    return indexed(address, fetch(ADDRESS_OF(QRCNTR)));
}

/* TS L; TCF +2; ADS high: a low word's carry into the high word. */
static void carry_into(unsigned high)
{
    if (ts(L_ADDRESS)) ads(high);
}

/* The sign of a double word, +1, -1 or +0, into A: CCS of the high word
 * ("TEST SIGN OF HIGH ORDER PART."), and when that is zero, CCS of the low
 * word ("SIGN TEST FOR LOW ORDER PART."), with CAF BIT1 and CS BIT1 at its
 * exits. A double zero leaves A the +0 the CCS gives. */
static void sign_of_double(unsigned high)
{
    switch (ccs_at(high)) {
    case POSITIVE: set_A(BIT1); return;
    case NEGATIVE: set_A(negate(BIT1)); return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(high + 1u)) {
    case POSITIVE: set_A(BIT1); return;
    case NEGATIVE: set_A(negate(BIT1)); return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: return; /* TCF +2 */
    }
}

/* ---- ROOTCYCL -- LM p. 1484 ---- */

/* ROOTCYCL, by TC at at: "ONE NEWTON SQUARE ROOT ALGORITHM ITERATION."
 * "THE INITIAL GUESS AT THE SQUARE ROOT IS PRESUMED TO BE IN THE A
 * REGISTER AND ONE-HALF THE SQUARE IS TAKEN FROM HALFARG." TS SR halves
 * X: "X/2 NOW IN SR"; a TS SCRATCH that overflows would skip it. */
static void rootcycl(word at)
{
    tc_at(at);
    if (!ts(ADDRESS_OF(SCRATCH))) ts(SR_ADDRESS); /* "STORE X" */
    ca(ADDRESS_OF(HALFARG));
    zl();
    dv_at(ADDRESS_OF(SCRATCH)); /* "(ARG/X)/2" */
    ad_at(SR_ADDRESS);          /* "(X + ARG/X)/2 IN THE A REG" */
    tc_q();
}

/* ---- GTSQRT -- LM pp. 1481-1482 ---- */

/* MULBUSH and SCALLOOP from GOODARG +1, A the shift (0, or 7 from
 * ZEROHIGH) for SHFTFLAG: ININDEX counts 12, 10, ... down while the
 * argument is not less than 2 to the power -ININDEX ("IF ARG IS NOT LESS
 * THAN REFERENCE, GO AROUND THE MULBERRY BUSH ONCE MORE."). Then the
 * argument is divided by that power ("RESCALE ARGUMENT."), unless ININDEX
 * passed zero: "IF ARG IS NOT LESS THAN 1/4, INDEX IS ZERO, INDICATING NO
 * SHIFT NEEDED." (SCALSTRT). */
static void scale(void)
{
    unsigned index = ADDRESS_OF(ININDEX);

    ts(ADDRESS_OF(SHFTFLAG));
    set_A(TWELVE); /* "INITIALIZE THE SCALING LOOP." */
    ts(index);
    for (;;) {
        cs(ADDRESS_OF(FUNCTION)); /* SCALLOOP */
        ad_at(indexed(BIT15_ADDRESS, fetch(index)));
        if (!A_is_zero_or_minus()) break;
        set_A(NEG2); /* MULBUSH */
        ads(index);
        if (A_is_zero_or_minus()) {
            ca(ADDRESS_OF(FUNCTION)); /* SCALSTRT */
            return;
        }
    }
    ca(indexed(BIT15_ADDRESS, fetch(index))); /* "THIS IS THE SCALE MAGNITUDE" */
    xch(ADDRESS_OF(HALFARG));
    dca(ADDRESS_OF(FUNCTION));
    dv_at(ADDRESS_OF(HALFARG));
}

/* GTSQRT, by TC GTSQRT from RSTOFGTS: "A SINGLE-PRECISION SQUARE ROOT OF
 * THE FOURTEEN MOST SIGNIFICANT BITS OF THE ARGUMENT" in FUNCTION,
 * FUNCTION +1, and in SHFTFLAG the exponent S: "THE SQUARE ROOT (RETURNED
 * IN THE A REGISTER) MUST BE SHIFTED RIGHT" by S. "ZERO OR NEGATIVE
 * ARGUMENTS YIELD ZERO FOR SQUARE ROOTS." (ZEROOT, which returns by TC
 * Q). A zero high word moves the low one up (ZEROHIGH: "14 MOST
 * SIGNIFICANT BITS ARE IN THE LOWER WORD.", seven more to shift). Then
 * SCALDONE: Q saved in FUNCTION +1, half the scaled argument in HALFARG,
 * "INITIAL GUESS IS ROOT 1/2 OR POSMAX" (by bit 13 of HALFARG), three
 * Newton steps, and TC FUNCTION +1: the saved return, PRODUCT. */
static void gtsqrt(void)
{
    bool zero_root = false;

    switch (ccs_at(ADDRESS_OF(FUNCTION))) {
    case POSITIVE: set_A(ZERO); break; /* GOODARG */
    case NEGATIVE: zero_root = true; break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO:
        ca(ADDRESS_OF(FUNCTION) + 1u);
        if (A_is_zero_or_minus()) {
            zero_root = true;
        } else {
            xch(ADDRESS_OF(FUNCTION)); /* ZEROHIGH */
            xch(ADDRESS_OF(FUNCTION) + 1u);
            set_A(SEVEN);
        }
        break;
    }
    if (zero_root) {
        set_A(ZERO); /* ZEROOT */
        ts(ADDRESS_OF(SHFTFLAG));
        tc_q();
        return;
    }
    scale();
    qxch(ADDRESS_OF(FUNCTION) + 1u); /* SCALDONE: "SAVE Q FOR RETURN" */
    mp(BIT14);
    ts(ADDRESS_OF(HALFARG));
    mask(BIT13);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: set_A(OCT11276); break;
    case IS_PLUS_ZERO: break;
    default: not_modelled(); return; /* a MASK of BIT13 is never negative */
    }
    ad(ROOTHALF);
    rootcycl(SCALDONE + 011u);
    rootcycl(SCALDONE + 012u);
    rootcycl(SCALDONE + 013u);
    tc_through(ADDRESS_OF(FUNCTION) + 1u);
}

/* ---- GTS -- LM pp. 1472-1476, 1482-1484 ---- */

/* GTSQAXIS -3 to ALGORTHM: the axis's offset acceleration ALPHA and rate
 * OMEGA into ACENTRAL and WCENTRAL, and its control authority K into
 * KCENTRAL. "AOS SCALED AT PI/2", MP BIT2 "RESCALE AOS TO PI/4" (its low
 * word, unless the high one is nonzero: then "USE FULL SCALE FOR LARGER
 * AOS ESTIMATES.", "LIMITS +1 CONTAINS NEGMAX." "LIMITS -1 CONTAINS
 * POSMAX."). The rates: "RATE CELLS USE ADJACENT, NOT SEPARATED." Returns
 * whether K is nonzero: "CONTROL AUTHORITY ZERO.  AVOID DRIVING ENGINE
 * BELL TO THE STOPS." */
static bool axis_inputs(void)
{
    ca(by_axis(ADDRESS_OF(AOSQ)));
    mp(BIT2);
    if (!A_is_zero()) {
        cs(indexed(LIMITS, a_word()));
        xch(L_ADDRESS);
    }
    switch (ccs_at(ADDRESS_OF(QRCNTR))) {
    case POSITIVE: ca(indexed(ADDRESS_OF(EDOTQ), a_word())); break;
    case IS_PLUS_ZERO: ca(ADDRESS_OF(EDOTQ)); break;
    case NEGATIVE: break;                             /* to GTSQAXIS */
    case IS_MINUS_ZERO: not_modelled(); return false; /* QRCNTR is 2, +-1 or 0 */
    }
    dxch(ADDRESS_OF(WCENTRAL));  /* GTSQAXIS */
    ca(by_axis(ADDRESS_OF(KQ))); /* "COLLECT K FOR THIS AXIS" */
    ts(ADDRESS_OF(KCENTRAL));
    return !A_is_zero();
}

/* ALGORTHM: the first term of NEGUSUM, K times the error ("Q(R)DIFF IS
 * THETA (ERROR) SCALED AT PI."), "D.P. AT 4*PI(2)" in K2THETA. Then
 * ALPHA(2)/(2*K) in A2CNTRAL, "LIMITING QUOTIENT TO AVOID OVERFLOW."
 * ("K-ALPHA(2)/2 SHOULD BE PNZ FO DIVISION"; else HUGEQUOT, POSMAX). */
static void error_term_and_quotient(void)
{
    ca(by_axis(ADDRESS_OF(QDIFF))); /* "QDIFF, RDIFF ARE STORED IN D.P." */
    mp_at(ADDRESS_OF(KCENTRAL));
    lxch(ADDRESS_OF(K2THETA));
    mp(BIT5); /* "RESCALE TO 4*PI(2)" */
    dxch(ADDRESS_OF(K2THETA));
    mp(BIT5);
    ads(ADDRESS_OF(K2THETA) + 1u); /* "NO CARRY NEEDED" */

    cs(ADDRESS_OF(ACENTRAL));
    mp(BIT14);
    mp_at(ADDRESS_OF(ACENTRAL));
    ad_at(ADDRESS_OF(KCENTRAL));
    if (A_is_zero_or_minus()) {
        set_A(POSMAX); /* HUGEQUOT */
        ts(L_ADDRESS);
        dxch(ADDRESS_OF(A2CNTRAL));
        return;
    }
    dcom(); /* "ALPHA(2)/2 - K" */
    ad_at(ADDRESS_OF(KCENTRAL));
    dv_at(ADDRESS_OF(KCENTRAL)); /* "HIGH ORDER OF QUOTIENT." */
    xch(ADDRESS_OF(A2CNTRAL));
    ca(L_ADDRESS); /* "SHIFT UP THE REMAINDER." */
    zl();          /* "ZERO LOW-ORDER DIVIDEND." */
    dv_at(ADDRESS_OF(KCENTRAL));
    xch(ADDRESS_OF(A2CNTRAL) + 1u);
}

/* HAVEQUOT to GETROOT: "RESCALE OMEGA AT 16*PI IN D.P." into K2CNTRAL
 * and FUNCTION; "OMEGA + ALPHA*ABS(ALPHA)/(2*K) AT 16*PI" in FUNCTION, and
 * DEL its sign, +1, -1 or +0 ("DEL = +1 FOR FUNCT1 GREATER THAN ZERO.").
 * "REPLACE OMEGA BY DEL*OMEGA" (DEFUNCT: +0; NEGFNCT2: negated). FUNCT2:
 * "DEL*OMEGA + ALPHA(2)/(2*K)" in K2CNTRAL; FUNCT3: A2CNTRAL times -1/3,
 * plus K2CNTRAL: "DEL*OMEGA + ALPHA(2)/(3*K)", times ALPHA into K2THETA
 * ("TWO TERMS OF NEGUSUM ACCUMULATED, SO FAR"). GETROOT: K times
 * K2CNTRAL, "THE TERM FOR WHICH A SQUARE ROOT IS NEEDED.", in FUNCTION. */
static void second_term(void)
{
    ca(ADDRESS_OF(WCENTRAL)); /* HAVEQUOT */
    mp(BIT9);
    dxch(ADDRESS_OF(K2CNTRAL)); /* "LOWER WORD OVERLAYS OMEGA IN WCENTRAL" */
    dca(ADDRESS_OF(K2CNTRAL));
    dxch(ADDRESS_OF(FUNCTION));
    ca(ADDRESS_OF(ACENTRAL)); /* "GET ALPHA*ABS(ALPHA)/(2*K)" */
    if (A_is_zero_or_minus())
        dcs(ADDRESS_OF(A2CNTRAL));
    else
        dca(ADDRESS_OF(A2CNTRAL));
    das(ADDRESS_OF(FUNCTION));

    sign_of_double(ADDRESS_OF(FUNCTION));
    ts(ADDRESS_OF(DEL));
    switch (ccs_at(ADDRESS_OF(DEL))) {
    case POSITIVE: break; /* "POSITIVE DEL VALUE.  PROCEED." */
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(K2CNTRAL)); /* DEFUNCT */
        ts(ADDRESS_OF(K2CNTRAL) + 1u);
        break;
    case NEGATIVE:
        dcs(ADDRESS_OF(K2CNTRAL)); /* NEGFNCT2 */
        dxch(ADDRESS_OF(K2CNTRAL));
        break;
    }
    dca(ADDRESS_OF(A2CNTRAL)); /* FUNCT2 */
    das(ADDRESS_OF(K2CNTRAL));

    ca(ADDRESS_OF(A2CNTRAL)); /* FUNCT3 */
    mp(NEG1_3);
    dxch(ADDRESS_OF(A2CNTRAL));
    ca(L_ADDRESS);
    mp(NEG1_3);
    ads(ADDRESS_OF(A2CNTRAL) + 1u);
    carry_into(ADDRESS_OF(A2CNTRAL)); /* "-ALPHA(2)/(6*K)" */
    dca(ADDRESS_OF(K2CNTRAL));
    das(ADDRESS_OF(A2CNTRAL));
    ca(ADDRESS_OF(A2CNTRAL));
    mp_at(ADDRESS_OF(ACENTRAL));
    das(ADDRESS_OF(K2THETA));
    ca(ADDRESS_OF(A2CNTRAL) + 1u);
    mp_at(ADDRESS_OF(ACENTRAL)); /* "ACENTRAL MAY NOW BE OVERLAID." */
    ads(ADDRESS_OF(K2THETA) + 1u);
    carry_into(ADDRESS_OF(K2THETA));

    ca(ADDRESS_OF(K2CNTRAL)); /* GETROOT */
    mp_at(ADDRESS_OF(KCENTRAL));
    dxch(ADDRESS_OF(FUNCTION));
    ca(ADDRESS_OF(K2CNTRAL) + 1u);
    mp_at(ADDRESS_OF(KCENTRAL));
    ads(ADDRESS_OF(FUNCTION) + 1u);
    carry_into(ADDRESS_OF(FUNCTION)); /* "DESIRED TERM IN FUNCTION, AT PI(2)/16" */
}

/* DOSHIFT's INDEX SHFTFLAG; CA BIT15: 2 to the power -S into A. The index
 * is added to the whole instruction word: SHFTFLAG is S, 0 to 13, after a
 * root, but after the zero root, whose ININDEX is ACENTRAL's cell, it is
 * ACENTRAL/2, and the CA reads another word (CA Z the address after it),
 * or the sum is another order, which is not modelled. */
static void shift_divisor(void)
{
    word order = indexed_instruction(read_word(DOSHIFT + 7u), fetch(ADDRESS_OF(SHFTFLAG)));

    if (OPERATION_CODE(order) != CA_CODE) not_modelled();
    if (ADDRESS_FIELD(order) == Z_ADDRESS) continue_at(DOSHIFT + 010u);
    ca(ADDRESS_FIELD(order));
}

/* The third term, when DEL is nonzero: RSTOFGTS, the square root of
 * FUNCTION, then PRODUCT: times K2CNTRAL, "DEL*(DEL*OMEGA + ALPHA /(2*K))"
 * with its root, and DOSHIFT: "MULTIPLY IN THE FACTOR 2(-S), RETURNED BY
 * THE GTSQRT SUBROUTINE" (S is SHFTFLAG plus half ININDEX), added into
 * K2THETA (ADDITIN, for S zero: "NOW ADD IN THE K2THETA TERM."). */
static void third_term(void)
{
    tc_at(RSTOFGTS);
    gtsqrt();
    xch(ADDRESS_OF(K2CNTRAL)); /* PRODUCT */
    mp_at(ADDRESS_OF(K2CNTRAL));
    dxch(ADDRESS_OF(K2CNTRAL));
    mp_at(L_ADDRESS);
    ads(ADDRESS_OF(K2CNTRAL) + 1u);
    carry_into(ADDRESS_OF(K2CNTRAL));

    ca(ADDRESS_OF(ININDEX)); /* DOSHIFT */
    mp(BIT14);
    ads(ADDRESS_OF(SHFTFLAG));
    if (A_is_zero()) {
        dca(ADDRESS_OF(K2CNTRAL)); /* ADDITIN */
        das(ADDRESS_OF(K2THETA));
        return;
    }
    shift_divisor();
    xch(ADDRESS_OF(K2CNTRAL));
    mp_at(ADDRESS_OF(K2CNTRAL));
    das(ADDRESS_OF(K2THETA));
    xch(ADDRESS_OF(K2CNTRAL));
    mp_at(ADDRESS_OF(K2CNTRAL) + 1u);
    ads(ADDRESS_OF(K2THETA) + 1u);
    carry_into(ADDRESS_OF(K2THETA));
}

/* NEGUSUM to LOUPE, A the new drive: +1 for a positive NEGUSUM (NEGDRIVE),
 * -1 (POSDRIVE), or +0 ("STOP GIMBAL DRIVE FOR A ZERO NEGUSUM."). It goes
 * to NEGUQ or NEGUR, and the old drive times the new one tells: "NON-ZERO
 * GIMBAL DRIVE BEING CONTINUED.", LOUPE; "NON-ZERO GIMBAL DRIVE BEING
 * REVERSED.", REVERSAL: "A ZERO-DRIVE PAUSE IS NEEDED HERE." (QACCDOT or
 * RACCDOT +0, the axis's drive bits off in channel 12); else ZEROLOUP:
 * "SET UP REQUEST FOR ACDT+C12 CALL." */
static void new_drive(void)
{
    ts(L_ADDRESS); /* "SAVE FOR DRIVE REVERSAL TEST." */
    xch(by_axis(ADDRESS_OF(NEGUQ)));
    mp_at(L_ADDRESS); /* "MULTIPLY OLD NEGU AND NEW NEGU." */
    switch (ccs_at(L_ADDRESS)) {
    case POSITIVE: return;
    case NEGATIVE:
        ts(by_axis(ADDRESS_OF(QACCDOT))); /* REVERSAL */
        cs(by_axis(GMBLBITA));
        wand_channel(CHAN12);
        break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* "NO REVERSAL PROBLEM HERE." */
    }
    cs(ADDRESS_OF(RCSFLAGS)); /* ZEROLOUP */
    mask(CALLGMBL);
    ads(ADDRESS_OF(RCSFLAGS));
}

/* GTS, from Q_R-AXIS by DTCB: "THE ENTRY TO THE GIMBAL TRIM SYSTEM FOR
 * CONTROLLING ATTITUDE ERRORS AND RATES AS WELL AS ACCELERATIONS." "MAKE
 * THE NEXT PASS THROUGH THE DAP BE THROUGH RCS CONTROL," (COTROLER -1)
 * "AND ENSURE THAT IT IS NOT A SKIP." (SKIPU, SKIPV four); INGTS and the
 * gimbal timers two ("SET TIMERS TO 200 MSEC TO AVOID BOTH RUNAWAY AND
 * INTERFERENCE BY NULLING."); SR saved doubled, "TO CORRECT FOR THE RIGHT
 * SHIFT DUE TO EDITING." Then, for the R axis and the Q axis (QRCNTR 2,
 * then 0; a TS SAVESR that overflows leaves QRCNTR +1 or -1), "THE DRIVE
 * SETTING ALGORITHM": NEGUSUM, the sum of K times the error, ALPHA times
 * "DEL*OMEGA + ALPHA /(3*K)", and DEL times the 3/2 power of K times
 * "DEL*OMEGA + ALPHA /(2*K)"; "DRIVE = -SGN(NEGUSUM)". Then SR as it was,
 * and GOCLOSE: "TERMINATE THE JASK." by DTCB to CLOSEOUT. */
void gts(void)
{
    bool more = true;

    set_A(NEGONE);
    ts(ADDRESS_OF(COTROLER));
    set_A(FOUR);
    ts(ADDRESS_OF(SKIPU));
    ts(ADDRESS_OF(SKIPV));
    set_A(TWO);
    ts(ADDRESS_OF(INGTS)); /* "SET INDICATOR OF GTS CONTROL POSITIVE." */
    ts(ADDRESS_OF(QGIMTIMR));
    ts(ADDRESS_OF(RGIMTIMR));
    ca(SR_ADDRESS); /* "SAVE THE SR." */
    ad_at(A_ADDRESS);
    if (!ts(ADDRESS_OF(SAVESR))) set_A(TWO); /* GTSGO+ON: "SET INDEXER FOR R-AXIS CALCULATIONS." */
    while (more) {
        ts(ADDRESS_OF(QRCNTR)); /* GOQTRIMG +1 */
        if (axis_inputs()) {
            error_term_and_quotient();
            second_term();
            switch (ccs_at(ADDRESS_OF(DEL))) {
            case POSITIVE: third_term(); break;
            case NEGATIVE: /* NEGATE */
                dcs(ADDRESS_OF(K2CNTRAL));
                dxch(ADDRESS_OF(K2CNTRAL));
                third_term();
                break;
            case IS_PLUS_ZERO:
            case IS_MINUS_ZERO: break; /* to NEGUSUM */
            }
            sign_of_double(ADDRESS_OF(K2THETA)); /* NEGUSUM */
        }
        new_drive();
        switch (ccs_at(ADDRESS_OF(QRCNTR))) { /* LOUPE: "HAVE BOTH AXES BEEN PROCESSED?" */
        case POSITIVE: set_A(ZERO); break;    /* GOQTRIMG: "DO Q AXIS NEXT." */
        case IS_PLUS_ZERO:
            ca(ADDRESS_OF(SAVESR)); /* "RESTORE THE SR" */
            ts(SR_ADDRESS);
            more = false;
            break;
        case NEGATIVE:
            ts(SR_ADDRESS);
            more = false;
            break;
        case IS_MINUS_ZERO: not_modelled(); return; /* QRCNTR is 2, +-1 or 0 */
        }
    }
    set_A(CLOSEADR[0]); /* GOCLOSE: EXTEND; DCA CLOSEADR; DTCB */
    set_L(CLOSEADR[1]);
    dtcb(GOCLOSE + 2u);
}

/* ---- ACDT+C12 -- LM pp. 1476-1477 ---- */

/* ACDT+C12, by IBNKCALL (from TIMEGMBL, or P-AXIS when CALLGMBL of
 * RCSFLAGS asks), Q the return: "SETS Q(R)ACCDOT TO REFLECT THE NEW
 * DRIVES." (the drive times ACCDOTQ or ACCDOTR, the acceleration's rate of
 * change, NEGUQ and NEGUR being "THE NEGATIVES OF THE DESIRED ACCELERATION
 * CHANGES"), and the drive bits 9 to 12 of channel 12 from their signs:
 * bit 10 or 9 for Q, 12 or 11 for R, none for +0, in CHNL12 ("(STORED
 * RESULT NOT USED AT PRESENT)"). "WARNING:  ACDT+C12 AND WRCHN12 MUST BE
 * CALLED WITH INTERRUPT INHIBITED." Then "TURN OFF REQUEST FOR ACDT+C12
 * EXECUTION." and "RETURN TO CALLER." */
void acdt_c12(void)
{
    cs(ADDRESS_OF(NEGUQ));
    mp_at(ADDRESS_OF(ACCDOTQ)); /* "GIMBAL DRIVE REQUESTS." */
    lxch(ADDRESS_OF(QACCDOT));
    cs(ADDRESS_OF(NEGUR));
    mp_at(ADDRESS_OF(ACCDOTR));
    lxch(ADDRESS_OF(RACCDOT));

    switch (ccs_at(ADDRESS_OF(NEGUQ))) {
    case POSITIVE: set_A(BIT10); break;
    case NEGATIVE: set_A(BIT9); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break; /* A +0 */
    }
    ts(ADDRESS_OF(CHNL12));
    switch (ccs_at(ADDRESS_OF(NEGUR))) {
    case POSITIVE: set_A(BIT12); break;
    case NEGATIVE: set_A(BIT11); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    ads(ADDRESS_OF(CHNL12));

    set_A(negate(BGIM));
    rand_channel(CHAN12);
    ad_at(ADDRESS_OF(CHNL12));
    write_channel(CHAN12);

    set_A(negate(CALLGMBL));
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    tc_q();
}

/* ---- TIMEGMBL -- LM pp. 1478-1480 ---- */

enum drive { DRIVE_OFF, DRIVE_NOT_ALLOWED, DRIVE_ON };

/* The drive for the axis QRNDXER names, 2 (R) or 0 (Q): "A ZERO ALPHA OR
 * ACCDOT OR A ZERO QUOTIENT TURNS OFF THE GIMBAL DRIVE IMMEDIATELY."
 * "OTHERWISE, THE GIMBAL IS TURNED ON DRIVING IN THE CORRECT DIRECTION."
 * ALPHA is "-1.6*AOS AT PI/2 = -.4*AOS AT PI/8.", MP OCT23146 ("OCTAL
 * 23146 IS DECIMAL .6") plus -AOS; when that overflows ("YES.  START DRIVE
 * WITHOUT WAITLIST."), NEGU gets -SGN(AOS) and the drive is not allowed.
 * Else NEGU gets SGN(ALPHA), and the time "AS ABS(ALPHA/ACCDOT)" is left in
 * A, unless it is over two seconds. */
static enum drive drive_setting(unsigned axis)
{
    ca(indexed(ADDRESS_OF(ACCDOTQ), axis));     /* "ACCDOT IS PRESUMED TO BE AT PI/2(7)." */
    if (A_is_zero_or_minus()) return DRIVE_OFF; /* "IS ACCDOT LESS THAN OR EQUAL TO 0?" */
    ts(ADDRESS_OF(NZACCDOT));
    cs(indexed(ADDRESS_OF(AOSQ), axis)); /* ALPHATRY */
    if (A_is_zero()) return DRIVE_OFF;   /* "IS ALPHA ZERO?" */
    ts(Q_ADDRESS);                       /* "SAVE A COPY OF  -AOS." */
    mp(OCT23146);
    ad_at(Q_ADDRESS);
    if (ts(L_ADDRESS)) { /* "WAS THERE OVERFLOW?" */
        com();           /* "RECOVER  -SGN(AOS) IN THE A REGISTER." */
        xch(indexed(ADDRESS_OF(NEGUQ), axis));
        return DRIVE_NOT_ALLOWED;
    }
    if (A_is_zero_or_minus()) { /* SETNEGU */
        ts(ADDRESS_OF(ITEMP2)); /* POSALPH */
        set_A(BIT1);
    } else {
        com();
        ts(ADDRESS_OF(ITEMP2)); /* "STORE  -ABS(.4*AOS) SCALED AT PI/8." */
        set_A(negate(BIT1));
    }
    ts(indexed(ADDRESS_OF(NEGUQ), axis)); /* "STORE SGN(ALPHA) AS NEGU" */
    ca(ADDRESS_OF(NZACCDOT));
    mp(BIT12); /* "2*ACCDOT, SCALED AT PI/8." */
    ad_at(ADDRESS_OF(ITEMP2));
    if (A_is_zero_or_minus()) return DRIVE_NOT_ALLOWED; /* "IS DRIVE TIME MORE THAN TWO SECONDS?" */
    cs(ADDRESS_OF(ITEMP2));                             /* "NO.  COMPUTE DRIVE TIME." */
    mp(OCT00240);
    dv_at(ADDRESS_OF(NZACCDOT));       /* "QUOTIENT IS DRIVE TIME AT WAITLIST." */
    if (A_is_zero()) return DRIVE_OFF; /* "DRIVE TIME MUST BE GREATER THAN ZERO." */
    return DRIVE_ON;
}

/* TIMEGMBL, by TC, Q the return: "COMPUTES THE DRIVE TIME NEEDED FOR THE
 * TRIM GIMBAL TO POSITION THE DESCENT ENGINE NOZZLE SO AS TO NULL THE
 * OFFSET ANGULAR ACCELERATION ABOUT THE Q (OR R) AXIS." For the R axis,
 * then the Q axis: TGOFFNOW ("TURN OFF GIMBAL NOW.", NEGU +0); NOTALLOW
 * (the timer 25, and ALLOWGTS +0: "DO NOT PERMIT FURTHER GTS
 * ATTITUDE-RATE CONTROL UNTIL AOSTASK APPROVES."); or DRIVEON, the time in
 * QGIMTIMR or RGIMTIMR. Then ACDT+C12 by IBNKCALL: "TURN OFF CHANNEL
 * BITS, SET Q(R)ACCDOTS.", RUPTREG3,4 kept in ITEMP2,3: "PROTECT IBNKCALL
 * ERASABLES.  ACDT+C12 LEAVES ITEMPS2,3 ALONE." */
void timegmbl(void)
{
    bool more = true;

    set_A(ONE); /* "INITIALZE ALLOWGTS." */
    ts(ADDRESS_OF(ALLOWGTS));
    set_A(TWO);      /* "SET UP LOOP FOR R AXIS." */
    lxch(Q_ADDRESS); /* "SAVE RETURN ADDRESS." */
    lxch(ADDRESS_OF(RUPTREG2));
    while (more) {
        unsigned axis;

        ts(ADDRESS_OF(QRNDXER));
        axis = fetch(ADDRESS_OF(QRNDXER));
        switch (drive_setting(axis)) {
        case DRIVE_OFF:
            set_A(ZERO); /* TGOFFNOW */
            ts(indexed(ADDRESS_OF(NEGUQ), axis));
            break;
        case DRIVE_NOT_ALLOWED:
            set_A(OCT31); /* NOTALLOW */
            ts(indexed(ADDRESS_OF(QGIMTIMR), axis));
            set_A(ZERO);
            ts(ADDRESS_OF(ALLOWGTS));
            break;
        case DRIVE_ON: ts(indexed(ADDRESS_OF(QGIMTIMR), axis)); break; /* DRIVEON */
        }
        switch (ccs_at(ADDRESS_OF(QRNDXER))) { /* DONEYET */
        case POSITIVE: set_A(ZERO); break;     /* TIMQGMBL: "NOW DO THE Q-AXIS" */
        case IS_PLUS_ZERO: more = false; break;
        default: not_modelled(); return; /* QRNDXER is 2 or 0 */
        }
    }
    dxch(ADDRESS_OF(RUPTREG3));
    dxch(ADDRESS_OF(ITEMP2));
    tc_to(DONEYET + 4u, IBNKCALL); /* CADR ACDT+C12 */
}

/* DONEYET +6, where ACDT+C12 returns: "RESTORE ERASABLES FOR IBNKCALL."
 * and "RETURN TO CALLER." by TC RUPTREG2. */
void timegmbl_return(void)
{
    dxch(ADDRESS_OF(ITEMP2));
    dxch(ADDRESS_OF(RUPTREG3));
    tc_through(ADDRESS_OF(RUPTREG2));
}

#endif
