/* SPDX-License-Identifier: MIT */
/* IMU_CALIBRATION_AND_ALIGNMENT.agc of the CM, pages 423-429: IMUTEST, the
 * IMU performance test the system test group ran with the CM on the pad,
 * its positioning (GEOIMUTT), the PIPA test (PIPACHK) and the vertical
 * drift test (VERTDRFT). The faithful C is c64-port/src/cm_imu_calibration.c.
 *
 * The test, in outline (the LM's P07 is much the same: src/imu_performance):
 *   - IMUTEST clears the drift test's flags (DRIFTT, GEOCOMP1), sets
 *     LENGTHOT, the test's length, coarse aligns the platform to +0
 *     (COAALIGN), and GUESS works out the earth rate at the site from the
 *     latitude (LATAZCHK lets the crew check it and the azimuth).
 *   - GEOIMUTT zeroes the IMU (IMUZERO) and NBPOSPL puts the platform's
 *     axes where the test wants them (XNB up, YNB and ZNB rotated by the
 *     azimuth from north); CALCGA finds the gimbal angles, IMUCOARS drives
 *     the platform there. Optical verification (P03) comes in here too.
 *   - In gimbal lock (GLOKFAIL set) NDXCTR is 1 and only the PIPA test runs
 *     (PIPACHK): it times the vertical PIPA over 58 seconds, catching each
 *     pulse (CHECKG) to the scaler's 1/1600 second (FINETIME); AINGOTN turns
 *     the time into the PIPA's rate.
 *   - Otherwise IMUFINE fine aligns, LONGCALL waits PERFDLAY for the
 *     suspension to settle, and ESTIMS starts the drift test (drift_loop.c).
 *   - VERTDRFT is the hour-long vertical drift test: the platform offset to
 *     miss the PIP dead zones, and only the south gyro compensated for the
 *     earth's rotation.
 *
 * Every word is in bank 33. The erasable words are EBANK 5's. The
 * interpretive code among the basic (GUESS, the axes, AINGOTN, OVERFFIX)
 * is written in the listing's notation: tables the readable interpreter
 * runs (src/interpreter/notation.h). */
#include "imu_calibration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- IMUTEST and the positioning -- pp. 423-424 ---- */

/* IMUTEST -- p. 423. The IMU performance test's start: DRIFTT and GEOCOMP1
 * +0 (no drift test, no gyrocompassing yet), LENGTHOT the test's length
 * (TESTTIME), and "TAKE CARE OF DRIFT FLAG": coarse align to +0. */
void imutest(void)
{
    set_A(ZERO);
    ts(E(DRIFTT));
    ts(E(GEOCOMP1));
    set_A(TESTTIME);
    ts(E(LENGTHOT));
    tc_at(IMUTEST + 5u);
    coaalign();
}

/* IMUTEST +6 -- p. 423. COAALIGN's return. ALLOOP's interval, 1SECXT1,
 * one second; 1/PIPADT, the PIPA reading interval, 14400 (octal); and
 * GUESS: "CALCULATE -COS LATITUDE AND SIN LATITUDE" (WANGI and WANGO, the
 * earth rate's horizontal and vertical parts) in the interpreter. */
void imutest_after_coaalign(void)
{
    set_A(ONE_SECX);
    ts(E(1SECXT1));
    set_A(OC14400);
    ts(E(1_PIPADT));
    tc_at(GUESS);
    intpret();
}

/* GUESS +1 -- p. 423. LATAZCHK (bank 4, optical_verification.c) lets the
 * crew check the latitude and azimuth. Then the earth rate's parts at the
 * site, each doubled (SL1): WANGI = -cos LAT, WANGO = sin LAT. The EXIT
 * goes on to GEOIMUTT. */
static const word guess_words[] = {
    ORDER(CALL),      ADDRESS(LATAZCHK), ORDERS(COS, DCOMP),
    ORDER(SL1),       STODL(WANGI),      ADDRESS_PLUS_ONE(LATITUDE),
    ORDERS(SIN, SL1), STORE(WANGO),      ORDER(EXIT),
};

