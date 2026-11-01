/* SPDX-License-Identifier: MIT */
/* IMU_PERFORMANCE_TEST_2.agc (Luminary099 pages 373-381) and
 * IMU_PERFORMANCE_TESTS_4.agc (pages 382-389): P07, the IMU performance
 * test, which the system test group ran on the ground with the LM on the
 * pad. "POSITIONING ROUTINES FOR THE IMU PERFORMANCE TESTS AS WELL AS SOME
 * OF THE TESTS THEMSELVES" (TEST_2, by the System Test Group, March 1967)
 * and "THE FILTER FOR THE GYRO DRIFT TESTS" (TESTS_4, by George Schmidt,
 * November 1966). The faithful C is c64-port/src/lm_imu_performance.c.
 *
 * The test, in outline:
 *   - GEOIMUTT zeroes the IMU (IMUZERR), and GUESS asks the crew for the
 *     site's latitude and azimuth (V06 N41). From them it works out where
 *     the platform's axes should point (XNB, YNB, ZNB: X up, Z north
 *     rotated by the azimuth), and CALCGA the gimbal angles; IMUCOARS
 *     drives the platform there.
 *   - In gimbal lock (GLOKFAIL set) NDXCTR is 1 and only the PIPA test runs
 *     (PIPACHK): it counts the time the vertical PIPA takes to overflow 17
 *     times, catching each PIPA pulse (CHECKG) to the scaler's 1/1600
 *     second (FINETIME), and AINGOTN turns the time into the PIPA's rate.
 *   - Otherwise FINIMUDD fine aligns, LONGCALL waits PERFDLAY for the
 *     suspension to settle, and ESTIMS starts the drift test: ALLOOP, a
 *     task each second, reads and zeroes the PIPAs; ALFLT, a job, runs the
 *     filter (the gains ALK and ALDK, the integrals INTY, the velocities
 *     VLAUN and the angles ANGX..ANGZ), and the erasable program at
 *     ATIGINC (keyed in on the ground) does the rest. When LENGTHOT runs
 *     out, SETUPER1 torques the platform by the angles found (OGC, IMUPULSE)
 *     and SHOW flashes the result (V06 N98) for the crew.
 *   - Throughout, EARTHR compensates the gyros for the earth's rotation
 *     (ERTHRVSE works out its vector, ERVECTOR, from the latitude), and
 *     VERTDRFT is the hour-long vertical drift test.
 *
 * Basic code, and interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs: one
 * table for each run of it between basic code (IMU_PERFORMANCE_CODES).
 * Every word is in bank 37 (IMU2 and IMU4), but FINETIME's, in
 * fixed-fixed. The erasable words are EBANK 5's (POSITON, NDXCTR, QPLACE,
 * ZERONDX ...).
 *
 * Each function below runs from its word (Z) until control leaves it: for
 * another of this file's words (continue_at; the binding,
 * src/binding/imu_performance_routines.c, runs that word's function next),
 * another file's routine (BANKCALL, TWIDDLE, ...), or the interpreter. */
#include "imu_performance.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099 /* (the LM's alone) */

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define E(name)      ADDRESS_OF(name) /* an erasable word */

/* ---- Two things every routine here does ---- */

/* INDEX k, then an instruction on erasable word base: the word C(k) on.
 * Here the index is always a count from +0 into a table (POSITON into
 * SOUTHDR, PIPINDEX into the PIPAs, RESULTCT into DATAPL, L in ZEROING);
 * one that would take the instruction out of erasable memory (a negative
 * index among them) is not modelled. */
static unsigned indexed_word(unsigned base, unsigned by)
{
    word index = fetch(by);

    if (index == MINUS_ZERO) index = PLUS_ZERO; /* -0 indexes nothing */
    if (index > 01777u - base) not_modelled();
    return base + index;
}

/* TC QPLACE, TC QPLACES, TC ZERONDX and TC Q: back to the caller, through
 * the return its QXCH (or its TC) saved there (tc_through). The callers
 * are this file's own TCs, and SCALPREP's TC FINETIME +1, so the word is
 * an address in fixed memory, after a TC; not one into FINETIME, BANKCALL's
 * SWCALL or the interpreter, which no caller leaves. Any other word is not
 * modelled. */
static void back_to_caller(unsigned saved)
{
    word to = fetch(saved);

    if (to < 02000u || to > 07777u) not_modelled();
    if ((to >= FINETIME && to <= FINETIME + 021u) || (to >= SWCALL - 1u && to <= SWCALL + 6u) ||
        (to >= INTPRET && to <= ADDRESS_OF(DCOMP) + 2u))
        not_modelled();
    tc_through(saved);
}

/* ======== IMU_PERFORMANCE_TEST_2 ======== */

/* REDO -- p. 373. TC NEWMODEX; MM 07: the display shows P07. */
void redo(void)
{
    tc_at(REDO);
    continue_at(NEWMODEX);
}

/* GEOIMUTT -- p. 373. The test's start: zero the IMU first (IMUZERR, which
 * comes back to IMUBACK). */
void geoimutt(void)
{
    tc_at(GEOIMUTT);
    continue_at(IMUZERR);
}

/* IMUBACK -- p. 373. Clear NDXCTR (the gimbal lock count), TORQNDX and
 * TORQNDX +1 (set in the vertical drift test) and OVFLOWCK (ALLOOP's
 * stop); then NBPOSPL zeroes the 18 words XNB to ZNB +5 (ZERONDX, the
 * count less one: DEC17) with ZEROING, which comes back below. */
void imuback(void)
{
    set_A(ZERO);
    ts(E(NDXCTR));
    ts(E(TORQNDX));
    ts(E(TORQNDX) + 1u);
    ts(E(OVFLOWCK));
    set_A(DEC17); /* NBPOSPL */
    ts(E(ZERONDX));
    set_A(XNBADR);
    tc_at(NBPOSPL + 3u);
    continue_at(ZEROING);
}

/* NBPOSPL +4 -- p. 373. XNB's first component one half (the X axis up),
 * and GUESS: into the interpreter at LATAZCHK. */
