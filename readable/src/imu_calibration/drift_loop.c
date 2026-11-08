/* SPDX-License-Identifier: MIT */
/* IMU_CALIBRATION_AND_ALIGNMENT.agc of the CM, pages 430-438, and the
 * small routines of pages 442 and 446-447 they call: the earth rate
 * compensation (EARTHR*), the display of a result (SHOW), the scaler's time
 * (FINETIME), and the drift test's loop: ESTIMS starts it, ALLOOP, a
 * waitlist task each second, reads the PIPAs and starts ALFLT, the job
 * that runs the filter (NORMLOP, in the interpreter). The same loop runs
 * the prelaunch alignment (gyrocompassing, GEOCOMP1 nonzero), which
 * CHKCOMED ends at liftoff: then P11. The faithful C is
 * c64-port/src/cm_imu_calibration.c.
 *
 * Every word is in bank 33 but FINETIME's, in fixed-fixed. The erasable
 * words are EBANK 5's. The interpretive code among the basic (EARTHR,
 * ERTHRVSE, the filter) is written in the listing's notation: tables the
 * readable interpreter runs (src/interpreter/notation.h). ERTHRVSE is in
 * bank 32. */
#include "imu_calibration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- ERTHRVSE and EARTHR -- p. 430 ---- */

/* ERTHRVSE -- p. 430, in bank 32. The earth rate vector in stable member
 * axes, "PD24 = (SIN -COS 0)(OMEG/MS)": with X up and Z north (at the
 * site, azimuth aside) the earth turns about (sin LAT, -cos LAT, 0), at
 * OMEG/MS. Into ERVECTOR; TMARK the time now (RTB LOADTIME), ERCOMP1
 * cleared. */
static const word erthrvse_words[] = {
    ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE(SCHZEROS),
    ADDRESS_PLUS_ONE(LATITUDE),
    ORDERS(COS, DCOMP),
    ORDERS(PDDL, SIN),
    ADDRESS_PLUS_ONE(LATITUDE),
    ORDERS(VDEF, VXSC),
    ADDRESS_PLUS_ONE(OMEG_MS),
    STORE(ERVECTOR),
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STOVL(TMARK),
    ADDRESS_PLUS_ONE(SCHZEROS),
    STORE(ERCOMP1),
    ORDER(RVQ),
};

const struct interpretive_code ERTHRVSE_CODE = {032u, ERTHRVSE, COUNT(erthrvse_words),
                                                erthrvse_words};

/* EARTHR -- p. 430. "CALCULATES AND COMPENSATES EARTH RATE" since TMARK:
 * the time since (TEMPTIME less TMARK, OVERFFIX if the clock went round),
 * times ERVECTOR, through XSM (the stable member's matrix), added into
 * ERCOMP1; then PULSEIMU torques the gyros by it (AXT,1 its ECADR). ITA
 * S2 keeps the return, GOTO S2 goes back. */
static const word earthr_words[] = {
    ORDERS(ITA, RTB),
    ADDRESS(S2),
    ADDRESS(LOADTIME),
    STORE(TEMPTIME),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(TMARK),
    ADDRESS(ERTHR),
    ORDER(CALL),
    ADDRESS(OVERFFIX),
    /* ERTHR */ ORDERS(SL, VXSC),
    SL_COUNT(9),
    ADDRESS_PLUS_ONE(ERVECTOR),
    ORDERS(MXV, VAD),
    ADDRESS_PLUS_ONE(XSM),
    ADDRESS_PLUS_ONE(ERCOMP1),
    STODL(ERCOMP1),
    ADDRESS_PLUS_ONE(TEMPTIME),
    STORE(TMARK),
    ORDERS(AXT_1, RTB),
    ADDRESS(ERCOMP1), /* ECADR ERCOMP1 */
    ADDRESS(PULSEIMU),
    ORDER(GOTO),
    ADDRESS(S2),
};

const struct interpretive_code EARTHR_CODE = {033u, EARTHR, COUNT(earthr_words), earthr_words};

/* ---- EARTHR* and SHOW -- pp. 430-431 ---- */

/* EARTHR* -- p. 430. EARTHR from basic code: the return into QPLACES,
 * and EARTHR, in the interpreter, torques the gyros by the earth's
 * rotation since TMARK (ERCOMP1, PULSEIMU). */
