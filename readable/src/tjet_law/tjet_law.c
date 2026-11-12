/* SPDX-License-Identifier: MIT */
/* TJET_LAW.agc, the LM's: Luminary099 pages 1460-1469, bank 17, in EBANK 6
 * (the listing's EBANK= TJP). The faithful C is c64-port/src/lm_tjet_law.c.
 *
 * Every branch of TJETLAW ends at RETURNTJ "WITH TJET" "(SCALED AT 4 SEC)
 * IN THE ACCUMULATOR." So each piece below leaves the jet time in A and
 * returns true, and tjetlaw() then runs RETURNTJ. A piece returns false
 * when it has left at TC CCSHOLE instead. */
#include "tjet_law.h"

#if LUMINARY099

/* RETURN: TC Q. */
static void tc_q(void)
{
    tc_through(Q_ADDRESS);
}

/* The two indexers: ADRSDIF1 picks this axis's parameters from the 1/ACCS
 * block (and the next word for a negative EDOT); ADRSDIF2 too, moved on
 * to the max-jet ones when a decision calls for them. */
static unsigned by_adrsdif1(unsigned address)
{
    return indexed(address, fetch(ADDRESS_OF(ADRSDIF1)));
}

static unsigned by_adrsdif2(unsigned address)
{
    return indexed(address, fetch(ADDRESS_OF(ADRSDIF2)));
}

/* INDEX AXISCTR: the axis's word of TJP, TJU, TJV (or of ACCSWU, -V). */
static unsigned by_axis(unsigned address)
{
    return indexed(address, fetch(ADDRESS_OF(AXISCTR)));
}

/* COASTTJ -- LM p. 1463: "SET TJET = 0." */
static bool coasttj(void)
{
    set_A(ZERO);
    return true;
}

/* FULLTIME -- LM p. 1464: "250 MSEC SCALED AT 4 SEC." */
static bool fulltime(void)
{
    set_A(BIT11);
    return true;
}

/* ---- RETURNTJ -- LM p. 1464 ---- */

/* RETURNTJ -- LM p. 1464: "ROTSENSE APPLIES SIGN AND CHANGES SCALE." The
 * time goes to TJP, TJU or TJV; then NUMBERT is four, "SET SWITCH FOR JET
 * SELECT IF ROTATION IS" "IN A SENSE FOR WHICH 1/ACCS HAS FORCED" "A MAX-JET
 * CALCULATION.", and "RETURN VIA SAVED Q." */
static void returntj(void)
{
    mp_at(ADDRESS_OF(ROTSENSE));
    ts(by_axis(ADDRESS_OF(TJETU)));
    mp_at(by_axis(ADDRESS_OF(ACCSWU)));
    ca(L_ADDRESS);
    if (!A_is_zero_or_minus()) {
        set_A(FOUR);
        ts(ADDRESS_OF(NUMBERT));
    }
    tc_through(ADDRESS_OF(HOLDQ));
}

/* ---- CHKMINTJ -- LM p. 1467 ---- */

/* CHKMINTJ -- LM p. 1467: "IF COMPUTED JET TIME IS LESS THAN TJMIN, TJET IS
 * SET TO ZERO." */
static bool chkmintj(void)
{
    ad(MINUS_TJMIN);
    if (A_is_zero_or_minus()) return coasttj(); /* "YES, SET TIME TO ZERO." */
    ad(TJMIN);                                  /* "NO, RESTORE COMPUTED TIME." */
    return true;
}

/* ---- Z123COMP -- LM p. 1465 ---- */

/* Z123COMP -- LM p. 1465, by TC at at: "SUBROUTINIZED COMPUTATIONS REQUIRED
 * FOR ALL ENTRIES INTO CODING FOR ZONES 1, 2, AND 3." ROTSENSE reversed, and
 * TTOAXIS, the time to the axis. Returns whether "TIME TO AXIS LESS THAN 150
 * MSEC." (RETURN); if not, the caller goes to FULLTIME: "FIRE JETS, DO NOT
 * CALCULATE TJET." */