const struct interpretive_code GUESS_CODE = {033u, GUESS + 1u, COUNT(guess_words), guess_words};

/* GEOIMUTT -- p. 423, GUESS's EXIT: "GYROCOMPASS COMES IN HERE". Zero the
 * IMU (IMUZERO). */
void geoimutt(void)
{
    tc_at(GEOIMUTT);
    continue_at(BANKCALL); /* CADR IMUZERO */
}

/* GEOIMUTT +2 -- p. 423. Wait for the IMU to settle (IMUSTLLG), which
 * comes back to IMUBACK. */
void geoimutt_after_imuzero(void)
{
    tc_at(GEOIMUTT + 2u);
    imustllg();
}

/* IMUBACK -- p. 423. NDXCTR (the gimbal lock count) and TORQNDX (the
 * vertical drift test's) +0, and NBPOSPL. */
void imuback(void)
{
    set_A(ZERO);
    ts(E(NDXCTR));
    ts(E(TORQNDX));
    ts(E(TORQNDX) + 1u);
    nbpospl();
}

/* NBPOSPL -- p. 423 (also the optical verification's, SETNBPOS). Clear
 * the 18 words XNB to ZNB +5 (ZERONDX1, the count less one: DEC17) with
 * ZEROING, which comes back below. */
void nbpospl(void)
{
    set_A(DEC17);
    ts(E(ZERONDX1));
    set_A(XNBADR);
    tc_at(NBPOSPL + 3u);
    zeroing();
}

/* NBPOSPL +4 -- p. 424. XNB's first component one half (X up), and the
 * interpreter: YNB and ZNB from the azimuth,
 *   YNB = (0, sin AZ, cos AZ), ZNB = (0, -cos AZ, sin AZ). */
void nbpospl_after_zeroing(void)
{
    set_A(HALF);
    ts(E(XNB));
    tc_at(NBPOSPL + 6u);
    intpret();
}

/* NBPOSPL +7 -- p. 424. The azimuth's sine into YNB +2 and ZNB +4, its
 * cosine into YNB +4, negated into ZNB +2. */
static const word axes_words[] = {
    ORDERS(DLOAD, SIN),        ADDRESS_PLUS_ONE(AZIMUTH),
    STORE_AT(YNB, 2),          STODL_AT(ZNB, 4),
    ADDRESS_PLUS_ONE(AZIMUTH), ORDER(COS),
    STORE_AT(YNB, 4),          ORDER(DCOMP),
    STORE_AT(ZNB, 2),          ORDER(EXIT),
};

const struct interpretive_code AXES_CODE = {033u, NBPOSPL + 7u, COUNT(axes_words), axes_words};

/* NBPOSPL +021 -- p. 424, the EXIT: "SEE IF IN OPTICAL VERIFICATION"
 * (major mode 03, P03). */
void nbpospl_after_exit(void)
{
    tc_at(NBPOSPL + 021u);
    continue_at(CHECKMM); /* MM 03 */
}

/* NBPOSPL +023: not in P03. TCF +2: CALCGA, in the interpreter, finds the
 * gimbal angles for the axes. */
void nbpospl_not_verifying(void)
{
    tc_at(NBPOSPL + 025u);
    intpret();
}

/* NBPOSPL +026 -- p. 424. */
static const word calcga_words[] = {
    ORDER(CALL),
    ADDRESS(CALCGA),
    ORDER(EXIT),
};

const struct interpretive_code CALCGA_CODE = {033u, NBPOSPL + 026u, COUNT(calcga_words),
                                              calcga_words};

/* NBPOSPL +024: in P03, on to the optical verification (SETNBPOS +1). */
void nbpospl_verifying(void)
{
    continue_at(SETNBPOS + 1u);
}

/* NBPOSPL +031 -- p. 424, CALCGA's EXIT: coarse align to its angles. */
void nbpospl_after_calcga(void)
{
    tc_at(NBPOSPL + 031u);
    continue_at(BANKCALL); /* CADR IMUCOARS */
}