void earthr_star(void)
{
    qxch(E(QPLACES));
    tc_at(EARTHR_STAR + 2u);
    intpret();
}

/* EARTHR* +3 -- p. 430. */
static const word earthr_star_words[] = {
    ORDER(CALL),
    ADDRESS(EARTHR),
    /* PROUT */ ORDER(EXIT),
};

const struct interpretive_code EARTHR_STAR_CODE = {033u, EARTHR_STAR + 3u, COUNT(earthr_star_words),
                                                   earthr_star_words};

/* PROUT +1 -- p. 430, EARTHR's EXIT: wait for the gyros (IMUSTLLG). */
void prout_after_exit(void)
{
    tc_at(PROUT + 1u);
    imustllg();
}

/* PROUT +2: back to the caller, through QPLACES. */
void prout_after_imustllg(void)
{
    tc_return(E(QPLACES));
}

/* SHOW1 -- p. 431. DSPTEM2 +2 the position; flash V06 N98 (DSPTEM2: the
 * result) until the crew answers. */
static void show1(void)
{
    ca(E(POSITON));
    ts(E(DSPTEM2) + 2u);
    set_A(VB06N98);
    tc_at(SHOW1 + 3u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* SHOW -- p. 431. The return into QPLACE, and SHOW1. */
void show(void)
{
    qxch(E(QPLACE));
    show1();
}

/* SHOW +7: "V 34", terminate: the test ends. */
void show_after_terminate(void)
{
    tc_at(SHOW1 + 5u);
    endtest1();
}

/* SHOW +010: "V33", proceed: back to the caller, through QPLACE. */
void show_after_proceed(void)
{
    tc_return(E(QPLACE));
}

/* SHOW +011: a load (enter): show it again. TCF SHOW1. */
void show_after_enter(void)
{
    show1();
}

/* P11OUT -- p. 431. Back to P11, at MATRXJOB. */
void p11out(void)
{
    tc_at(P11OUT);
    continue_at(BANKCALL); /* CADR MATRXJOB */
}

/* ---- FINETIME -- p. 431, in fixed-fixed ---- */

/* FINETIME. The time now: the scaler's low part (LOSCALAR) in L, and its
 * high part (HISCALAR) in A. "RETURNS WITH INTERRUPT INHIBITED". */
void finetime(void)
{
    inhint();
    continue_at(FINETIME + 1u);
}

/* FINETIME +1. LOSCALAR read twice: if the two disagree (it moved between
 * them), once more. At 37777 the low part is about to carry into the high:
 * read it all again (the AGC waits here for the scaler; the binding stops
 * the run). Then HISCALAR, and back through Q. */
void finetime_plus1(void)
{
    read_channel(LOSCALAR);
    ts(L_ADDRESS);
    rxor_channel(LOSCALAR);
    if (!A_is_zero()) { /* EXTEND; BZF +4 */
        read_channel(LOSCALAR);
        ts(L_ADDRESS);
    }
    set_A(negate(POSMAX)); /* +4: CS POSMAX; AD L */
    ad_at(L_ADDRESS);
    if (A_is_zero()) { /* EXTEND; BZF FINETIME +1 */
        continue_at(FINETIME + 1u);
        return;
    }
    read_channel(HISCALAR);
    tc_q();
}

/* ---- ESTIMS: the drift test's start -- pp. 434-435 ---- */

/* ESTIMS -- p. 434. "COMES HERE FROM IMU2": two restart phase changes
 * (2PHSCHNG: OCT 00075, and OCT 00004, "TURN OFF GROUP 4 IF ON"), which
 * come back to RSTGTS1. */
void estims(void)
{
    tc_at(ESTIMS);
    continue_at(TWO_PHSCHNG); /* OCT 00075; OCT 00004 */
}

/* RSTGTS1 -- p. 434. "COMES HERE PHASE1 RESTART": GTSWTLT1 the time now
 * (ALLOOP's next run is a second on); "ZERO THE PIPAS"; then "ZERO ALL
 * NECESSARY LOCATIONS": the filter's 78 words from ALX1S on (77DECML, the
 * count less one; ALXXXZ, the address). */
void rstgts1(void)
{
    inhint();
    ca(ADDRESS_OF(TIME1));
    ts(E(GTSWTLT1));
    set_A(ZERO);
    ts(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(PIPAY));
    ts(ADDRESS_OF(PIPAZ));
    relint();
    set_A(DEC77);
    ts(E(ZERONDX1));
    set_A(ALXXXZ);
    tc_at(RSTGTS1 + 013u);
    zeroing();
}

/* RSTGTS1 +014 -- p. 434. Into the interpreter: GCOMPSW, "SOME INITIAL
 * DRIFT GAINS" (INTVAL +2 into ALX1S), GCOMP and DELVX cleared. */
void rstgts1_after_zeroing(void)
{
    tc_at(RSTGTS1 + 014u);
    intpret();
}

/* RSTGTS1 +015 -- p. 434. GCOMPSW (the word before it too, from the SLOAD:
 * STOVL GCOMPSW -1) cleared; from INTVAL +2, ALX1S and CMPX1 and the
 * first gains ("LOAD SOME INITIAL DRIFT GAINS"); GCOMP and DELVX cleared
 * ("GCOMPZER SUBROUTINE NO LONGER NEEDED"). */
static const word rstgts1_words[] = {
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(SCHZEROS),
    (word)(STOVL(GCOMPSW) - 1u), /* STOVL GCOMPSW -1 */
    ADDRESS_PLUS_ONE_AT(INTVAL, 2),
    STOVL(ALX1S),
    ADDRESS_PLUS_ONE(SCHZEROS),
    STORE(GCOMP),
    STORE(DELVX),
    ORDER(EXIT),
};

const struct interpretive_code RSTGTS1_CODE = {033u, RSTGTS1 + 015u, COUNT(rstgts1_words),
                                               rstgts1_words};

/* ESTIMS +031 -- p. 434, the EXIT. GEOCOMP1: "NON ZERO IF COMPASS" (the
 * prelaunch alignment): then the earth rate vector first (ERTHRVSE, in the
 * interpreter); +0 (the drift test) to SLEEPIE +1. (-0 would run the CALL
 * after the TC INTPRET, a data word: the faithful port does not.) */
void estims_after_exit(void)
{
    switch (ccs_at(E(GEOCOMP1))) {
    case POSITIVE:
    case NEGATIVE: estims_compass(); return; /* TC +2 */
    case IS_PLUS_ZERO: estims_drift_test(); return;
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* SLEEPIE +1 -- p. 436. "CHANGE PHASE" (PHASCHNG, OCT 00135). */
void sleepie_plus1(void)
{
    tc_at(SLEEPIE + 1u);
    continue_at(PHASCHNG); /* OCT 00135 */
}

/* ESTIMS +033: the drift test. */
void estims_drift_test(void)
{
    tc_at(ESTIMS + 033u);
    sleepie_plus1();
}

/* ESTIMS +034: gyrocompassing: ERTHRVSE in the interpreter. */
void estims_compass(void)
{
    tc_at(ESTIMS + 034u);
    intpret();
}

/* ESTIMS +035 -- p. 434. */
static const word estims_words[] = {
    ORDER(CALL),
    ADDRESS(ERTHRVSE),
    ORDER(EXIT),
};

const struct interpretive_code ESTIMS_CODE = {033u, ESTIMS + 035u, COUNT(estims_words),
                                              estims_words};

/* ESTIMS +040 -- p. 435, ERTHRVSE's EXIT. ERECTIME the time to erect the
 * platform ("TIMES FIVE IS THE NUM OF SEC ERECTING"); the major mode P02. */
void estims_after_erthrvse(void)
{
    ca(E(LENGTHOT));
    ts(E(ERECTIME));
    tc_at(ESTIMS + 042u);
    continue_at(NEWMODEX); /* MM 02 */
}

/* ESTIMS +044: "SET UP PIPA FAIL TO CAUSE ISS ALARM" (PIPUSE; "COMPASS
 * NEVER TURNS THIS OFF"). */
void estims_after_newmodex(void)
{
    tc_at(ESTIMS + 044u);
    continue_at(BANKCALL); /* CADR PIPUSE */
}

/* ANNNNNN -- p. 446. LENGTHOT nine, and SLEEPIE +1. */
void annnnnn(void)
{
    set_A(NINE);
    ts(E(LENGTHOT));
    tc_at(ANNNNNN + 2u);
    sleepie_plus1();
}

/* ESTIMS +046: "END OF FIRST TIME THROUGH". */
void estims_after_pipuse(void)
{
    tc_at(ESTIMS + 046u);
    annnnnn();
}

/* ---- SLEEPIE and the waitlist -- p. 436 ---- */

/* SLEEPIE -- p. 436. "TEST NOT OVER-DECREMENT LENGTHOT" (a TS that
 * overflows would skip to the OCT after the TC: not modelled), then
 * SLEEPIE +1. */
void sleepie(void)
{
    if (ts(E(LENGTHOT))) not_modelled();
    sleepie_plus1();
}

/* SLEEPIE +3. "ARE WE DOING VERTDRIFT": TORQNDX positive, earth rate
 * compensation first (EARTHR*, which comes back to WTLISTNT). Then the
 * next ALLOOP; -0 ends the job without it. */
void sleepie_after_phaschng(void)
{
    switch (ccs_at(E(TORQNDX))) {
    case POSITIVE:
        tc_at(SLEEPIE + 4u);
        earthr_star(); /* "TRUE TORQUE SOUTH GYRO" */
        return;
    case IS_PLUS_ZERO: wtlistnt(); return;
    case NEGATIVE: wtlistnt_after_chkcomed(); return;
    case IS_MINUS_ZERO: wtlistnt_after_setgwlst(); return;
    }
}

/* WTLISTNT -- p. 436. "SEE IF COMPASS OVER" (CHKCOMED). */
void wtlistnt(void)
{
    tc_at(WTLISTNT);
    chkcomed();
}

/* WTGTSMPL -- p. 436. ALLOOP in A centiseconds (TWIDDLE). */
static void wtgtsmpl(void)
{
    tc_at(WTGTSMPL);
    continue_at(TWIDDLE); /* ADRES ALLOOP */
}

/* SETGWLST -- p. 436. "CALLED EVERY WAITLIST OR AZIMUTH CHANGE": the next
 * ALLOOP a second after the last (GTSWTLT1 + 1SECXT1, "1 SEC FOR
 * CALIBRATION, .5 SEC IN COMPASS"), from now; the time since the last taken
 * past the scaler's turn (NEGMAX) when it is positive. When that time has
 * gone ("10 MS ERROR OK"), in 40 ms (RIGHTGTS). The return into MPAC. */
static void setgwlst(void)
{
    qxch(ADDRESS_OF(MPAC));
    inhint();
    cs(ADDRESS_OF(TIME1));
    ad_at(E(GTSWTLT1));
    if (!A_is_zero_or_minus()) ad(NEGMAX); /* EXTEND; BZMF +2 */
    ad_at(E(1SECXT1));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF RIGHTGTS */
        set_A(FOUR);            /* RIGHTGTS */
        tc_at(RIGHTGTS + 1u);
    }
    wtgtsmpl();
}

/* WTLISTNT +1: set up the next ALLOOP (SETGWLST). */
void wtlistnt_after_chkcomed(void)
{
    tc_at(WTLISTNT + 1u);
    setgwlst();
}

/* WTLISTNT +2: this job ends. */
void wtlistnt_after_setgwlst(void)
{
    tc_at(WTLISTNT + 2u);
    continue_at(ENDOFJOB);
}

/* WTGTSMPL +2: back through MPAC. */
void wtgtsmpl_after_twiddle(void)
{
    tc_return(ADDRESS_OF(MPAC));
}

/* ---- STOREDTA and LOADSTDT -- p. 436 ---- */

/* The words a restart keeps (THETAX1 to RESTARPT): 26 of them, from
 * MPAC = 25DECML down. INDEX MPAC, both ways. */
static unsigned kept_word(enum indexed_use use)
{
    return indexed_erasable(E(THETAX1), fetch(ADDRESS_OF(MPAC)), use);
}

static unsigned restart_copy(enum indexed_use use)
{
    return indexed_erasable(E(RESTARPT), fetch(ADDRESS_OF(MPAC)), use);
}

static void alloop_plus1(void);

/* LOADSTDT +1 -- p. 436. The words back from RESTARPT into THETAX1 on;
 * return through Q. (The count, MPAC, is never negative: were it, the run
 * would go on into ALLOOP, as the faithful port's does.) */
static void loadstdt_plus1(void)
{
    for (;;) {
        ts(ADDRESS_OF(MPAC));
        ca(restart_copy(INDEXED_READ));
        ts(kept_word(INDEXED_STORE));
        switch (ccs_at(ADDRESS_OF(MPAC))) {
        case POSITIVE: continue; /* TCF LOADSTDT +1 */
        case IS_PLUS_ZERO: tc_q(); return;
        case NEGATIVE: alloop(); return;
        case IS_MINUS_ZERO: alloop_plus1(); return;
        }
    }
}

/* LOADSTDT -- p. 436. "COMES HERE ON RESTART". */
static void loadstdt(void)
{
    set_A(DEC25);
    loadstdt_plus1();
}

/* STOREDTA -- p. 436. "STORE DATA IN CASE OF RESTART IN JOB": THETAX1 on
 * into RESTARPT on; return through Q. (Into LOADSTDT on a negative count,
 * as the faithful port.) */
void storedta(void)
{
    set_A(DEC25);
    for (;;) {
        ts(ADDRESS_OF(MPAC));
        ca(kept_word(INDEXED_READ));
        ts(restart_copy(INDEXED_STORE));
        switch (ccs_at(ADDRESS_OF(MPAC))) {
        case POSITIVE: continue; /* TCF STOREDTA +1 */
        case IS_PLUS_ZERO: tc_q(); return;
        case NEGATIVE: loadstdt(); return;
        case IS_MINUS_ZERO: loadstdt_plus1(); return;
        }
    }
}

/* ---- ALLOOP, the task each second -- p. 437 ---- */

/* ALLOOP -- p. 437. "STORE TIME TO SET UP NEXT WAITLIST" (GTSWTLT1); ALTIM
 * kept in GEOSAVE1 for a restart; a phase change (PHASCHNG, OCT 00115). */
void alloop(void)
{
    ca(ADDRESS_OF(TIME1));
    alloop_plus1();
}

static void alloop_plus1(void)
{
    ts(E(GTSWTLT1));
    ca(E(ALTIM)); /* ALLOOP3 */
    ts(E(GEOSAVE1));
    tc_at(ALLOOP + 4u);
    continue_at(PHASCHNG); /* OCT 00115 */
}

/* ALLOOP1 -- p. 437. ALTIM back from GEOSAVE1, and one less in magnitude
 * (CCS): its sign into ALTIMS, the magnitude, negated, into ALTIM. ("SHOULD
 * NEVER HIT THIS LOCATION": ALTIM positive.) The PIPAs read and cleared at
 * once (XCH) into DELVX, DELVY and DELVZ; a new phase, 19DECML, for
 * restart group 5 (NEWPHASE, OCT 00005). */
void alloop1(void)
{
    ca(E(GEOSAVE1));
    ts(E(ALTIM));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        ca(A_ADDRESS);
        ts(E(ALTIMS));
        cs(A_ADDRESS);
        ts(E(ALTIM));
        break;
    case IS_PLUS_ZERO:
        ts(E(ALTIMS));
        cs(A_ADDRESS);
        ts(E(ALTIM));
        break;
    case NEGATIVE:
        cs(A_ADDRESS);
        ts(E(ALTIM));
        break;
    case IS_MINUS_ZERO: ts(E(ALTIM)); break;
    }
    set_A(ZERO);
    xch(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(DELVX));
    set_A(ZERO);
    xch(ADDRESS_OF(PIPAY));
    ts(ADDRESS_OF(DELVY));
    set_A(ZERO);
    xch(ADDRESS_OF(PIPAZ));
    ts(ADDRESS_OF(DELVZ));
    set_A(DEC19);
    tc_at(ALLOOP1 + 021u);
    continue_at(NEWPHASE); /* OCT 00005 */
}

/* SPECSTS -- p. 437. "START THE JOB" ALFLT; the task ends. */
void specsts(void)
{
    set_A(PRIO22);
    tc_at(SPECSTS + 1u);
    continue_at(FINDVAC); /* 2CADR ALFLT */
}

/* SPECSTS +4. */
void specsts_after_findvac(void)
{
    tc_at(SPECSTS + 4u);
    continue_at(TASKOVER);
}

/* ---- ALFLT, the job each second -- p. 438 ---- */

/* ALFLT -- p. 438. "THIS IS THE JOB DONE EVERY ITERATION": keep its words
 * for a restart (STOREDTA). */
void alflt(void)
{
    tc_at(ALFLT);
    storedta();
}

/* ALFLT +1: a phase change (PHASCHNG, OCT 00215). */
void alflt_after_storedta(void)
{
    tc_at(ALFLT + 1u);
    continue_at(PHASCHNG); /* OCT 00215 */
}

/* ALFLT +3: TCF +2. */
void alflt_after_phaschng(void)
{
    alflt1_after_loadstdt();
}

/* ALFLT1 -- p. 438. "COMES HERE ON RESTART": the words back (LOADSTDT). */
void alflt1(void)
{
    tc_at(ALFLT1);
    loadstdt();
}

/* ALFLT1 +1. GEOCOMP1: +0, the drift test's filter (NORMLOP); otherwise
 * gyrocompassing: "SEE IF PRELAUNCH OVER" (CHKCOMED) and the PIPA
 * compensation (1/PIPA) first. */
void alflt1_after_loadstdt(void)
{
    switch (ccs_at(E(GEOCOMP1))) {
    case POSITIVE:
    case NEGATIVE: alflt1_compass(); return; /* TC +2 */
    case IS_PLUS_ZERO: alflt1_calibration(); return;
    case IS_MINUS_ZERO: alflt1_after_chkcomed(); return;
    }
}

/* ALFLT1 +3. */
void alflt1_calibration(void)
{
    tc_at(ALFLT1 + 3u);
    normlop();
}

/* ALFLT1 +4. */
void alflt1_compass(void)
{
    tc_at(ALFLT1 + 4u);
    chkcomed();
}

/* ALFLT1 +5: "COMPENSATION IF IN COMPASS". */
void alflt1_after_chkcomed(void)
{
    tc_at(ALFLT1 + 5u);
    continue_at(BANKCALL); /* CADR 1/PIPA */
}

/* NORMLOP -- p. 438. The filter, in the interpreter. */
void normlop(void)
{
    tc_at(NORMLOP);
    intpret();
}

/* NORMLOP +1 -- pp. 438-441. The filter, one pass a second:
 *   - S1 (the index step) from INTVAL; DELVX, the PIPA counts, through
 *     XSM (to stable member axes), doubled (VSL1); its Y and Z
 *     components, Y negated, into DPIPAY and DPIPAZ.
 *   - GEOCOMP1 positive (-GEOCOMP1 negative): gyrocompassing, ALWAYSG ("DO
 *     A QUICK COMPASS").
 *   - ALCGKK: when the gains are due (ALTIMS +0 or positive), ALKCG loads
 *     the next set: ALDK's words from ALFDK +144D (by X1 from ALX1S, which
 *     steps by -2 each word and is kept back in ALX1S), "LOADS SLOPES AND
 *     TIME CONSTANTS AT RQST".
 *   - ALFLT3, DELMLP: for each axis (X1 8, then 4: Y and Z), the
 *     measurement DELM: the PIPA count, scaled by PIPASC, less the
 *     integral INTY (shifted right 9: SLR BDSU), which takes its place;
 *     less the velocity VLAUN scaled (VELSC); stored twice, DELM and
 *     DELM +2.
 *   - ALILP: each gain ALK times its rate of change ALDK (X2 8, 4).
 *   - ALKLP: for each axis, the filter's update: INTY, ALK +12D and INTY
 *     +16D gain by gain, and VLAUN by the sway gain ALSK (X1 from CMPX1,
 *     +-1, the axis's sign).
 *   - LOOSE: "EXTRAPOLATE SWAY VARIABLES": ACCWD, VLAUN and POSNV (each
 *     axis's) through TRANSM1, and back.
 *   - BOOP: "EVALUATE SINES AND COSINES" of the angles ANGX.. (X1 2, X2 6:
 *     by GEORGEJ, 2/pi), into the push-down list: sines at 16D, cosines
 *     at 22D, by X2.
 *   - PERFERAS: EXIT. */
static const word normlop_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(INTVAL),
    STOVL(S1),
    ADDRESS_PLUS_ONE(DELVX),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(XSM),
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE_AT(MPAC, 3),
    STODL(DPIPAY),
    ADDRESS_PLUS_ONE_AT(MPAC, 5),
    STORE(DPIPAZ),
    ORDERS(SETPD, AXT_1),
    ADDRESS_PLUS_ONE_D(0),
    NUMBER(8),
    ORDERS(SLOAD, DCOMP),
    ADDRESS_PLUS_ONE(GEOCOMP1),
    ORDER(BMN),
    ADDRESS(ALWAYSG), /* "DO A QUICK COMPASS" */
    /* ALCGKK */ ORDERS(SLOAD, BMN),
    ADDRESS_PLUS_ONE(ALTIMS),
    ADDRESS(ALFLT3),                  /* "NO NEW GAINS NEEDED" */
    /* ALKCG */ ORDERS(AXT_2, LXA_1), /* "LOADS SLOPES AND TIME CONSTANTS AT RQST" */
    NUMBER(12),
    ADDRESS(ALX1S),
    /* ALKCG2 */ ORDERS(DLOAD_STAR, INCR_1),
    ADDRESS_PLUS_ONE_AT_X1(ALFDK, 144),
    MINUS_NUMBER(2), /* DEC -2 */
    STORE_AT_X2(ALDK, 10),
    ORDERS(TIX_2, SXA_1),
    ADDRESS(ALKCG2),
    ADDRESS(ALX1S),
    /* ALFLT3 */ ORDER(AXT_1), /* "MEASUREMENT INCORPORATION ROUTINES" */
    NUMBER(8),
    /* DELMLP */ ORDERS(DLOAD_STAR, DMP),
    ADDRESS_PLUS_ONE_AT_X1(DPIPAY, 8),
    ADDRESS_PLUS_ONE(PIPASC),
    ORDERS(SLR, BDSU_STAR),
    SLR_COUNT(9),
    ADDRESS_PLUS_ONE_AT_X1(INTY, 8),
    STORE_AT_X1(INTY, 8),
    ORDERS(PDDL, DMP_STAR),
    ADDRESS_PLUS_ONE(VELSC),
    ADDRESS_PLUS_ONE_AT_X1(VLAUN, 8),
    ORDER(SL2R),
    ORDERS(DSU, STADR),
    AFTER_STADR(STORE_AT_X1(DELM, 8)),
    STORE_AT_X1(DELM, 10),
    ORDERS(TIX_1, AXT_2),
    ADDRESS(DELMLP),
    NUMBER(4),
    /* ALILP */ ORDERS(DLOAD_STAR, DMPR_STAR),
    ADDRESS_PLUS_ONE_AT_X2(ALK, 4),
    ADDRESS_PLUS_ONE_AT_X2(ALDK, 4),
    STORE_AT_X2(ALK, 4),
    ORDERS(TIX_2, AXT_2),
    ADDRESS(ALILP),
    NUMBER(8),
    /* ALKLP */ ORDERS(LXC_1, SXA_1),
    ADDRESS(CMPX1),
    ADDRESS(CMPX1),
    ORDERS(DLOAD_STAR, DMPR_STAR),
    ADDRESS_PLUS_ONE_AT_X1(ALK, 1),
    ADDRESS_PLUS_ONE_AT_X2(DELM, 8),
    ORDER(DAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(INTY, 8),
    STORE_AT_X2(INTY, 8),
    ORDERS(DLOAD_STAR, DAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(ALK, 12),
    ADDRESS_PLUS_ONE_AT_X2(ALDK, 12),
    STORE_AT_X2(ALK, 12),
    ORDERS(DMPR_STAR, DAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(DELM, 8),
    ADDRESS_PLUS_ONE_AT_X2(INTY, 16),
    STORE_AT_X2(INTY, 16),
    ORDERS(DLOAD_STAR, DMP_STAR),
    ADDRESS_PLUS_ONE_AT_X1(ALSK, 1),
    ADDRESS_PLUS_ONE_AT_X2(DELM, 8),
    ORDERS(SL1R, DAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(VLAUN, 8),
    STORE_AT_X2(VLAUN, 8),
    ORDERS(TIX_2, AXT_1),
    ADDRESS(ALKLP),
    NUMBER(8),
    /* LOOSE */ ORDERS(DLOAD_STAR, PDDL_STAR), /* "EXTRAPOLATE SWAY VARIABLES" */
    ADDRESS_PLUS_ONE_AT_X1(ACCWD, 8),
    ADDRESS_PLUS_ONE_AT_X1(VLAUN, 8),
    ORDERS(PDDL_STAR, VDEF),
    ADDRESS_PLUS_ONE_AT_X1(POSNV, 8),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(TRANSM1),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(MPAC),
    STORE_AT_X1(POSNV, 8),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_AT(MPAC, 3),
    STORE_AT_X1(VLAUN, 8),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_AT(MPAC, 5),
    STORE_AT_X1(ACCWD, 8),
    ORDER(TIX_1),
    ADDRESS(LOOSE),
    ORDERS(AXT_2, AXT_1), /* "EVALUATE SINES AND COSINES" */
    NUMBER(6),
    NUMBER(2),
    /* BOOP */ ORDERS(DLOAD_STAR, DMPR),
    ADDRESS_PLUS_ONE_AT_X1(ANGX, 2),
    ADDRESS_PLUS_ONE(GEORGEJ),
    ORDER(SR2R),
    ORDERS(PUSH, SIN),
    ORDERS(SL3R, XAD_1),
    ADDRESS(X1),
    STORE_X2_D(16),
    ORDER(DLOAD), /* (the angle back from the push-down list) */
    ORDER(COS),
    STORE_X2_D(22), /* "COSINES" */
    ORDER(TIX_2),
    ADDRESS(BOOP),
    /* PERFERAS */ ORDER(EXIT),
};

const struct interpretive_code NORMLOP_CODE = {033u, NORMLOP + 1u, COUNT(normlop_words),
                                               normlop_words};

/* ---- SOMEERRR and SOMERR2 -- p. 442 ---- */

/* SOMEERRR -- p. 442. Alarm 1600, "OVERFLOW IN DRIFT TEST" (p. 432). */
void someerrr(void)
{
    tc_at(SOMEERRR);
    continue_at(ALARM); /* OCT 1600 */
}

/* SOMEERRR +2: on as SOMERR2's after its alarm. TC +3. */
void someerrr_after_alarm(void)
{
    tc_at(SOMEERRR + 2u);
    somerr2_after_alarm();
}

/* SOMERR2 -- p. 442. Alarm 1601, "BAD IMU TORQUE ABORT" (p. 433). */
void somerr2(void)
{
    tc_at(SOMERR2);
    continue_at(ALARM); /* OCT 1601 */
}

/* SOMERR2 +2: a phase change (PHASCHNG, OCT 00005), and the test ends. */
void somerr2_after_alarm(void)
{
    tc_at(SOMERR2 + 2u);
    continue_at(PHASCHNG); /* OCT 00005 */
}

/* SOMERR2 +4. */
void somerr2_after_phaschng(void)
{
    tc_at(SOMERR2 + 4u);
    endtest1();
}

/* ---- CHKCOMED -- pp. 446-447 ---- */

/* PRELTERM -- p. 447. "PRELAUNCH DONE - SET UP P11": this job's priority
 * "HIGHER THAN SERVICER" (PRIOCHNG). */
static void prelterm(void)
{
    set_A(PRIO22);
    tc_at(PRELTERM + 1u);
    continue_at(PRIOCHNG);
}

/* CHKCOMED -- p. 446. Is the prelaunch alignment over? In P07 (the
 * performance test) never: back through Q at once, interrupts still
 * inhibited. Otherwise over at liftoff (channel 30's bit 5, inverted) or
 * when the ground sent a backup liftoff (FLAGWRD5's GRRBKBIT): PRELTERM.
 * Not over: back through Q. */
void chkcomed(void)
{
    inhint();
    cs(ADDRESS_OF(MODREG));
    ad(SEVEN);
    if (A_is_zero()) { /* EXTEND; BZF GOBKCALB */
        tc_q();
        return;
    }
    set_A(negate(ZERO));
    rxor_channel(ADDRESS_OF(CHAN30));
    mask(BIT5);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* "LIFTOFF HAS OCCURRED" */
        prelterm();
        return;
    }
    set_A(GRRBKBIT);
    mask_at(ADDRESS_OF(FLAGWRD5));
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* "BACKUP RECEIVED" */
        prelterm();
        return;
    }
    relint();
    tc_q(); /* GOBKCALB */
}

/* PRELTERM +2 -- p. 447. On to P11 (POSTJUMP). */
void prelterm_after_priochng(void)
{
    inhint();
    tc_at(PRELTERM + 3u);
    continue_at(POSTJUMP); /* CADR P11 */
}

#endif