static bool z123comp(word at)
{
    tc_at(at);
    cs(ADDRESS_OF(ROTSENSE));
    ts(ADDRESS_OF(ROTSENSE));
    ca(ADDRESS_OF(EDOT));
    mp_at(by_adrsdif2(ADDRESS_OF(1_ANET1)));
    ts(ADDRESS_OF(TTOAXIS)); /* "STORE TIME-TO-AXIS SCALED AT 4 SECONDS." */
    ad(MINUS_TJMAX);
    if (!A_is_zero_or_minus()) return false;
    tc_q();
    return true;
}

/* ---- ZONE1 -- LM p. 1466 ---- */

/* ZONE1 -- LM p. 1466, with FIREFCT in A: H, "DOUBLE PRECISION H SCALED AT 8
 * SEC(2).", then the jet time by one of two approximations, or FULLTIME. */
static bool zone1(void)
{
    su_at(by_adrsdif1(ADDRESS_OF(AXISDIST)));
    mp_at(by_adrsdif2(ADDRESS_OF(ACCFCTZ1)));
    ddoubl();
    ddoubl();
    dxch(ADDRESS_OF(HH));
    ca(ADDRESS_OF(TTOAXIS)); /* "TEST WHETHER TOTAL TIME REQUIRED GREATER THAN 150 MSEC:" */
    ad(MINUS_TJMAX);
    square();
    su_at(ADDRESS_OF(HH));
    if (A_is_zero_or_minus()) return fulltime(); /* "YES.  NEED NOT CALCULATE TJET." */
    ca(ADDRESS_OF(HH));
    ad(NEG2);
    if (!A_is_zero_or_minus()) {
        /* "FOR APPROXIMATION OVER MORE THAN 50 MSEC." */
        set_A(POINT1AT2);
        dxch(ADDRESS_OF(HH));
        dv_at(ADDRESS_OF(HH));
        ad_at(ADDRESS_OF(TTOAXIS));
        ad(POINT0375AT4);
        return true;
    }
    set_A(negate(MINUS_POINT025AT2)); /* FORMULA3 */
    dxch(ADDRESS_OF(HH));
    dv_at(ADDRESS_OF(HH));
    ad_at(ADDRESS_OF(TTOAXIS));
    return chkmintj();
}

/* ---- ZONE2,3 -- LM p. 1466 ---- */

/* ZONE2,3 -- LM p. 1466: "FIRE FOR AXIS OR, IF CLOSE, FIRE MINIMUM IMPULSE. 
 * IF ON
 * AXIS, COAST." */
static bool zone2_3(void)
{
    cs(ADDRESS_OF(ZONE3LIM));
    ad_at(ADDRESS_OF(TTOAXIS));
    if (!A_is_zero_or_minus()) {
        ca(ADDRESS_OF(TTOAXIS)); /* ZONE2: "FIRE TO AXIS." */
        return true;
    }
    switch (ccs_at(ADDRESS_OF(EDOT))) { /* ZONE3: "CHECK IF EDOT IS ZERO." */
    case POSITIVE: set_A(BIT6); break;  /* "FIRE A ONE-JET MINIMUM IMPULSE." */
    case IS_PLUS_ZERO: break;           /* "TJET = +0." */
    case NEGATIVE:                      /* "CANNOT BE BECAUSE NEG EDOT COMPLEMENTED." */
        tc_at(ZONE3 + 3u);
        continue_at(CCSHOLE);
        return false;
    case IS_MINUS_ZERO: break; /* "TJET = +0." */
    }
    return true;
}

/* ZON1,2,3 -- LM p. 1465: Z123COMP, then "IF THE (NEG) DISTANCE BEYOND
 * PARABOLA IS LESS THAN FLAT, USE SPECIAL" "LOGIC TO ACQUIRE MINIMUM IMPULSE
 * LIMIT CYCLE." */
static bool zon1_2_3(void)
{
    if (!z123comp(ZON1_2_3)) return fulltime();
    ca(ADDRESS_OF(FIREFCT));
    ad_at(ADDRESS_OF(FLAT));
    if (A_is_zero_or_minus()) return zone1(); /* "NOT IN SPECIAL ZONES." */
    return zone2_3();
}

/* ---- ZONE4 -- LM p. 1463 ---- */

/* DRIFT/ON -- LM p. 1463: "CAN TARGET STRIP OF AXIS BE REACHED FROM" "THIS
 * POINT IN THE PHASE PLANE?" If so, "CALCULATE TJET AS THOUGH IN ZONE 2" "OR
 * 3 AFTER COMPUTING REQUIRED VALUES." */