/* NBPOSPL +033 -- p. 424. "IF GLOKFAIL SET, GIMBAL LOCK": then NDXCTR one
 * more ("+1 IF IN GIMBAL LOCK,OTHERWISE 0"), and "RESET GIMBAL LOCK
 * FLAG". */
void nbpospl_after_imucoars(void)
{
    set_A(GLOKFBIT);
    mask_at(ADDRESS_OF(FLAGWRD3));
    if (!A_is_zero()) incr(E(NDXCTR)); /* EXTEND; BZF +2 */
    tc_at(NBPOSPL + 040u);
    continue_at(DOWNFLAG); /* ADRES GLOKFAIL */
}

/* NBPOSPL +042 -- p. 424. Wait for the IMU (IMUSTLLG). */
void nbpospl_after_downflag(void)
{
    tc_at(NBPOSPL + 042u);
    imustllg();
}

/* NBPOSPL +043 -- p. 424. "IF ONE GO AND DO A PIPA TEST ONLY": NDXCTR
 * positive, PIPACHK; +0, "ALIGN AND MEASURE VERTICAL PIPA RATE": fine
 * align. (Negative it would run the CADR after the TC; -0 the TC IMUSTLLG,
 * as the faithful port does.) */
void nbpospl_after_imustllg(void)
{
    switch (ccs_at(E(NDXCTR))) {
    case POSITIVE:
        tc_at(NBPOSPL + 044u);
        pipachk();
        return;
    case IS_PLUS_ZERO: nbpospl_fine_align(); return;
    case NEGATIVE: not_modelled(); return; /* (CADR IMUFINE, a data word) */
    case IS_MINUS_ZERO: nbpospl_after_imufine(); return;
    }
}

/* NBPOSPL +045 -- p. 424. Fine align (IMUFINE). */
void nbpospl_fine_align(void)
{
    tc_at(NBPOSPL + 045u);
    continue_at(BANKCALL); /* CADR IMUFINE */
}

/* NBPOSPL +047 -- p. 424. Wait for the IMU (IMUSTLLG). */
void nbpospl_after_imufine(void)
{
    tc_at(NBPOSPL + 047u);
    imustllg();
}

/* NBPOSPL +050 -- p. 424. Let the suspension settle: GOESTIMS in PERFDLAY
 * centiseconds (LONGCALL), then this job sleeps till it wakes it. */
void nbpospl_settle(void)
{
    dca(E(PERFDLAY));
    tc_at(NBPOSPL + 052u);
    continue_at(LONGCALL); /* 2CADR GOESTIMS */
}

/* NBPOSPL +055 -- p. 424. */
void nbpospl_after_longcall(void)
{
    set_A(ESTICADR);
    tc_at(NBPOSPL + 056u);
    continue_at(JOBSLEEP);
}

/* GOESTIMS -- p. 424. LONGCALL's task: wake the job, at ESTIMS. */
void goestims(void)
{
    set_A(ESTICADR);
    tc_at(GOESTIMS + 1u);
    continue_at(JOBWAKE);
}

/* GOESTIMS +2: the task ends. */
void goestims_after_jobwake(void)
{
    tc_at(GOESTIMS + 2u);
    continue_at(TASKOVER);
}

/* ---- The PIPA test -- pp. 425-426 ---- */

/* TORQUE -- p. 425. Show the drift found: DRIFTI in DSPTEM2 +1, and keep
 * it for this position (SOUTHDR -1 + POSITON). */
void torque(void)
{
    set_A(ZERO);
    ts(E(DSPTEM2));
    ca(E(DRIFTI));
    ts(E(DSPTEM2) + 1u);
    ts(indexed_erasable(E(SOUTHDR) - 1u, fetch(E(POSITON)), INDEXED_STORE));
    tc_at(TORQUE + 6u);
    show();
}

