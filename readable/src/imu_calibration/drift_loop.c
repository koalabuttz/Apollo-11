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
 * ERTHRVSE, the filter) the faithful port runs. */
#include "imu_calibration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

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
static void sleepie_plus1(void)
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
static void annnnnn(void)
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
