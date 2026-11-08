/* SPDX-License-Identifier: MIT */
/* TVCROLLDAP.agc (Comanche055 pp. 984-998, bank 16): the "TVC ROLL
 * AUTOPILOT", "AN ADAPTATION OF THE LEM P-AXIS CONTROLLER". It keeps the
 * outer gimbal angle OGA "WITHIN 5 DEG DEADBND OF OGAD" and its rate below
 * a limit cycle rate by firing roll jet pairs, timed by T6, from a
 * switching logic in the phase plane (OGAERROR against OGARATE). ROLLDAP
 * runs as a WAITLIST task TVCEXEC calls every half second; NOROLL1, the T6
 * task JETROLL sets up, shuts the jets off. The faithful C is
 * c64-port/src/cm_tvc_roll.c.
 *
 * The phase plane holds twelve regions (1 to 6 and 1-prime to 6-prime);
 * a test for each in turn (REG1TST to REG5TST) finds where the operating
 * point is. Out of the coast region, the desired OGARATE is where the
 * control parabola through the operating point (or, in region 1, a dummy
 * point) meets the straight switch line, by "A STRAIGHT LINE APPROXIMATION
 * TO A QUADRATIC SOLUTION" (ONROLL); the jet firing time follows from the
 * change in rate (ROLLSET). OGA itself holds the error (OGA - OGAD). */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* INDEX I; 0 <address>: I holds an order, CA (PRIO30, 030000) when DELOGA
 * is positive and CS (BIT15, 040000) when not, so the word comes with the
 * sign of DELOGA ("TSET ON I SGN(OGARATE)", the first use). */
static void index_i(unsigned address)
{
    if (fetch(ADDRESS_OF(I)) == PRIO30)
        ca(address);
    else
        cs(address);
}

/* TS TEMREG with its "OVERFLOW PROTECT": an overflow skips the TCF +3 to
 * INDEX A; CS LIMITS, so the +1 or -1 it left in A picks LIMITS +1 or
 * LIMITS -1, a limit of the overflow's sign. */
static void ts_temreg_protected(void)
{
    if (ts(ADDRESS_OF(TEMREG))) cs(indexed(ADDRESS_OF(LIMITS), a_word()));
}

/* ---- The jets -- pp. 994-996 ---- */

/* T6ENABL: channel 13's bit 15 enables T6; RDAPEND, "EXIT ROLL DAP". */
static void t6enabl(void)
{
    set_A(BIT15);
    wor_channel(CHAN13);
    continue_at(TASKOVER);
}

/* +TORQUE: the positive roll jets, pairs 9-11 and 13-15 in turn ("WHAT WAS
 * THE LAST +TORQUE COMBINATION"), bit 1 of ROLLWORD naming the last. */
static void plus_torque(void)
{
    ca(ADDRESS_OF(ROLLWORD));
    mask(BIT1);
    if (A_is_zero()) { /* NO.9-11: "NOT 9-11, SO USE IT THIS TIME" */
        set_A(BIT1);
        ads(ADDRESS_OF(ROLLWORD)); /* "CHANGE BIT 1 TO ONE" */
        set_A(PLUS_ROLL1);
    } else {                 /* NO.13-15 */
        set_A(negate(BIT1)); /* CS */
        mask_at(ADDRESS_OF(ROLLWORD));
        ts(ADDRESS_OF(ROLLWORD)); /* "CHANGE BIT 1 TO ZERO" */
        set_A(PLUS_ROLL2);
    }
    write_channel(CHAN6);
    t6enabl();
}

/* -TORQUE: the negative roll jets, pairs 12-10 and 16-14, by bit 2. */
static void minus_torque(void)
{
    ca(ADDRESS_OF(ROLLWORD));
    mask(BIT2);
    if (A_is_zero()) { /* NO.12-10: "NOT 12-10, SO USE IT THIS TIME" */
        set_A(BIT2);
        ads(ADDRESS_OF(ROLLWORD)); /* "CHANGE BIT 2 TO ONE" */
        set_A(MINUS_ROLL1);
    } else {                 /* NO.16-14 */
        set_A(negate(BIT2)); /* CS */
        mask_at(ADDRESS_OF(ROLLWORD));
        ts(ADDRESS_OF(ROLLWORD)); /* "CHANGE BIT 2 TO ZERO" */
        set_A(MINUS_ROLL2);
    }
    write_channel(CHAN6);
    t6enabl();
}

/* JETROLL: NOROLL1 as the T6 task, TEMREG (the firing time, or -0) into
 * TIME6. "I=0 IF MOREROLL,KEEP SAME JETS ON" (SAMEJETS); otherwise the
 * jets of ROLLFIRE's sign, or none. */