/* INDEX NDXCTR; TC +1 at word at: the TC goes NDXCTR words on from
 * at + 1 (so in gimbal lock, NDXCTR 1, past the TC EARTHR* there). A word
 * outside memory the faithful port does not run. */
static void tc_indexed_by_ndxctr(word at)
{
    long to = (long)(at + 1u) + signed_word(fetch(E(NDXCTR)));

    if (to < 0 || to > 07777) not_modelled();
    tc_at(at);
    continue_at((word)to);
}

/* PIPACHK -- p. 425. "PIPA TEST". Earth rate compensation first (EARTHR*),
 * unless in gimbal lock. */
void pipachk(void)
{
    tc_indexed_by_ndxctr(PIPACHK + 1u);
}

/* PIPACHK +2 -- p. 425. */
void pipachk_earthr(void)
{
    tc_at(PIPACHK + 2u);
    earthr_star();
}

/* PIPACHK +3 -- p. 425, where EARTHR* returns: LENGTHOT, the time
 * allotted, 58 seconds (DEC57); RESULTCT 1, where CHECKG puts the first
 * pulse's time; the PIPA (PIPINDEX: whichever is up), DATAPL and DATAPL +4
 * cleared. Then catch a pulse. */
void pipachk_start(void)
{
    set_A(DEC57);
    ts(E(LENGTHOT));
    set_A(ONE);
    ts(E(RESULTCT));
    set_A(ZERO);
    ts(indexed_erasable(ADDRESS_OF(PIPAX), fetch(E(PIPINDEX)), INDEXED_STORE));
    ts(E(DATAPL));
    ts(E(DATAPL) + 4u);
    tc_at(PIPACHK + 014u);
    checkg();
}

/* PIPACHK +015 -- p. 425. The first pulse caught: PIPATASK every 20 ms
 * counts LENGTHOT down; this job ends. */
void pipachk_after_checkg(void)
{
    inhint();
    set_A(TWO);
    tc_at(PIPACHK + 017u);
    continue_at(TWIDDLE); /* ADRES PIPATASK */
}

/* PIPACHK +021 -- p. 425. */
void pipachk_after_twiddle(void)
{
    tc_at(PIPACHK + 021u);
    continue_at(ENDOFJOB);
}

/* PIPATASK -- p. 425. A task: LENGTHOT one less; while it is left, again
 * in 5.12 seconds (BIT10 centiseconds); when it is out, STARTPIP. */
void pipatask(void)
{
    dim(E(LENGTHOT));
    ca(E(LENGTHOT));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF STARTPIP */
        startpip();
        return;
    }
    set_A(BIT10);
    tc_at(PIPATASK + 6u);
    continue_at(TWIDDLE); /* ADRES PIPATASK */
}

/* STARTPIP -- p. 425. Start PIPJOBB as a job. */
void startpip(void)
{
    set_A(PRIO20);
    tc_at(STARTPIP + 1u);
    continue_at(FINDVAC); /* 2CADR PIPJOBB */
}

/* STARTPIP +4: the task ends. */
void startpip_after_findvac(void)
{
    tc_at(STARTPIP + 4u);
    continue_at(TASKOVER);
}

/* PIPJOBB -- p. 425. INDEX NDXCTR; TC +1, as PIPACHK's. */
void pipjobb(void)
{
    tc_indexed_by_ndxctr(PIPJOBB + 1u);
}

/* PIPJOBB +2 -- p. 425. */
void pipjobb_earthr(void)
{
    tc_at(PIPJOBB + 2u);
    earthr_star();
}

/* PIPJOBB +3 -- p. 425. Unless LENGTHOT has run out, the job ends; else
 * catch the closing pulse, its time into DATAPL +5 (RESULTCT 5). */
void pipjobb_check_time(void)
{
    ca(E(LENGTHOT));
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        tc_at(PIPJOBB + 6u);
        continue_at(ENDOFJOB);
        return;
    }
    set_A(FIVE);
    ts(E(RESULTCT));
    tc_at(PIPJOBB + 011u);
    checkg();
}