void nbpospl_after_zeroing(void)
{
    set_A(HALF);
    ts(E(XNB));
    tc_at(GUESS);
    intpret();
}

/* LATAZCHK -- p. 373. The latitude and azimuth (double precision, in
 * revolutions) for the crew to check: the latitude shifted left two into
 * DSPTEM1 +1, the azimuth (RTB 1STO2S) as a CDU angle; the EXIT comes back
 * below with it in MPAC. */
static const word latazchk_words[] = {
    ORDERS(DLOAD, SL2),        ADDRESS_PLUS_ONE(LATITUDE), STODL_AT(DSPTEM1, 1),
    ADDRESS_PLUS_ONE(AZIMUTH), ORDERS(RTB, EXIT),          ADDRESS(1STO2S),
};

/* LATAZCHK +6 -- p. 373. The azimuth into DSPTEM1. */
void latazchk_after_exit(void)
{
    xch(E(MPAC));
    ts(E(DSPTEM1));
    latazchk_display();
}

/* LATAZCHK +010 -- p. 373. Flash V06 N41, the azimuth and latitude, and
 * wait for the crew: V34 ends the test (ENDTEST1), V33 accepts the values,
 * and loading new ones (enter) flashes them again. */
void latazchk_display(void)
{
    set_A(VN0641);
    tc_at(LATAZCHK + 011u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

/* LATAZCHK +013: GOFLASH's return on V34, terminate. */
void latazchk_after_terminate(void)
{
    tc_at(LATAZCHK + 013u);
    continue_at(ENDTEST1);
}

/* LATAZCHK +014: on V33, proceed. TC +2. */
void latazchk_after_proceed(void)
{
    tc_at(LATAZCHK + 014u);
    continue_at(LATAZCHK + 016u);
}

/* LATAZCHK +015: on enter, new values. TC -5: display them again. */
void latazchk_after_enter(void)
{
    tc_at(LATAZCHK + 015u);
    continue_at(LATAZCHK + 010u);
}

/* LATAZCHK +016 -- p. 374. Into the interpreter. */
void latazchk_accepted(void)
{
    tc_at(LATAZCHK + 016u);
    intpret();
}

/* LATAZCHK +017 -- p. 374. The values the crew accepted back into
 * AZIMUTH (RTB CDULOGIC: CDU angle to revolutions) and LATITUDE. The
 * platform's axes, as the stable member is to point: X up (XNB, set
 * above), Y and Z level, Z rotated by the azimuth from north:
 *   YNB = (0, cos AZ, -sin AZ), ZNB = (0, sin AZ, cos AZ),
 * and the earth rate at the site: WANGI = -cos LAT (horizontal, its input
 * axis component) and WANGO = sin LAT (vertical), both doubled (SL1).
 * POSGMBL: CALCGA finds the gimbal angles for those axes (STCALL stores
 * ZNB +2 first). */
static const word posgmbl_words[] = {
    ORDERS(SLOAD, RTB),
    ADDRESS_PLUS_ONE(DSPTEM1),
    ADDRESS(CDULOGIC),
    STORE(AZIMUTH),
    ORDERS(SLOAD, SR2),
    ADDRESS_PLUS_ONE_AT(DSPTEM1, 1),
    STORE(LATITUDE),
    ORDERS(COS, DCOMP),
    ORDER(SL1),
    STODL(WANGI),
    ADDRESS_PLUS_ONE(LATITUDE),
    ORDERS(SIN, SL1),
    STODL(WANGO),
    ADDRESS_PLUS_ONE(AZIMUTH),
    ORDERS(PUSH, SIN),
    STORE_AT(YNB, 2),
    STODL_AT(ZNB, 4), /* (the azimuth back from the push-down list) */
    ORDER(COS),
    STORE_AT(YNB, 4),
    ORDER(DCOMP),
    /* POSGMBL */ STCALL_AT(ZNB, 2),
    ADDRESS(CALCGA),
    ORDER(EXIT),
};

/* POSGMBL +3 -- p. 374. Coarse align to CALCGA's angles. */
void posgmbl_after_exit(void)
{
    tc_at(POSGMBL + 3u);
    continue_at(BANKCALL); /* CADR IMUCOARS */
}

/* POSGMBL +5 -- p. 374. "IF BIT14 SET, GIMBAL LOCK" (FLAGWRD3's GLOKFAIL):
 * then NDXCTR is "+1 IF IN GIMBAL LOCK, OTHERWISE 0". DOWNFLAG "RESET
 * GIMBAL LOCK FLAG". */
void posgmbl_after_imucoars(void)
{
    set_A(BIT14);
    mask_at(ADDRESS_OF(FLAGWRD3));
    if (!A_is_zero()) incr(E(NDXCTR)); /* EXTEND; BZF +2 */
    tc_at(POSGMBL + 012u);
    continue_at(DOWNFLAG); /* ADRES GLOKFAIL */
}

/* POSGMBL +014 -- p. 374. Wait for the IMU to settle (IMUSLLLG). */
void posgmbl_after_downflag(void)
{
    tc_at(POSGMBL + 014u);
    continue_at(IMUSLLLG);
}

/* POSGMBL +015 -- p. 374. "IF ONE GO AND DO A PIPA TEST ONLY": PIPACHK
 * "ALIGN AND MEASURE VERTICAL PIPA RATE". Otherwise fine align (FINIMUDD),
 * which comes back to the DCA below. */
void posgmbl_after_imuslllg(void)
{
    switch (ccs_at(E(NDXCTR))) {
    case POSITIVE:
        tc_at(POSGMBL + 016u);
        continue_at(PIPACHK);
        return;
    case IS_PLUS_ZERO:
        tc_at(POSGMBL + 017u);
        continue_at(FINIMUDD);
        return;
    case NEGATIVE: posgmbl_after_finimudd(); return;
    case IS_MINUS_ZERO:
        /* The fourth word is the DCA's own (its EXTEND the third): without
         * the EXTEND it is CA PERFDLAY +1. */
        ca(E(PERFDLAY) + 1u);
        tc_at(POSGMBL + 022u);
        continue_at(LONGCALL);
        return;
    }
}

/* POSGMBL +020 -- p. 374. "DELAY WHILE SUSPENSION STABILIZES": LONGCALL
 * starts GOESTIMS when PERFDLAY (double precision, centiseconds) is up, and
 * this job sleeps until then (JOBSLEEP, its wake address ESTIMS). */
void posgmbl_after_finimudd(void)
{
    dca(E(PERFDLAY));
    tc_at(POSGMBL + 022u);
    continue_at(LONGCALL); /* 2CADR GOESTIMS */
}

/* POSGMBL +025 -- p. 374. */
void posgmbl_after_longcall(void)
{
    set_A(ESTICADR);
    tc_at(POSGMBL + 026u);
    continue_at(JOBSLEEP);
}

/* GOESTIMS -- p. 374. LONGCALL's task: wake the job at ESTIMS. */
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

/* TORQUE -- p. 374. Show the drift found: DRIFTI in DSPTEM2 +1, and
 * remember it in SOUTHDR -1 + POSITON, the drift for this position. */
void torque(void)
{
    set_A(ZERO);
    ts(E(DSPTEM2));
    ca(E(DRIFTI));
    ts(E(DSPTEM2) + 1u);
    ts(indexed_word(E(SOUTHDR) - 1u, E(POSITON)));
    tc_at(TORQUE + 6u);
    continue_at(SHOW);
}

/* PIPACHK -- p. 375. "PIPA TEST". INDEX NDXCTR; TC +1: the TC goes NDXCTR
 * words on, so in gimbal lock (NDXCTR 1) the test skips EARTHR*, the earth
 * rate compensation. */
void pipachk(void)
{
    word index = fetch(E(NDXCTR));

    if (index == MINUS_ZERO) index = PLUS_ZERO;
    if (index > 03777u - (PIPACHK + 2u)) not_modelled(); /* past the bank */
    tc_at(PIPACHK + 1u);
    continue_at((word)(PIPACHK + 2u + index));
}

/* PIPACHK +2 -- p. 375, where INDEX NDXCTR; TC +1 lands, or EARTHR*
 * returns to (Z is the word). "ALLOW PIP COUNTER TO OVERFLOW 17 TIMES IN
 * THE ALLOTTED TIME INTERVAL" (DATAPL +4); LENGTHOT, the time allotted, 58
 * seconds; RESULTCT 1, where CHECKG puts the first pulse's time; and the
 * PIPA (PIPINDEX: X, Y or Z, whichever is up) and DATAPL cleared. Then
 * catch a pulse. */
void pipachk_landing(void)
{
    switch ((word)(Z - PIPACHK)) {
    case 02u:
        tc_at(PIPACHK + 2u);
        continue_at(EARTHR_STAR);
        return;
    case 03u: set_A(DEC17);                             /* fall through */
    case 04u: ts(E(DATAPL) + 4u);                       /* fall through */
    case 05u: set_A(DEC58);                             /* fall through */
    case 06u: ts(E(LENGTHOT));                          /* fall through */
    case 07u: set_A(ONE);                               /* fall through */
    case 010u: ts(E(RESULTCT));                         /* fall through */
    case 011u: set_A(ZERO);                             /* fall through */
    case 012u: ts(indexed_word(E(PIPAX), E(PIPINDEX))); /* fall through */
    case 014u: ts(E(DATAPL));                           /* fall through */
    case 015u:
        tc_at(PIPACHK + 015u);
        continue_at(CHECKG);
        return;
    default: not_modelled(); /* (the TS after an INDEX: not a word of its own) */
    }
}

/* PIPACHK +016 -- p. 375. The first pulse caught: PIPATASK, every 20 ms,
 * counts LENGTHOT down; this job ends. */
void pipachk_after_checkg(void)
{
    inhint();
    set_A(TWO);
    tc_at(PIPACHK + 020u);
    continue_at(TWIDDLE); /* ADRES PIPATASK */
}

/* PIPACHK +022 -- p. 375. */
void pipachk_after_twiddle(void)
{
    tc_at(PIPACHK + 022u);
    continue_at(ENDOFJOB);
}

/* PIPATASK -- p. 375. A task: LENGTHOT one less; while it is left, again
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

/* STARTPIP -- p. 375. Start PIPJOBB as a job. */
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

/* PIPJOBB -- p. 375. INDEX NDXCTR; TC +1, as PIPACHK's. */
void pipjobb(void)
{
    word index = fetch(E(NDXCTR));

    if (index == MINUS_ZERO) index = PLUS_ZERO;
    if (index > 03777u - (PIPJOBB + 2u)) not_modelled(); /* past the bank */
    tc_at(PIPJOBB + 1u);
    continue_at((word)(PIPJOBB + 2u + index));
}

/* PIPJOBB +2 -- p. 375, where the TC lands, or EARTHR* returns to. Unless
 * LENGTHOT has run out, the job ends; else catch the closing pulse, its
 * time into DATAPL +5 (RESULTCT 5). */
void pipjobb_landing(void)
{
    switch ((word)(Z - PIPJOBB)) {
    case 02u:
        tc_at(PIPJOBB + 2u);
        continue_at(EARTHR_STAR);
        return;
    case 03u: ca(E(LENGTHOT)); /* fall through */
    case 04u:
        if (A_is_zero_or_minus()) goto closing; /* EXTEND; BZMF +2 */
        /* fall through */
    case 06u:
        tc_at(PIPJOBB + 6u);
        continue_at(ENDOFJOB);
        return;
    closing:
    case 07u: set_A(FIVE);      /* fall through */
    case 010u: ts(E(RESULTCT)); /* fall through */
    case 011u:
        tc_at(PIPJOBB + 011u);
        continue_at(CHECKG);
        return;
    default: not_modelled(); /* (BZMF without its EXTEND) */
    }
}

/* PIPJOBB +012 -- p. 375. The pulses counted, double precision, into
 * DATAPL +4: 17 overflows of the PIPA counter (signed as the PIPA counts:
 * the first pulse's count, DATAPL +1; +0 there is a CCSHOLE) and the
 * closing count (DATAPL +5) less the first. */
void pipjobb_after_checkg(void)
{
    switch (ccs_at(E(DATAPL) + 1u)) {
    case POSITIVE: break; /* TC +4 */
    case IS_PLUS_ZERO:
        tc_at(PIPJOBB + 014u);
        continue_at(CCSHOLE);
        return;
    case NEGATIVE: cs(E(DATAPL) + 4u); /* fall through */
    case IS_MINUS_ZERO: ts(E(DATAPL) + 4u); break;
    }
    dcs(E(DATAPL));
    das(E(DATAPL) + 4u);
    tc_at(PIPJOBB + 022u);
    intpret();
}

/* PIPJOBB +023 -- p. 376. The time between the pulses, DATAPL +6 less
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
    ORDERS(DMPR, RTB),
    ADDRESS_PLUS_ONE(DEC585), /* "DEC585 HAS BEEN REDEFINED FOR LEM" */
    ADDRESS(SGNAGREE),
    STORE(DSPTEM2),
    ORDER(EXIT),
};

/* AINGOTN +7 -- p. 376. In gimbal lock, coarse align out of it
 * (COAALIGN) and show the rate; else (NDXCTR +0) just show it. NDXCTR
 * negative: VERTDRFT. */
void aingotn_after_exit(void)
{
    switch (ccs_at(E(NDXCTR))) {
    case POSITIVE:
        tc_at(AINGOTN + 010u);
        continue_at(COAALIGN); /* "TAKE PLATFORM OUT OF GIMBAL LOCK" */
        return;
    case IS_PLUS_ZERO: aingotn_show(); return;
    case NEGATIVE: vertdrft(); return;
    case IS_MINUS_ZERO: vertdrft_plus1(); return; /* (A +0) */
    }
}

/* AINGOTN +011 -- p. 376. COAALIGN comes back here: show the result. */
void aingotn_show(void)
{
    tc_at(AINGOTN + 011u);
    continue_at(SHOW);
}

/* PON2, PON4 and PON -- p. 376. "OFFSET PLATFORM TO MISS PIP DEAD-ZONES":
 * a bias of BIT5 into the earth rate compensation ERCOMP (pairs of words
 * by axis), "Z-UP IN POS 4", X up otherwise; then PON compensates for
 * the earth rate (EARTHR*). */
static void pon(void)
{
    tc_at(PON);
    continue_at(EARTHR_STAR);
}

static void pon2_plus2(void)
{
    set_A(BIT5);
    ads(E(ERCOMP) + 4u);
    pon(); /* TCF PON */
}

static void pon2_plus1(void)
{
    ads(E(ERCOMP) + 2u);
    pon2_plus2();
}

static void pon4(void)
{
    set_A(negate(BIT5));
    ads(E(ERCOMP) + 2u);
    set_A(BIT5);
    ads(E(ERCOMP));
    pon();
}

/* VERTDRFT -- p. 376. "ABOUT 1 HOUR VERTICAL DRIFT TEST": LENGTHOT 3990
 * (seconds); DRIFTT, the drift last found in this position, negated. Then
 * the offset by PIPINDEX (positive: Z up, position 4; +0: X up). */
void vertdrft(void)
{
    set_A(DEC3990);
    vertdrft_plus1();
}

void vertdrft_plus1(void)
{
    ts(E(LENGTHOT));
    cs(indexed_word(E(SOUTHDR) - 2u, E(POSITON)));
    ts(E(DRIFTT));
    switch (ccs_at(E(PIPINDEX))) {
    case POSITIVE: pon4(); return; /* TCF PON4 */
    case IS_PLUS_ZERO:             /* PON2: "X-UP" */
        set_A(negate(BIT5));
        pon2_plus1();
        return;
    case NEGATIVE: pon2_plus1(); return;
    case IS_MINUS_ZERO: pon2_plus2(); return;
    }
}

/* PON +1 -- p. 376. "ALLOW ONLY SOUTH GYRO EARTH RATE COMPENS": clear
 * ERVECTOR's first two words. GUESS1: TORQNDX and TORQNDX +1 at POSMAX
 * mark the vertical drift test; LOSVEC keeps CDUX. Then ESTIMS. */
void pon_after_earthr_star(void)
{
    set_A(ZERO);
    ts(E(ERVECTOR));
    ts(E(ERVECTOR) + 1u);
    set_A(POSMAX); /* GUESS1 */
    ts(E(TORQNDX));
    ts(E(TORQNDX) + 1u);
    ca(E(CDUX));
    ts(E(LOSVEC));
    tc_at(GUESS1 + 5u);
    continue_at(ESTIMS);
}

/* VALMIS -- p. 377. Show DRIFTO, the drift found. */
void valmis(void)
{
    ca(E(DRIFTO));
    ts(E(DSPTEM2) + 1u);
    set_A(ZERO);
    ts(E(DSPTEM2));
    tc_at(VALMIS + 4u);
    continue_at(SHOW);
}

/* ENDTEST1 -- p. 377. The test's end: IMUSE clear (the IMU free), the
 * major mode back to -0 (no program), and the job ends (ENDEXT). */
void endtest1(void)
{
    tc_at(ENDTEST1);
    continue_at(DOWNFLAG); /* ADRES IMUSE */
}

void endtest1_after_downflag(void)
{
    set_A(negate(ZERO));
    tc_at(ENDTEST1 + 3u);
    continue_at(NEWMODEA);
}

void endtest1_after_newmodea(void)
{
    tc_at(ENDTEST1 + 4u);
    continue_at(ENDEXT);
}

/* OVERFFIX -- p. 378. The scaler went round between the two times: add a
 * full count, DPPOSMAX and ONEDPP (one in the low word), and RVQ. */
static const word overffix_words[] = {
    ORDERS(DAD, DAD),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    ADDRESS_PLUS_ONE(ONEDPP),
    ORDER(RVQ),
};

/* COAALIGN -- p. 378. "COARSE ALIGN SUBROUTINE": the return into
 * ZERONDX, THETAD (the gimbal angles wanted) all +0, and IMUCOARS, which
 * comes back to ALIGNCOA. */
void coaalign(void)
{
    qxch(E(ZERONDX));
    set_A(ZERO);
    ts(E(THETAD));
    ts(E(THETAD) + 1u);
    ts(E(THETAD) + 2u);
    tc_at(COAALIGN + 6u);
    continue_at(BANKCALL); /* CADR IMUCOARS */
}

/* ALIGNCOA -- p. 378. Wait for the IMU (IMUSTALL): back to the caller
 * through ZERONDX, or SOMERR2 on a bad return. */
void aligncoa(void)
{
    tc_at(ALIGNCOA);
    continue_at(BANKCALL); /* CADR IMUSTALL */
}

void aligncoa_after_bad_stall(void)
{
    tc_at(ALIGNCOA + 2u);
    continue_at(SOMERR2);
}

void aligncoa_after_stall(void)
{
    back_to_caller(E(ZERONDX));
}

/* IMUSLLLG, FINIMUDD and IMUZERR -- p. 378. Each keeps its return in
 * ZERONDX and ends at ALIGNCOA: IMUSLLLG only waits; FINIMUDD fine aligns
 * (IMUFINE) first, IMUZERR zeroes the IMU (IMUZERO). */
void imuslllg(void)
{
    qxch(E(ZERONDX));
    tc_at(IMUSLLLG + 2u);
    continue_at(ALIGNCOA);
}

void finimudd(void)
{
    qxch(E(ZERONDX));
    tc_at(FINIMUDD + 2u);
    continue_at(BANKCALL); /* CADR IMUFINE */
}

void finimudd_after_imufine(void)
{
    tc_at(FINIMUDD + 4u);
    continue_at(ALIGNCOA);
}

void imuzerr(void)
{
    qxch(E(ZERONDX));
    tc_at(IMUZERR + 2u);
    continue_at(BANKCALL); /* CADR IMUZERO */
}

void imuzerr_after_imuzero(void)
{
    tc_at(IMUZERR + 4u);
    continue_at(ALIGNCOA);
}

/* CHECKG -- p. 378. "PIP PULSE CATCHING ROUTINE": wait for the PIPA
 * (PIPINDEX's) to count, and note the time it did to the scaler's
 * resolution. The return goes into QPLACE. */
void checkg(void)
{
    qxch(E(QPLACE));
    tc_at(CHECKG + 2u);
    checkg_after_chang1(); /* TC +6 */
}

/* CHECKG1 -- p. 378. Let the PIPA's interrupt in, and any job of higher
 * priority run (CHANG1, which comes back below), then look again. */
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

/* CHECKG1 +5 -- p. 378. The PIPA as it is now, complemented, in ZERONDX. */
void checkg_after_chang1(void)
{
    inhint();
    cs(indexed_word(ADDRESS_OF(PIPAX), E(PIPINDEX)));
    ts(E(ZERONDX));
    checkg_look();
}

/* CHECKG1 +011 -- p. 378. Has the PIPA moved? Not yet: back to CHECKG1,
 * where the AGC waits for it (the binding stops the run there). */
void checkg_look(void)
{
    inhint();
    ca(indexed_word(ADDRESS_OF(PIPAX), E(PIPINDEX)));
    ad_at(E(ZERONDX));
    if (A_is_zero()) { /* EXTEND; BZF CHECKG1 */
        continue_at(CHECKG1);
        return;
    }
    checkg_caught();
}

/* CHECKG1 +017 -- p. 379. A pulse: the PIPA's count into DATAPL +
 * RESULTCT, and the time (FINETIME: A the scaler's low part, L its high
 * part) after it. */
void checkg_caught(void)
{
    ca(indexed_word(ADDRESS_OF(PIPAX), E(PIPINDEX)));
    ts(indexed_word(E(DATAPL), E(RESULTCT)));
    tc_at(CHECKG1 + 023u);
    continue_at(FINETIME);
}

/* CHECKG1 +024 -- p. 379. ENDCHKG: back through QPLACE. */
void checkg_after_finetime(void)
{
    ts(indexed_word(E(DATAPL) + 1u, E(RESULTCT)));
    lxch(indexed_word(E(DATAPL) + 2u, E(RESULTCT)));
    relint();
    back_to_caller(E(QPLACE)); /* ENDCHKG */
}

/* ZEROING -- p. 379. Clear C(ZERONDX) +1 words from the address in A,
 * kept in L; return through Q. (ZEROING1 is the loop's head.) */
void zeroing(void)
{
    ts(L_ADDRESS);
    for (;;) {
        unsigned at = indexed_word(0u, L_ADDRESS); /* INDEX L; TS 0 */

        if (at < 020u) not_modelled(); /* (never a central register) */
        set_A(ZERO);
        ts(at);
        incr(L_ADDRESS);
        switch (ccs_at(E(ZERONDX))) {
        case POSITIVE: ts(E(ZERONDX)); continue; /* TCF ZEROING1 */
        case IS_PLUS_ZERO: back_to_caller(Q_ADDRESS); return;
        default: not_modelled(); /* (the next words are ERTHRVSE's) */
        }
    }
}

/* ERTHRVSE -- p. 380. The earth rate vector in stable member axes,
 * "PD24 = (SIN -COS 0)(OMEG/MS)": with X up and Z north (at the site,
 * azimuth aside) the earth turns about (sin LAT, -cos LAT, 0), at OMEG/MS.
 * Into ERVECTOR; TMARK the time now (RTB LOADTIME), ERCOMP cleared. */
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
    STORE(ERCOMP),
    ORDER(RVQ),
    /* EARTHR -- p. 380. Compensate for the earth's rotation since TMARK:
     * the time since (TEMPTIME less TMARK, OVERFFIX if the clock went
     * round), times ERVECTOR, through XSM (the stable member's matrix),
     * added into ERCOMP; then PULSEIMU torques the gyros by it (AXT,1 its
     * ECADR). ITA S2 keeps the return, GOTO S2 goes back. */
    /* EARTHR */ ORDERS(ITA, RTB),
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
    ADDRESS_PLUS_ONE(ERCOMP),
    STODL(ERCOMP),
    ADDRESS_PLUS_ONE(TEMPTIME),
    STORE(TMARK),
    ORDERS(AXT_1, RTB),
    ADDRESS(ERCOMP), /* ECADR ERCOMP */
    ADDRESS(PULSEIMU),
    ORDER(GOTO),
    ADDRESS(S2),
};