static void jetroll(void)
{
    dca(ADDRESS_OF(NOROL1T6));
    dxch(ADDRESS_OF(T6LOC));
    ca(ADDRESS_OF(TEMREG));
    ts(ADDRESS_OF(TIME6));
    ca(ADDRESS_OF(I));
    if (A_is_zero()) { /* SAMEJETS */
        continue_at(TASKOVER);
        return;
    }
    switch (ccs_at(ADDRESS_OF(ROLLFIRE))) {
    case POSITIVE: plus_torque(); break;
    case NEGATIVE: minus_torque(); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: t6enabl(); break;
    }
}

/* NOROLL, "COAST....(NEG ZERO FOR TIME6)": ROLLFIRE and TEMREG -0 ("NOTE,
 * JETS CAN FIRE NEXT PASS"). */
static void noroll(void)
{
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(ROLLFIRE));
    ts(ADDRESS_OF(TEMREG));
    jetroll();
}

/* NOMXFIRE: ROLLFIRE POSMAX or NEGMAX, "FOR TORQUE SIGN" (TEMREG is not
 * zero here), and TEMREG its magnitude. FIRELOOK: "IS IT MOREROLL"; if so,
 * FIREPLUG, "CHECK FOR EXTENDED FIRING": "IF EXTENSION WANTED, DONT, EXIT
 * ROLL DAP" (EXTENTST), as "JET FIRINGS MAY NOT BE EXTENDED, ONLY
 * SHORTENED". */
static void nomxfire(void)
{
    if (ccs_at(ADDRESS_OF(TEMREG)) == POSITIVE)
        set_A(POSMAX); /* "POSITIVE TORQUE REQUIRED" */
    else
        set_A(NEGMAX); /* "NEGATIVE TORQUE REQUIRED" */
    ts(ADDRESS_OF(ROLLFIRE));
    com();
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +3 */
        cs(ADDRESS_OF(TEMREG));
        ts(ADDRESS_OF(TEMREG));
    }
    ca(ADDRESS_OF(I)); /* FIRELOOK */
    if (!A_is_zero()) {
        jetroll();
        return;
    }
    ca(ADDRESS_OF(TIME6)); /* FIREPLUG: CAE */
    su_at(ADDRESS_OF(TEMREG));
    if (A_is_zero_or_minus()) { /* EXTENTST */
        continue_at(TASKOVER);
        return;
    }
    jetroll();
}

/* MAXTFIRE: "PROCEED WITH NEW FIRING BUT NOT LONGER THAN TMAXFIRE" (MP by
 * 1/TMXFIR is not zero from 2.56 seconds on: MAXTST). */
static void maxtfire(void)
{
    ca(ADDRESS_OF(TEMREG));
    mp(ONE_OVER_TMXFIR);
    if (!A_is_zero()) { /* EXTEND; BZF NOMXFIRE */
        if (ccs_register(A_ADDRESS) == POSITIVE)
            set_A(TMAXFIRE); /* "USE MAXIMUM" */
        else
            set_A(negate(TMAXFIRE)); /* CS */
        ts(ADDRESS_OF(TEMREG));
    }
    nomxfire();
}

/* NEWROLL: "START NEW FIRING BUT CHECK IF GREATER THAN MIN FIRE TIME":
 * TMINFIRE less the magnitude of TEMREG ("CALL THIS T6FIRE"); "IF NOT
 * GREATER THAN TMINFIRE" (MINTST), no firing. */
static void newroll(void)
{
    switch (ccs_at(ADDRESS_OF(TEMREG))) {
    case POSITIVE:
    case NEGATIVE: ad(ONE); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    com(); /* "-MAG(T6FIRE)" */
    ad(TMINFIRE);
    com();
    if (A_is_zero_or_minus()) { /* MINTST */
        noroll();
        return;
    }
    maxtfire();
}

/* ROLLSET, with the desired OGARATE in A: the jet firing time, the change
 * in rate times T6SCALE and 1/CONACC ("JET FIRE TIME AT 625 MICROSEC/BIT",
 * "POS MEANS POSITIVE ROLL TORQUE"), each step overflow protected. None:
 * NOROLL. Otherwise by its sign against ROLLFIRE's (MP ROLLFIRE; CCS A):
 * MOREROLL, "CONTINUE FIRING WITH PRESENT POLARITY" (I +0, "USE TEMP. AS
 * MOREROLL SWITCH"); NEWROLL, "START NEW FIRING NOW" (the jets now off);
 * or NOROLL, "TERMINATE OLD FIRING, NEW SIGN REQUESTED". */
static void rollset(void)
{
    ts(ADDRESS_OF(TEMREG)); /* "STORE DESIRED OGARATE" */
    su_at(ADDRESS_OF(OGARATE));
    ts_temreg_protected();
    mp(T6SCALE);
    mp_at(ADDRESS_OF(1_CONACC));
    ddoubl();
    ddoubl();
    ts_temreg_protected();
    ts(ADDRESS_OF(TEMREG));
    if (A_is_zero()) {
        noroll();
        return;
    }
    ca(ADDRESS_OF(TEMREG)); /* CAE */
    mp_at(ADDRESS_OF(ROLLFIRE));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: /* MOREROLL */
        set_A(ZERO);
        ts(ADDRESS_OF(I));
        maxtfire();
        break;
    case NEGATIVE: noroll(); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: newroll(); break;
    }
}