/* PIPJOBB +012 -- p. 426. The counts between the two pulses, double
 * precision, DATAPL +4 = DATAPL +4 - DATAPL; then the times, in the
 * interpreter: the time between them (OVERFFIX when the scaler went round)
 * and AINGOTN, the PIPA's rate. */
void pipjobb_after_checkg(void)
{
    dcs(E(DATAPL));
    das(E(DATAPL) + 4u);
    tc_at(PIPJOBB + 015u);
    intpret();
}

/* PIPJOBB +016 -- p. 426. The time between the pulses, DATAPL +6 less
 * DATAPL +2 (scaler times, double precision); negative, the scaler went
 * round, and OVERFFIX adds a full count back. AINGOTN: the pulses (DATAPL
 * +4) over the time (PDDL keeps it on the push-down list, and DDV divides
 * by it), scaled by DEC585, the PIPA's rate (SGNAGREE makes its words'
 * signs agree), into DSPTEM2 for display. */
static const word aingotn_words[] = {
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_AT(DATAPL, 6),
    ADDRESS_PLUS_ONE_AT(DATAPL, 2),
    ORDERS(BPL, CALL),
    ADDRESS(AINGOTN),
    ADDRESS(OVERFFIX),
    /* AINGOTN */ ORDERS(PDDL, DDV), /* (the time, from the push-down list) */
    ADDRESS_PLUS_ONE_AT(DATAPL, 4),
    ORDERS(SL4, DMPR),
    ADDRESS_PLUS_ONE(DEC585), /* "DEC585 HAS BEEN REDEFINED FOR LEM" */
    ORDER(RTB),
    ADDRESS(SGNAGREE),
    STORE(DSPTEM2),
    ORDER(EXIT),
};

const struct interpretive_code AINGOTN_CODE = {033u, PIPJOBB + 016u, COUNT(aingotn_words),
                                               aingotn_words};

/* AINGOTN +010 -- p. 426, the EXIT: in gimbal lock (NDXCTR positive)
 * "TAKE PLATFORM OUT OF GIMBAL LOCK" (COAALIGN, back to AINGOTN +012);
 * otherwise show the result. (Negative the faithful port runs VERTDRFT,
 * -0 the CA after it: below.) */
void aingotn_after_exit(void)
{
    switch (ccs_at(E(NDXCTR))) {
    case POSITIVE:
        tc_at(AINGOTN + 011u);
        coaalign();
        return;
    case IS_PLUS_ZERO: aingotn_show(); return;
    case NEGATIVE: vertdrft(); return;
    case IS_MINUS_ZERO: vertdrft_plus1(); return;
    }
}

/* AINGOTN +012 -- p. 426. Show the PIPA's rate (DSPTEM2). */
void aingotn_show(void)
{
    tc_at(AINGOTN + 012u);
    show();
}

/* ---- The vertical drift test -- pp. 426-427 ---- */

/* VERTDRFT -- p. 426. "ABOUT 1 HOUR VERTICAL DRIFT TEST": LENGTHOT 3990
 * (seconds times five less one, ALLOOP's count). */
void vertdrft(void)
{
    set_A(DEC3990);
    vertdrft_plus1();
}

/* VERTDRFT +1 -- p. 426. DRIFTT the drift found here before, negated
 * (SOUTHDR -2 + POSITON). "OFFSET PLATFORM" to miss the PIPs' dead zones:
 * BIT5 off ERCOMP1 +2, and onto ERCOMP1 (position 4, XSM +4 +0) or
 * ERCOMP1 +4 (PON2), the gyro torques EARTHR sends. */