static bool drift_on(void)
{
    cs(by_adrsdif1(ADDRESS_OF(FIREDB)));
    double_A();
    ad_at(ADDRESS_OF(FIREFCT));
    if (!A_is_zero_or_minus()) return coasttj();
    if (!z123comp(DRIFT_ON + 010u)) return fulltime();
    return zone2_3();
}

/* ZONE4 -- LM p. 1463: "ZONE 4 IS THE COAST REGION.  HOWEVER, IF THE JETS
 * ARE ON AND
 * DRIVING TOWARD" the axis or the target parabola, "THEN THE THRUSTERS ARE
 * KEPT ON." JETSON's CCS FLAT asks "IS THIS DRIFTING OR POWERED FLIGHT?":
 * positive is drifting. For powered flight (+0) "CAN TARGET PARABOLA" "BE
 * REACHED FROM THIS POINT IN THE" "PHASE PLANE?" A negative FLAT enters
 * that test past its CS FIREFCT, with FLAT's magnitude less one in A; -0
 * past its INDEX too, so AXISDIST is read for the U axis. */
static bool zone4(void)
{
    cs(by_axis(ADDRESS_OF(TJETU)));
    mp_at(ADDRESS_OF(ROTSENSE));
    if (A_is_zero_or_minus()) return coasttj(); /* "NO.  COAST." */
    switch (ccs_at(ADDRESS_OF(FLAT))) {
    case POSITIVE: return drift_on();
    case IS_PLUS_ZERO:
        cs(ADDRESS_OF(FIREFCT));
        ad_at(by_adrsdif1(ADDRESS_OF(AXISDIST)));
        break;
    case NEGATIVE: ad_at(by_adrsdif1(ADDRESS_OF(AXISDIST))); break;
    case IS_MINUS_ZERO: ad_at(ADDRESS_OF(AXISDIST)); break;
    }
    if (A_is_zero_or_minus()) return coasttj(); /* "NO. SET TJET = 0." */
    if (!z123comp(JETSON + 7u)) return fulltime();
    ca(ADDRESS_OF(FIREFCT));
    return zone1();
}

/* ---- ZONE5 -- LM pp. 1463-1464 ---- */

/* ZONE5 -- LM p. 1463, with the function in A: TTOAXIS and H, "THE
 * PARAMETERS UPON" "WHICH THE APPROXIMATIONS TO TJET ARE" "ABASED.", then
 * the time "FOR TJET LESS THAN 50 MSEC." (FORMULA1), "FOR TJET GREATER THAN
 * 50 MSEC." (FORMULA2), or FULLTIME. CCS ROTSENSE moves ADRSDIF2 back two
 * "IF THE" "(ACTUAL) ERROR RATE IS NEGATIVE."; a TS L that overflows skips
 * it. */
static bool zone5(void)
{
    if (!ts(L_ADDRESS)) {
        switch (ccs_at(ADDRESS_OF(ROTSENSE))) {
        case POSITIVE: break; /* TCF +4 */
        case IS_PLUS_ZERO:
            tc_at(ZONE5 + 3u);
            continue_at(CCSHOLE);
            return false;
        case NEGATIVE:
            set_A(negate(TWO));
            ads(ADDRESS_OF(ADRSDIF2));
            break;
        case IS_MINUS_ZERO: ads(ADDRESS_OF(ADRSDIF2)); break;
        }
    }
    ca(L_ADDRESS); /* +4 */
    mp_at(by_adrsdif2(ADDRESS_OF(ACCFCTZ5)));
    ddoubl();
    ddoubl();
    dxch(ADDRESS_OF(HH));
    ca(by_adrsdif2(ADDRESS_OF(1_ANET2)));
    mp_at(ADDRESS_OF(EDOT));
    ts(ADDRESS_OF(TTOAXIS)); /* "SCALED AT 4 SEC." */
    mp(MINUS_POINT05AT2);    /* "TEST WHETHER TJET GREATER THAN 50 MSEC." */
    ad_at(ADDRESS_OF(HH));
    ad(NEG2);
    if (A_is_zero_or_minus()) {
        /* FORMULA1: "TJET = H/(.025 + TTOAXIS)" */
        set_A(negate(MINUS_POINT025AT4));
        ad_at(ADDRESS_OF(TTOAXIS));
        dxch(ADDRESS_OF(HH));
        dv_at(ADDRESS_OF(HH));
        mp(BIT14); /* "RESCALE TJET FROM 2 TO USUAL 4 SEC." */
        return chkmintj();
    }
    ca(ADDRESS_OF(TTOAXIS)); /* "TEST WHETHER TJET GREATER THAN 150 MSEC." */
    mp(MINUS_POINT15AT2);
    ad_at(ADDRESS_OF(HH));
    ad(MINUS_POINT0112A8);
    if (A_is_zero_or_minus()) {
        /* FORMULA2: "TJET = (H + .00375)/(0.1 + TTOAXIS)" */
        set_L(POINT00375A8[1]);
        set_A(POINT00375A8[0]);
        das(ADDRESS_OF(HH));
        ca(ADDRESS_OF(TTOAXIS));
        ad(POINT1AT4);
        dxch(ADDRESS_OF(HH));
        dv_at(ADDRESS_OF(HH));
        mp(BIT14);
        return true;
    }
    return fulltime(); /* "ASSIGN IT VALUE OF 250 MSEC" */
}