/* ---- The desired rate -- pp. 991-993 ---- */

/* ONROLL, with DELOGART in A: "SOLVE STRAIGHT LINES SIMULTANEOUSLY TO
 * OBTAIN DESIRED OGARATE", DOGADOT = NUM/DEN, the denominator in TEMREG
 * (RATEDEN) and the numerator in A (RATENUM). The division by TEMREG
 * first divides +0,NUM, whose quotient is not zero only when NUM/TEMREG
 * overflows (MINLIMAP: then POSMAX of the overflow's sign); DVOK divides
 * NUM,+0. */
static void onroll(void)
{
    mp_at(ADDRESS_OF(1_CONACC));
    double_A();
    mp(MINUS_SLOPE);
    ts(ADDRESS_OF(TEMREG)); /* "2(-SLOPE)RATE /CONACC" */
    mp_at(ADDRESS_OF(DELOGART));
    ts(ADDRESS_OF(DELOGART)); /* "2(-SLOPE)(RATESQ)/CONACC" */
    set_A(negate(BIT11));     /* CS */
    index_i(A_ADDRESS);
    ads(ADDRESS_OF(TEMREG)); /* RATEDEN: "DENOMINATOR COMPLETED" */
    index_i(ADDRESS_OF(DELOGA));
    com();
    ad(DB);
    com();
    mp(MINUS_SLOPE);
    ads(ADDRESS_OF(DELOGART));
    set_A(LMCRATE);
    mp(BIT11);
    ad_at(ADDRESS_OF(DELOGART)); /* RATENUM: "NUMERATOR COMPLETED" */
    xch(L_ADDRESS);              /* "PLACE NUMERATOR IN L FOR OVERFL. CHECK" */
    set_A(ZERO);
    dv_at(ADDRESS_OF(TEMREG));
    if (!A_is_zero()) { /* MINLIMAP */
        if (ccs_register(A_ADDRESS) == POSITIVE)
            set_A(POSMAX); /* "POSITIVE OVERFLOW" */
        else
            set_A(negate(POSMAX)); /* CS: "NEGATIVE OVERFLOW" */
        rollset();
        return;
    }
    lxch(A_ADDRESS); /* DVOK: "PUT NUMERATOR BACK INTO A, 0 INTO L" */
    dv_at(ADDRESS_OF(TEMREG));
    rollset(); /* "RESULT OF DIVISION IS DESIRED OGARATE" */
}

/* REGION6: "USE ACTUAL OPERATING POINT FOR TANGENT". */
static void region6(void)
{
    ca(ADDRESS_OF(OGA)); /* CAE */
    ts(ADDRESS_OF(DELOGA));
    ca(ADDRESS_OF(OGARATE));
    ts(ADDRESS_OF(DELOGART));
    onroll();
}

/* ROLLON, region 1: the dummy operating point, OGAERROR = DELOGA + DB
 * SGN(DELOGA) and OGARATE = -LMCRATE SGN(DELOGA) ("EVALUATE STATE FOR
 * INDIRECT LINE"). */
static void rollon(void)
{
    index_i(ADDRESS_OF(DB));
    ads(ADDRESS_OF(DELOGA)); /* "DELOGA WAS DIST. FROM SWITCH PARABOLA" */
    set_A(negate(LMCRATE));  /* CS */
    index_i(A_ADDRESS);
    ts(ADDRESS_OF(DELOGART));
    onroll();
}

/* RATELIM, region 3: "IF I = CA, DESIRED RATE IS -MAXLIM". */
static void ratelim(void)
{
    set_A(negate(MAXLIM)); /* CS */
    index_i(A_ADDRESS);
    rollset();
}

/* ---- ROLLDAP -- pp. 989-991 ---- */

/* ROLLOGIC: TEMREG "DB-(1/2 CONACC) (OGARATE)SQ", SGNRT the sign of
 * OGARATE ("LET SGN(0) BE NEGATIVE"), DELOGA the distance from the switch
 * parabola ("PLUS TO RIGHT OF C-PARAB") and I the CA or CS of its sign.
 * Then the regions in turn. (An overflow at TS TEMREG skips the CA
 * OGARATE, and the sign is the +1 or -1 it leaves. One at DELOGAC skips
 * the EXTEND: BZMF +3 runs as an AD, which CAF PRIO30 then overwrites, so
 * I is CA.) */