void vertdrft_plus1(void)
{
    ts(E(LENGTHOT));
    cs(indexed_erasable(E(SOUTHDR) - 2u, fetch(E(POSITON)), INDEXED_READ));
    ts(E(DRIFTT));
    ca(E(XSM) + 4u);
    if (A_is_zero()) {       /* EXTEND; BZF PON2 */
        set_A(negate(BIT5)); /* PON2 */
        ads(E(ERCOMP1) + 2u);
        set_A(BIT5);
        ads(E(ERCOMP1) + 4u);
    } else {
        set_A(negate(BIT5)); /* PON4 */
        ads(E(ERCOMP1) + 2u);
        set_A(BIT5);
        ads(E(ERCOMP1));
    }
    tc_at(PONG); /* PONG */
    earthr_star();
}

/* PONG +1 -- p. 426. "ALLOW ONLY SOUTH GYRO EARTH RATE COMPENS": the first
 * two components of ERVECTOR +0. GUESS1: TORQNDX and TORQNDX +1 POSMAX (in
 * the vertical drift test), LOSVEC the X gimbal angle now; then the drift
 * test (ESTIMS). */
void pong_after_earthr_star(void)
{
    set_A(ZERO);
    ts(E(ERVECTOR));
    ts(E(ERVECTOR) + 1u);
    set_A(POSMAX); /* GUESS1 */
    ts(E(TORQNDX));
    ts(E(TORQNDX) + 1u);
    ca(ADDRESS_OF(CDUX));
    ts(E(LOSVEC));
    tc_at(GUESS1 + 5u);
    estims();
}

/* VALMIS -- p. 427. Show DRIFTO, the drift found. */
void valmis(void)
{
    ca(E(DRIFTO));
    ts(E(DSPTEM2) + 1u);
    set_A(ZERO);
    ts(E(DSPTEM2));
    tc_at(VALMIS + 4u);
    show();
}

/* ENDTEST1 -- p. 427. The test's end: "IMU NOT IN USE" (IMUSE clear),
 * the major mode -0 (no program), the marks released (MKRELEAS), and the
 * job ends (ENDEXT). */
void endtest1(void)
{
    tc_at(ENDTEST1);
    continue_at(DOWNFLAG); /* ADRES IMUSE */
}

/* ENDTEST1 +2. */
void endtest1_after_downflag(void)
{
    set_A(negate(ZERO));
    tc_at(ENDTEST1 + 3u);
    continue_at(NEWMODEX + 3u);
}

/* ENDTEST1 +4. */
void endtest1_after_newmodex(void)
{
    tc_at(ENDTEST1 + 4u);
    continue_at(BANKCALL); /* CADR MKRELEAS */
}

/* ENDTEST1 +6. */
void endtest1_after_mkreleas(void)
{
    tc_at(ENDTEST1 + 6u);
    continue_at(ENDEXT);
}

/* ---- The subroutines -- pp. 428-429 ---- */

/* OVERFFIX -- p. 428. The scaler went round between the two times: add a
 * full count, DPPOSMAX and ONEDPP (one in the low word), and RVQ. */
static const word overffix_words[] = {
    ORDERS(DAD, DAD),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    ADDRESS_PLUS_ONE(ONEDPP),
    ORDER(RVQ),
};

const struct interpretive_code OVERFFIX_CODE = {033u, OVERFFIX, COUNT(overffix_words),
                                                overffix_words};

/* COAALIGN -- p. 428. "COARSE ALIGN SUBROUTINE": the return into QPLACE,
 * THETAD (the gimbal angles wanted) all +0, and IMUCOARS. */
void coaalign(void)
{
    qxch(E(QPLACE));
    set_A(ZERO);
    ts(E(THETAD));
    ts(E(THETAD) + 1u);
    ts(E(THETAD) + 2u);
    tc_at(COAALIGN + 6u);
    continue_at(BANKCALL); /* CADR IMUCOARS */
}

/* COAALIGN +010 -- p. 428. Wait for the IMU (IMUSTALL). */
void coaalign_after_imucoars(void)
{
    tc_at(COAALIGN + 010u);
    continue_at(BANKCALL); /* CADR IMUSTALL */
}

/* COAALIGN +012: IMUSTALL's bad return: SOMERR2. */
void coaalign_bad_stall(void)
{
    tc_at(COAALIGN + 012u);
    somerr2();
}