/* ---- The fine law -- LM pp. 1462-1463 ---- */

/* SENSTEST: CCS SENSETYP, "DOES TRANSLATION PREFER MIN JETS." Positive or
 * negative: "YES.  USE MIN-JET PARAMETERS" (TJCALC); +0: "NO.  GET THE
 * MAX-JET PARAMETERS."; -0 lands past TJCALC's TS NUMBERT. */
enum jets { MIN_JETS, MAX_JETS, NUMBERT_KEPT };

static enum jets senstest(void)
{
    switch (ccs_at(ADDRESS_OF(SENSETYP))) {
    case POSITIVE: return MIN_JETS; /* TCF TJCALC */
    case IS_PLUS_ZERO: return MAX_JETS;
    case NEGATIVE: return MIN_JETS; /* TJCALC is the next word */
    case IS_MINUS_ZERO: break;
    }
    return NUMBERT_KEPT;
}

/* From SCALEDOT ("*** FINELAW STARTS HERE ***"), with E rescaled: EDOTSQ;
 * ERRTEST, "DOES BIG ERROR (THREE DEG BEYOND THE" "DEADBAND) REQUIRE
 * MAXIMUM JETS?"; MAXJETS ("ALWAYS DO THIS FOR P-AXIS"); then FIREFCT,
 * "-E-.5(EDOTSQ)/ACC-DB AT PI/4 RADIAN.", which picks the zone. A TS
 * FIREFCT that overflows skips the EXTEND of BZMF ZON1,2,3, which then
 * runs as AD of its own address: of ZON1,2,3's word, TC Z123COMP. */
static bool finelaw(void)
{
    enum jets jets = MAX_JETS;

    lxch(ADDRESS_OF(EDOT)); /* "EDOT IS SCALED AT PI/32 RADIANS/SECOND." */
    ca(ADDRESS_OF(EDOT));
    square();
    mp(BIT13);
    ts(ADDRESS_OF(EDOTSQ));
    switch (ccs_at(ADDRESS_OF(E))) { /* ERRTEST */
    case POSITIVE: ad(MINUS_3DEG); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: ad(MINUS_3DEG); break;
    case IS_MINUS_ZERO: break;
    }
    su_at(by_adrsdif1(ADDRESS_OF(FIREDB)));
    if (A_is_zero_or_minus()) jets = senstest();
    if (jets == MAX_JETS) {
        set_A(TWO); /* MAXJETS */
        ads(ADDRESS_OF(ADRSDIF2));
        set_A(FOUR);
    }
    if (jets != NUMBERT_KEPT) ts(ADDRESS_OF(NUMBERT)); /* TJCALC */
    cs(ADDRESS_OF(EDOTSQ));                            /* "BEGINNING OF TJET CALCULATIONS:" */
    mp_at(by_adrsdif2(ADDRESS_OF(1_ANET1)));
    ad_at(by_adrsdif1(ADDRESS_OF(FIREDB)));
    su_at(ADDRESS_OF(E));
    if (ts(ADDRESS_OF(FIREFCT))) {
        ad(fetch_fixed(ZON1_2_3));
    } else if (A_is_zero_or_minus()) {
        return zon1_2_3();
    }
    /* ZONE4,5: "TEST E+.5(EDOTSQ)/ACC+DB AT PI/4 RADIAN." */
    ca(by_adrsdif1(ADDRESS_OF(1_ACOAST)));
    mp_at(ADDRESS_OF(EDOTSQ));
    ad_at(ADDRESS_OF(E));
    ad_at(by_adrsdif1(ADDRESS_OF(COASTDB)));
    if (A_is_zero_or_minus()) return zone5(); /* "IF FUNCTION NEGATIVE, FIND TJET." */
    return zone4();                           /* "IF FUNCTION POSITIVE, IN ZONE 4." */
}