static void rollogic(void)
{
    cs(ADDRESS_OF(OGARATE));
    mp_at(ADDRESS_OF(1_CONACC));
    mp_at(ADDRESS_OF(OGARATE));
    ad(DB);
    if (!ts(ADDRESS_OF(TEMREG))) ca(ADDRESS_OF(OGARATE));
    if (A_is_zero_or_minus()) /* EXTEND; BZMF +3 */
        set_A(negate(BIT1));  /* CS */
    else
        set_A(BIT1);
    ts(ADDRESS_OF(SGNRT));
    mp_at(ADDRESS_OF(TEMREG)); /* "SGN(OGARATE) TEMREG NOW IN L" */
    cs(L_ADDRESS);
    ad_at(ADDRESS_OF(OGA));
    if (ts(ADDRESS_OF(DELOGA)) || !A_is_zero_or_minus()) /* DELOGAC */
        set_A(PRIO30);                                   /* "=CA (30000)" */
    else
        set_A(BIT15); /* "=CS (40000)" */
    ts(ADDRESS_OF(I));
    index_i(ADDRESS_OF(SGNRT));
    com();
    if (A_is_zero_or_minus()) { /* REG1TST: "DELOGA OGARATE SAME SIGN" */
        rollon();
        return;
    }
    index_i(ADDRESS_OF(OGARATE)); /* "BOTH MUST BE NEG. HERE" */
    ts(ADDRESS_OF(IOGARATE));
    ad(MAXLIM);
    if (A_is_zero_or_minus()) { /* REG3TST: "RATES TOO HIGH, FIRE JETS" */
        ratelim();
        return;
    }
    ca(ADDRESS_OF(OGARATE)); /* I((-SLOPE)OGA + OGARATE) - INTERCEPT */
    mp(BIT14);
    ts(ADDRESS_OF(TEMREG));
    ca(ADDRESS_OF(OGA));
    mp(MINUS_SLOPE);
    ddoubl();
    ddoubl();
    ddoubl(); /* "(OGA ERROR MUST BE LESS THAN +-225 DEG)" */
    ad_at(ADDRESS_OF(TEMREG));
    index_i(A_ADDRESS);
    com();
    ad(INTERCEP);
    com();
    if (A_is_zero_or_minus()) { /* REG2TST: "COAST SIDE OF STRT LINE" */
        noroll();
        return;
    }
    ca(ADDRESS_OF(IOGARATE)); /* "ALWAYS NEGATIVE" */
    ad(MINLIM);
    if (A_is_zero_or_minus()) { /* REG4TST: "COAST SIDE OF MINLIM" */
        noroll();
        return;
    }
    index_i(ADDRESS_OF(OGA));
    com();
    ad(DB);
    com();
    if (A_is_zero_or_minus()) { /* REG5TST: "COAST SIDE OF DB" */
        noroll();
        return;
    }
    region6();
}

/* ROLLDAP, the "OGA RATE ESTIMATOR...SIMPLE FIRST-ORDER" "DIFFERENCE
 * (SAMPLE TIME = 1/2 SEC)": OGARATE (OGANOW - OGAPAST) / 16. Then
 * DURATION, "EXAMINE DURATION OF LAST JET FIRING IF JETS ARE NOW ON": on
 * to ROLLOGIC if they are, or if TEMREG is zero; otherwise "JETS HAVE NOT
 * BEEN OFF FOR 1/2 SEC. WAIT" (TEMREG +0; WAIT1/2). */
void rolldap(void)
{
    ca(ADDRESS_OF(OGANOW)); /* CAE */
    msu_at(ADDRESS_OF(OGAPAST));
    mp(BIT5);
    lxch(A_ADDRESS);
    ts(ADDRESS_OF(OGARATE)); /* "SC.AT B-4 REV/SEC" */
    ca(ADDRESS_OF(ROLLFIRE));
    if (A_is_zero()) {          /* "ROLL JETS ARE NOW OFF." */
        ca(ADDRESS_OF(TEMREG)); /* CAE */
        if (!A_is_zero()) {
            set_A(ZERO);
            ts(ADDRESS_OF(TEMREG)); /* "RESET TEMREG" */
            continue_at(TASKOVER);  /* WAIT1/2: "EXIT ROLL DAP" */
            return;
        }
    }
    rollogic();
}

/* NOROLL1, "THIS T6 TASK SHUTS OFF ALL ROLL JETS": ROLLFIRE +0, "ZERO
 * INDICATES JETS NOW OFF", and channel 6 +0 (KILLJETS); NOQRSM. */
void noroll1(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    set_A(ZERO);
    ts(ADDRESS_OF(ROLLFIRE));
    write_channel(CHAN6);
    continue_at(NOQRSM);
}

#endif