/* COAALIGN +013: back to the caller, through QPLACE. */
void coaalign_after_imustall(void)
{
    tc_return(E(QPLACE));
}

/* IMUSTLLG -- p. 428. COAALIGN's wait alone: the return into QPLACE, and
 * IMUSTALL. */
void imustllg(void)
{
    qxch(E(QPLACE));
    tc_at(IMUSTLLG + 2u);
    coaalign_after_imucoars(); /* TC COAALIGN +10 */
}

/* CHECKG -- p. 428. "PIP PULSE CATCHING ROUTINE": wait for the PIPA
 * (PIPINDEX's) to count, and note the time it did to the scaler's 1/1600
 * second. The return goes into QPLACE. */
void checkg(void)
{
    qxch(E(QPLACE));
    tc_at(CHECKG + 2u);
    checkg_after_chang1(); /* TC +6 */
}

/* The PIPA, as an INDEX PIPINDEX names it. */
static unsigned pipa(void)
{
    return indexed_erasable(ADDRESS_OF(PIPAX), fetch(E(PIPINDEX)), INDEXED_READ);
}

/* CHECKG1 +011 -- p. 428. Has the PIPA moved since? Not yet: back to
 * CHECKG1, where the AGC waits for it (the binding stops the run there).
 * It has: its count into DATAPL + RESULTCT, and the time (FINETIME). */
static void checkg_look(void)
{
    inhint();
    ca(pipa());
    ad_at(E(ZERONDX));
    if (A_is_zero()) { /* EXTEND; BZF CHECKG1 */
        continue_at(CHECKG1);
        return;
    }
    ca(pipa());
    ts(indexed_erasable(E(DATAPL), fetch(E(RESULTCT)), INDEXED_STORE));
    tc_at(CHECKG1 + 023u);
    finetime();
}

/* CHECKG1 -- p. 428. Let the PIPA's counting interrupt in, and a job of
 * higher priority run (CHANG1, which comes back below); then look again. */
void checkg1(void)
{
    relint();
    ca(ADDRESS_OF(NEWJOB));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +6 */
        checkg_look();
        return;
    }
    tc_at(CHECKG1 + 4u);
    continue_at(CHANG1);
}

/* CHECKG1 +5 -- p. 428. The PIPA as it is now, complemented, in ZERONDX. */
void checkg_after_chang1(void)
{
    inhint();
    cs(pipa());
    ts(E(ZERONDX));
    checkg_look();
}

/* CHECKG1 +024 -- p. 428. FINETIME's return (A the scaler's low part, L
 * its high part): the time into DATAPL +1 and +2 (+ RESULTCT). A TS that
 * overflows skips the INDEX, as the faithful port does. ENDCHKG: back
 * through QPLACE. */
void checkg_after_finetime(void)
{
    if (ts(indexed_erasable(E(DATAPL) + 1u, fetch(E(RESULTCT)), INDEXED_STORE)))
        lxch(E(DATAPL) + 2u);
    else
        lxch(indexed_erasable(E(DATAPL) + 2u, fetch(E(RESULTCT)), INDEXED_EXCHANGE));
    relint();
    tc_return(E(QPLACE)); /* ENDCHKG */
}

/* ZEROING -- p. 429. Clear C(ZERONDX1) +1 words from the address in A,
 * kept in L; return through Q. (ZEROING1 is the loop's head.) */
void zeroing(void)
{
    ts(L_ADDRESS);
    for (;;) {
        set_A(ZERO);
        ts(indexed_erasable(0u, overflow_corrected(L), INDEXED_STORE)); /* INDEX L; TS 0 */
        incr(L_ADDRESS);
        switch (ccs_at(E(ZERONDX1))) {
        case POSITIVE: ts(E(ZERONDX1)); continue; /* TCF ZEROING1 */
        case IS_PLUS_ZERO: tc_q(); return;
        default: not_modelled(); /* (the next words are EARTHR's) */
        }
    }
}

#endif