/* ---- The rough laws -- LM pp. 1468-1469 ---- */

/* RUFSETUP -- LM p. 1469, by TC at at: "SUBROUTINE USED IN ALL ENTRIES TO
 * ROUGHLAW." "REVERSE ROTSENSE WHEN ENTER HERE." (RUFSETUP +2, from
 * SMALRATE, does not), then "REQUIRE MAXIMUM (2) JETS IN U,V-AXES." and
 * "SUGGEST MAXIMUM (4) JETS IN P-AXIS." */
static void rufsetup(word at, bool reverse)
{
    tc_at(at);
    if (reverse) {
        cs(ADDRESS_OF(ROTSENSE));
        ts(ADDRESS_OF(ROTSENSE));
    }
    set_A(FOUR);
    ts(ADDRESS_OF(NUMBERT));
    set_A(NEGMAX);
    ts(ADDRESS_OF(FIREFCT));
    tc_q();
}

/* RUFLAW12 -- LM p. 1468, with the desired rate change in A: "COMPUTE TJET"
 * "= (DESIRED RATE CHANGE)/(2-JET ACCEL.)"; "IF TJET, SCALED AT 32 SEC,
 * EXCEEDS" "4 SECONDS, SET TJET TO TJMAX." */
static bool ruflaw12(void)
{
    mp_at(by_adrsdif2(ADDRESS_OF(1_ANET1) + 2u));
    ad(MINUS_1_8);
    if (!A_is_zero_or_minus()) return fulltime();
    if (A_is_zero()) return fulltime();
    ad(BIT12); /* "RESTORE COMPUTED TJET TO ACCUMULATOR" */
    ddoubl();
    ddoubl();
    ddoubl();          /* "RESCALED TJET AT 4 SECONDS." */
    return chkmintj(); /* "RETURN AS FROM FINELAW." */
}

/* RUFLAW1 -- LM p. 1468: "ERROR MORE NEGATIVE THAN PI/16 RAD.  FIRE TO A
 * RATE OF 6.5
 * DEG/SEC (IF JET TIME EXCEEDS 20 MSEC.)." SMALRATE: "MODIFY INDEXER TO
 * POINT TO 1/ANET" "CORRESPONDING TO THE PROPER SENSE." */
static bool ruflaw1(void)
{
    set_A(negate(RUFRATE));
    ads(ADDRESS_OF(EDOT));
    if (!A_is_zero_or_minus()) {
        rufsetup(RUFLAW1 + 4u, true);
        ca(ADDRESS_OF(EDOT)); /* "PICK UP DESIRED RATE CHANGE." */
        return ruflaw12();
    }
    rufsetup(SMALRATE, false); /* "BRANCH IF RATE LESS THAN TARGET." */
    switch (ccs_at(ADDRESS_OF(ROTSENSE))) {
    case POSITIVE: set_A(ONE); break;
    case IS_PLUS_ZERO: break; /* ADS of +0 */
    case NEGATIVE: set_A(NEGONE); break;
    case IS_MINUS_ZERO: break; /* ADS of +0 */
    }
    ads(ADDRESS_OF(ADRSDIF2));
    cs(ADDRESS_OF(EDOT)); /* "(.144 AT PI/4 - EDOT) = DESIRED RATE CHNG." */
    return ruflaw12();
}

/* RUFLAW2 -- LM p. 1468: "ERROR MORE POSITIVE THAN PI/16 RAD.  FIRE TO AN
 * OPPOSING RATE
 * OF 6.5 DEG/SEC." */