/* EARTHR* -- p. 380. EARTHR from basic code: the return into QPLACES,
 * CALL EARTHR, and back. */
void earthr_star(void)
{
    qxch(E(QPLACES));
    tc_at(EARTHR_STAR + 2u);
    intpret();
}

static const word earthr_star_words[] = {
    ORDER(CALL),
    ADDRESS(EARTHR),
    ORDER(EXIT),
};

/* EARTHR* +6 -- p. 380. Wait for the IMU (IMUSLLLG), then back through
 * QPLACES. */
void earthr_star_after_exit(void)
{
    tc_at(EARTHR_STAR + 6u);
    continue_at(IMUSLLLG);
}

void earthr_star_after_imuslllg(void)
{
    back_to_caller(E(QPLACES));
}

/* SHOW -- p. 380. Flash V06 N98 (DSPTEM2: the result; +2 the position)
 * until the crew answers: V34 ends the test, V33 goes back through
 * QPLACE, a load (enter) shows it again. */
void show(void)
{
    qxch(E(QPLACE));
    show1();
}

void show1(void)
{
    ca(E(POSITON));
    ts(E(DSPTEM2) + 2u);
    set_A(VB06N98);
    tc_at(SHOW1 + 3u);
    continue_at(BANKCALL); /* CADR GOFLASH */
}