static bool ruflaw2(void)
{
    rufsetup(RUFLAW2, true);
    set_A(RUFRATE);
    ad_at(ADDRESS_OF(EDOT));
    if (ts(A_ADDRESS)) return fulltime(); /* "IF OVERFLOW SKIP, FIRE FOR FULL TIME." */
    return ruflaw12();
}

/* RUFLAW3 -- LM p. 1469: "ERROR RATE GREATER THAN PI/32 RAD/SEC AND ERROR
 * WITHIN BOUNDS. COAST IF BELOW FIREFCT, FIRE IF ABOVE" */
static bool ruflaw3(void)
{
    rufsetup(RUFLAW3, true);
    cs(by_adrsdif1(ADDRESS_OF(FIREDB))); /* "CALCULATE DISTANCE FROM SWITCH CURVE" */
    ad_at(ADDRESS_OF(E));
    mp(BIT11);
    xch(ADDRESS_OF(EDOT));
    square();
    mp_at(by_adrsdif1(ADDRESS_OF(1_ANET1) + 2u));
    ad_at(ADDRESS_OF(EDOT));
    if (A_is_zero_or_minus()) return coasttj(); /* "COAST IF BELOW IT." */
    return fulltime();                          /* "FIRE FOR FULL PERIOD IF ABOVE IT." */
}

/* ---- TJETLAW -- LM pp. 1461-1462 ---- */

/* The jet time, by the law for the size of E and EDOT. "TEST MAGNITUDE OF
 * E": MP BIT5 shifts it "RIGHT TEN BITS:  IF A-REGISTER IS" "ZERO, RESCALE
 * AND TEST EDOT." (SCALEE; -0 too); otherwise RUFLAW2 or RUFLAW1 by its
 * sign. Then "TEST MAGNITUDE OF EDOT": if MP BIT4 leaves A nonzero,
 * RUFLAW3, else the fine law. */
static bool jet_time(void)
{
    ca(ADDRESS_OF(E));
    mp(BIT5);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: return ruflaw2();
    case IS_PLUS_ZERO: break;
    case NEGATIVE: return ruflaw1();
    case IS_MINUS_ZERO: break;
    }
    set_A(BIT13); /* SCALEE: "ERROR IS IN L SCALED AT PI/16." */
    mp_at(L_ADDRESS);
    ts(ADDRESS_OF(E));
    ca(ADDRESS_OF(EDOT));
    mp(BIT4);
    if (!A_is_zero()) return ruflaw3();
    return finelaw();
}

/* TJETLAW -- LM p. 1461: "SAVE Q FOR RETURN.", then "SET INDEXERS TO
 * CORRESPOND TO THE AXIS AND TO THE SIGN OF EDOT". AXISDIFF holds the
 * distance from the U axis's 1/ACCS parameters to this axis's. For a
 * negative EDOT (NEGEDOT) E and EDOT are complemented, the indexers moved on
 * one and ROTSENSE made negative: "IN ORDER FOR NEG EDOT CASE TO USE CODING"
 * "OF POS EDOT". (A TS ADRSDIF2 that overflowed would skip the CS
 * SENSOR.) */
void tjetlaw(void)
{
    qxch(ADDRESS_OF(HOLDQ));
    set_A(fetch_fixed(by_axis(AXISDIFF)));
    ts(ADDRESS_OF(ADRSDIF1));
    ca(ADDRESS_OF(EDOT));
    if (!A_is_zero_or_minus()) {
        ca(ADDRESS_OF(ADRSDIF1));
        ts(ADDRESS_OF(ADRSDIF2));
        set_A(SENSOR); /* "FOR POSITIVE EDOT, ROTSENSE IS" "INITIALIZED POSITIVE." */
    } else {
        cs(ADDRESS_OF(E));
        ts(ADDRESS_OF(E));
        cs(ADDRESS_OF(EDOT));
        ts(ADDRESS_OF(EDOT));
        set_A(BIT1);
        ads(ADDRESS_OF(ADRSDIF1));
        if (!ts(ADDRESS_OF(ADRSDIF2))) set_A(negate(SENSOR));
    }
    ts(ADDRESS_OF(ROTSENSE)); /* SETSENSE */
    if (jet_time()) returntj();
}

#endif