void show_after_terminate(void)
{
    tc_at(SHOW1 + 5u);
    continue_at(ENDTEST1); /* "V34" */
}

void show_after_proceed(void)
{
    back_to_caller(E(QPLACE)); /* "V33" */
}

void show_after_enter(void)
{
    show1(); /* TCF SHOW1 */
}

/* FINETIME -- p. 381, in fixed-fixed. The time now: the scaler's low part
 * (LOSCALAR) in L, read until two reads agree (it may move between them),
 * and HISCALAR in A. "RETURNS WITH INTERRUPT INHIBITED". At 37777, the
 * low part is about to carry into the high: wait for it to move (the
 * binding stops the run at FINETIME +1). */
void finetime(void)
{
    inhint();
    finetime_plus1();
}

static void finetime_read(void)
{
    set_A(negate(POSMAX)); /* +4: CS POSMAX; AD L */
    ad_at(L_ADDRESS);
    if (A_is_zero()) { /* EXTEND; BZF FINETIME +1 */
        continue_at(FINETIME + 1u);
        return;
    }
    read_channel(HISCALAR);
    back_to_caller(Q_ADDRESS); /* TC Q */
}

void finetime_plus1(void)
{
    read_channel(LOSCALAR);
    ts(L_ADDRESS);
    rxor_channel(LOSCALAR);
    if (A_is_zero()) { /* EXTEND; BZF +4 */
        finetime_read();
        return;
    }
    finetime_reread();
}

/* FINETIME +010: the scaler moved between the two reads; read it again. */
void finetime_reread(void)
{
    read_channel(LOSCALAR);
    ts(L_ADDRESS);
    finetime_read();
}

/* ======== IMU_PERFORMANCE_TESTS_4 ======== */

/* ESTIMS -- p. 383. The drift test's start: ALLOOP every second (1SECXT
 * centiseconds), from now. */
void estims(void)
{
    inhint();
    ca(E(1SECXT));
    tc_at(ESTIMS + 2u);
    continue_at(TWIDDLE); /* ADRES ALLOOP */
}

/* ESTIMS +4 -- p. 383. "ZERO THE PIPAS", and the filter's 78 words from
 * ALX1S -1 on (ZEROING, 77DECML). */
void estims_after_twiddle(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(PIPAY));
    ts(ADDRESS_OF(PIPAZ));
    relint();
    set_A(DEC77);
    ts(E(ZERONDX));
    set_A(ALXXXZ);
    tc_at(ESTIMS + 014u);
    continue_at(ZEROING);
}

/* ESTIMS +015 -- p. 383. Into the interpreter. */
void estims_after_zeroing(void)
{
    tc_at(ESTIMS + 015u);
    intpret();
}

/* ESTIMS +016 -- p. 383. Start the filter: GCOMPSW -1 and GCOMPSW (the
 * gyro compensation's switch) cleared; from INTVAL +2, ALX1S 144 and CMPX1
 * -1 (the gain table's index and the axis sign), and ALK's first gains,
 * SOUPLY: "INITIAL GAINS FOR PIP OUTPUTS" and "INITIAL GAINS/4 FOR
 * ERECTION ANGLES"; DELVX and GCOMP cleared. In the vertical drift test
 * (TORQNDX positive: -TORQNDX negative) no new earth rate (VERTSKIP);
 * else ERTHRVSE. */
static const word estims_words[] = {
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(SCHZEROS),
    (word)(STOVL(GCOMPSW) - 1u), /* STOVL GCOMPSW -1 */
    ADDRESS_PLUS_ONE_AT(INTVAL, 2),
    STOVL(ALX1S),
    ADDRESS_PLUS_ONE(SCHZEROS),
    STORE(DELVX),
    STORE(GCOMP),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(TORQNDX),
    ORDERS(DCOMP, BMN),
    ADDRESS(VERTSKIP),
    ORDER(CALL),
    ADDRESS(ERTHRVSE),
    /* VERTSKIP */ ORDER(EXIT),
};

/* VERTSKIP +1 -- p. 383. TC SLEEPIE +1: end this job (or EARTHR* in the
 * vertical drift test). */
void vertskip_after_exit(void)
{
    tc_at(VERTSKIP + 1u);
    continue_at(SLEEPIE + 1u);
}

/* ALLOOP -- p. 384. The task each second. OVFLOWCK set (by SOMEERRR):
 * stop. ALTIM, negative, counts toward zero each second; at +0 it sets
 * ALTIMS +0, the filter's sign to load the next gains (ALCGKK). ALTIM
 * positive "SHOULD NEVER HIT" its CA A. Then again in a second, unless
 * LENGTHOT has run out (with GEOCOMPS 1, always); and either way the PIPAs
 * read and cleared (DELVX, DELVY, DELVZ) and the filter (ALFLT) started as
 * a job. */
void alloop(void)
{
    ca(E(OVFLOWCK));
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        tc_at(ALLOOP + 3u);
        continue_at(TASKOVER);
        return;
    }
    switch (ccs_at(E(ALTIM))) {
    case POSITIVE: ca(A_ADDRESS);     /* fall through */
    case IS_PLUS_ZERO: ts(E(ALTIMS)); /* fall through */
    case NEGATIVE: cs(A_ADDRESS);     /* fall through */
    case IS_MINUS_ZERO: ts(E(ALTIM)); break;
    }
    set_A(negate(ONE));
    ad_at(E(GEOCOMPS));
    if (!A_is_zero()) { /* EXTEND; BZF +4 */
        ca(E(LENGTHOT));
        if (A_is_zero_or_minus()) { /* EXTEND; BZMF +5 */
            alloop_read_pipas();
            return;
        }
    }
    ca(E(1SECXT));
    tc_at(ALLOOP + 021u);
    continue_at(TWIDDLE); /* ADRES ALLOOP */
}

/* ALLOOP +023 -- p. 384. */
void alloop_after_twiddle(void)
{
    set_A(ZERO);
    alloop_read_pipas();
}

/* ALLOOP +024: XCH: the PIPAs read and cleared at once. */
void alloop_read_pipas(void)
{
    xch(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(DELVX));
    set_A(ZERO);
    xch(ADDRESS_OF(PIPAY));
    ts(ADDRESS_OF(DELVY));
    set_A(ZERO);
    xch(ADDRESS_OF(PIPAZ));
    ts(ADDRESS_OF(DELVZ));
    set_A(PRIO20); /* SPECSTS */
    tc_at(SPECSTS + 1u);
    continue_at(FINDVAC); /* 2CADR ALFLT: "START THE JOB" */
}

/* SPECSTS +4: the task ends. */
void specsts_after_findvac(void)
{
    tc_at(SPECSTS + 4u);
    continue_at(TASKOVER);
}

/* ALFLT -- p. 385. The filter's job. GEOCOMPS positive or negative: first
 * compensate the PIPAs (1/PIPA, which comes back to NORMLOP); +0, no
 * compensation. (-0 would take the next word, a CADR.) */
void alflt(void)
{
    switch (ccs_at(E(GEOCOMPS))) {
    case POSITIVE: break; /* TC +2: to the BANKCALL */
    case IS_PLUS_ZERO:
        tc_at(ALFLT + 2u);
        continue_at(NORMLOP);
        return;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled(); return;
    }
    tc_at(ALFLT + 3u);
    continue_at(BANKCALL); /* CADR 1/PIPA */
}

/* NORMLOP -- p. 385. Into the interpreter. */
void normlop(void)
{
    tc_at(NORMLOP);
    intpret();
}

/* NORMLOP +1 -- pp. 385-387: the filter.
 *   - S1 from INTVAL (4, the step for the TIX loops); the PIPA counts
 *     (DELVX) through XSM into stable member axes, doubled; their Y and Z
 *     components, Y negated, into DPIPAY and DPIPAZ.
 *   - GEOCOMPS positive (-GEOCOMPS negative): no filter, PERFERAS.
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
 *   - LOOSE: ACCWD, VLAUN and POSNV (each axis's) through TRANSM1, and
 *     back.
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
    ADDRESS_PLUS_ONE(GEOCOMPS),
    ORDER(BMN),
    ADDRESS(PERFERAS),
    /* ALCGKK */ ORDERS(SLOAD, BMN),
    ADDRESS_PLUS_ONE(ALTIMS),
    ADDRESS(ALFLT3),
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
    /* ALFLT3 */ ORDER(AXT_1),
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
    /* LOOSE */ ORDERS(DLOAD_STAR, PDDL_STAR),
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

/* PERFERAS +1 -- p. 387. "GOTO ERASABLE TO CALCULATE ONLY TO RETN": the
 * erasable program at ATIGINC, in EBANK 7. "THE ERASABLE PROGRAM THAT DOES
 * THE CALCULATIONS MUST BE LOADED BEFORE ANY ATTEMPT IS MAKE TO RUN THE
 * IMU PERFORMANCE TEST." */
void perferas_after_exit(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_at(PERFERAS + 3u);
    continue_at(ATIGINC);
}

/* SETUPER1 -- p. 388: into the interpreter. */
static void setuper1(void)
{
    tc_at(SETUPER1);
    intpret();
}

/* PERFERAS +011 -- p. 388. LOSVEC +1 keeps CDUX, "FOR TROUBLESHOOTING VD
 * POSNS 2$4". */
static void keep_cdux(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(E(LOSVEC) + 1u);
    setuper1();
}

/* PERFERAS +4 -- p. 388. LENGTHOT left: this job ends (SLEEPIE). Run out:
 * the drift test is over, and SETUPER1 torques the platform; first, but
 * for TORQNDX +0, LOSVEC +1 keeps CDUX. */
void perferas_after_atiginc(void)
{
    switch (ccs_at(E(LENGTHOT))) {
    case POSITIVE:
        tc_at(PERFERAS + 5u);
        continue_at(SLEEPIE);
        return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: keep_cdux(); return;     /* TCF +2 */
    case IS_MINUS_ZERO: setuper1(); return; /* TC SETUPER1 */
    }
    switch (ccs_at(E(TORQNDX))) {
    case POSITIVE: /* TCF +2 */
    case NEGATIVE: keep_cdux(); return;
    case IS_PLUS_ZERO: setuper1(); return; /* TC SETUPER1 */
    case IS_MINUS_ZERO:
        ts(E(LOSVEC) + 1u); /* (A +0) */
        setuper1();
        return;
    }
}

/* SETUPER1 +1 -- p. 388. "ANGLES FROM DRIFT TEST ONLY": (ANGX, ANGY,
 * ANGZ) negated, by GEORGEJ, through XSM, halved: OGC, the gyro torques
 * that take the platform back by the angles found. */
static const word setuper1_words[] = {
    ORDERS(DLOAD, PDDL), /* "ANGLES FROM DRIFT TEST ONLY" */
    ADDRESS_PLUS_ONE(ANGZ),
    ADDRESS_PLUS_ONE(ANGY),
    ORDERS(PDDL, VDEF),
    ADDRESS_PLUS_ONE(ANGX),
    ORDERS(VCOMP, VXSC),
    ADDRESS_PLUS_ONE(GEORGEJ),
    ORDERS(MXV, VSR1),
    ADDRESS_PLUS_ONE(XSM),
    STORE(OGC),
    ORDER(EXIT),
};

/* SETUPER1 +014 -- p. 388. Torque the gyros by OGC (IMUPULSE, its ECADR
 * OGCPL), and wait for the IMU. */
void setuper1_after_exit(void)
{
    set_A(OGCPL);
    tc_at(SETUPER1 + 015u);
    continue_at(BANKCALL); /* CADR IMUPULSE */
}

void setuper1_after_imupulse(void)
{
    tc_at(SETUPER1 + 017u);
    continue_at(IMUSLLLG);
}

/* GEOSTRT4 -- p. 388. "ONLY POSITIVE IF IN VERTICAL DRIFT TEST": then
 * show the drift (VALMIS). +0: the earth rate again (ERTHRVSE), and
 * TORQUE. (Negative would take the next words, interpretive code.) */
void geostrt4(void)
{
    switch (ccs_at(E(TORQNDX))) {
    case POSITIVE:
        tc_at(GEOSTRT4 + 1u);
        continue_at(VALMIS);
        return;
    case IS_PLUS_ZERO:
        tc_at(GEOSTRT4 + 2u);
        intpret();
        return;
    default: not_modelled(); return;
    }
}

static const word geostrt4_words[] = {
    ORDER(CALL),
    ADDRESS(ERTHRVSE),
    ORDER(EXIT),
};

void geostrt4_after_exit(void)
{
    tc_at(GEOSTRT4 + 6u);
    continue_at(TORQUE);
}

/* SLEEPIE -- p. 388. "TEST NOT OVER-DECREMENT LENGTHOT". Then "ARE WE
 * DOING VERTDRIFT": TORQNDX positive, EARTHR* first; else the job ends.
 * TORQNDX negative: SOMEERRR. */
void sleepie(void)
{
    ts(E(LENGTHOT));
    sleepie_plus1();
}

void sleepie_plus1(void)
{
    switch (ccs_at(E(TORQNDX))) {
    case POSITIVE:
        tc_at(SLEEPIE + 2u);
        continue_at(EARTHR_STAR);
        return;
    case IS_PLUS_ZERO: sleepie_after_earthr_star(); return;
    case NEGATIVE: someerrr(); return;
    case IS_MINUS_ZERO: someerrr_plus1(); return;
    }
}

void sleepie_after_earthr_star(void)
{
    tc_at(SLEEPIE + 3u);
    continue_at(ENDOFJOB);
}

/* SOMEERRR -- p. 388. Alarm 1600, "OVERFLOW IN DRIFT TEST": OVFLOWCK 1
 * "STOP ALLOOP FROM CALLING ITSELF", and end the test. */
void someerrr(void)
{
    set_A(EBANK5);
    someerrr_plus1();
}

void someerrr_plus1(void)
{
    ts(EBANK_ADDRESS);
    set_A(ONE);
    ts(E(OVFLOWCK));
    tc_at(SOMEERRR + 4u);
    continue_at(ALARM); /* OCT 1600 */
}

void someerrr_after_alarm(void)
{
    tc_at(SOMEERRR + 6u);
    continue_at(ENDTEST1);
}

/* SOMERR2 -- p. 388. Alarm 1601, "BAD IMU MODING IN ANY ROUTINE THAT USES
 * IMUSTALL": IMUSE clear, and the job ends. */
void somerr2(void)
{
    set_A(OCT1601);
    tc_at(SOMERR2 + 1u);
    continue_at(VARALARM);
}

void somerr2_after_varalarm(void)
{
    tc_at(SOMERR2 + 2u);
    continue_at(DOWNFLAG); /* ADRES IMUSE */
}

void somerr2_after_downflag(void)
{
    tc_at(SOMERR2 + 4u);
    continue_at(ENDOFJOB);
}

/* ---- The interpretive tables, by where each lies in bank 37 ---- */

#define CODE(name, first) {037u, (first), COUNT(name), name}
static const struct interpretive_code latazchk_code = CODE(latazchk_words, LATAZCHK);
static const struct interpretive_code posgmbl_code = CODE(posgmbl_words, LATAZCHK + 017u);
static const struct interpretive_code aingotn_code = CODE(aingotn_words, AINGOTN - 6u);
static const struct interpretive_code overffix_code = CODE(overffix_words, OVERFFIX);
static const struct interpretive_code erthrvse_code = CODE(erthrvse_words, ERTHRVSE);
static const struct interpretive_code earthr_star_code = CODE(earthr_star_words, EARTHR_STAR + 3u);
static const struct interpretive_code estims_code = CODE(estims_words, ESTIMS + 016u);
static const struct interpretive_code normlop_code = CODE(normlop_words, NORMLOP + 1u);
static const struct interpretive_code setuper1_code = CODE(setuper1_words, SETUPER1 + 1u);
static const struct interpretive_code geostrt4_code = CODE(geostrt4_words, GEOSTRT4 + 3u);

const struct interpretive_code *const IMU_PERFORMANCE_CODES[] = {
    &latazchk_code,    &posgmbl_code, &aingotn_code, &overffix_code, &erthrvse_code,
    &earthr_star_code, &estims_code,  &normlop_code, &setuper1_code, &geostrt4_code,
};
const unsigned IMU_PERFORMANCE_CODE_COUNT = COUNT(IMU_PERFORMANCE_CODES);

#endif
