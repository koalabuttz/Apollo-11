/* SPDX-License-Identifier: MIT */
/* IMU_CALIBRATION_AND_ALIGNMENT.agc of the CM, the basic code of pages
 * 441-446: the end of the drift test and the gyrocompassing.
 *
 * At the end of each drift test PERFERAS runs the erasable program "THAT
 * DOES THE CALCULATIONS" (LAT(SPL), loaded before the test), and ONCEMORE
 * either waits for the next pass or torques the platform by the drift
 * found (TORQINCH) and goes on to a PIPA test and the display (TORQUE).
 * GTSCPSS, called by
 * V37 for P01, starts the prelaunch alignment: the same loop as the drift
 * test (ESTIMS, ALLOOP, ALFLT) with GEOCOMP1 one, ALFLT erecting and
 * gyrocompassing the platform every half second. ENDGTSAL, every five
 * seconds, torques in what the filter found (RESTAIER), and a new launch
 * azimuth (RESTEST3, RESCHNG) restarts the erection. The faithful C is
 * c64-port/src/cm_imu_calibration.c.
 *
 * Every word is in bank 33. The erasable words are EBANK 5's but those of
 * the unswitched banks (CDUX, 1/PIPADT, LGYRO). The interpretive code
 * among the basic (SETUPER1, POSN17C, LASTGTS ...) the faithful port runs. */
#include "imu_calibration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

/* ---- The drift test's end: PERFERAS to TORQINCH -- pp. 441-442 ---- */

/* PERFERAS +1 -- p. 441, after the EXIT: "GO TO ERASABLE ONLY TO
 * RETURN", the erasable program at LAT(SPL) in EBANK 7. */
void perferas_after_exit(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_at(PERFERAS + 3u);
    continue_at(LAT_SPL);
}

/* SETUPER1 -- p. 441. "DRIFT TEST OVER": the interpreter turns the
 * angles the drift test found (ANGX, ANGY, ANGZ) into the gyro torquing
 * command OGC, then TORQINCH. */
static void setuper1(void)
{
    tc_at(SETUPER1);
    intpret();
}

/* ONCEMORE +5: "FOR TROUBLESHOOTING POSNS 2$4 VD", CDUX into LOSVEC +1. */
void oncemore_cdux(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(E(LOSVEC) + 1u);
    setuper1();
}

/* ONCEMORE +2 -- p. 441. In the vertical drift test (TORQNDX) the X CDU
 * angle is kept too. */
void oncemore_torqndx(void)
{
    switch (ccs_at(E(TORQNDX))) {
    case POSITIVE:
    case NEGATIVE: oncemore_cdux(); return; /* TCF +2 */
    case IS_PLUS_ZERO:
        tc_at(ONCEMORE + 4u);
        setuper1();
        return;
    case IS_MINUS_ZERO: /* TS LOSVEC +1 */
        ts(E(LOSVEC) + 1u);
        setuper1();
        return;
    }
}

/* ONCEMORE -- p. 441. LENGTHOT, the passes left: "TEST NOT OVER SET UP
 * NEXT WAITLIST" (SLEEPIE). */
void oncemore(void)
{
    switch (ccs_at(E(LENGTHOT))) {
    case POSITIVE:
        tc_at(ONCEMORE + 1u);
        sleepie();
        return;
    case IS_PLUS_ZERO: oncemore_torqndx(); return;
    case NEGATIVE: oncemore_cdux(); return;
    case IS_MINUS_ZERO:
        tc_at(ONCEMORE + 4u);
        setuper1();
        return;
    }
}

/* TORQINCH -- p. 441. The phase (PHASCHNG, OCT 00005). */
void torqinch(void)
{
    tc_at(TORQINCH);
    continue_at(PHASCHNG);
}

/* TORQINCH +2: torque the gyros by OGC (IMUPULSE, its ECADR in A). */
void torqinch_after_phaschng(void)
{
    set_A(OGCPL);
    tc_at(TORQINCH + 3u);
    continue_at(BANKCALL); /* CADR IMUPULSE */
}

/* TORQINCH +5: wait for the gyros. */
void torqinch_after_imupulse(void)
{
    tc_at(TORQINCH + 5u);
    imustllg();
}

/* TORQINCH +6 -- p. 441. "+ IF IN VERTICAL DRIFT TEST": "VERT DRIFT
 * TEST OVER", VALMIS shows the drift. Otherwise the interpreter: "SET UP
 * ERATE FOR PIP TEST OR COMPASS" (ERTHRVSE). */
void torqinch_after_imustllg(void)
{
    switch (ccs_at(E(TORQNDX))) {
    case POSITIVE:
        tc_at(TORQINCH + 7u);
        valmis();
        return;
    case IS_PLUS_ZERO: torqinch_erate(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); return; /* (CALL, ERTHRVSE: data words) */
    }
}

/* TORQINCH +010: the interpreter's CALL ERTHRVSE. */
void torqinch_erate(void)
{
    tc_at(TORQINCH + 010u);
    intpret();
}

/* TORQINCH +014 -- p. 442. "GO TO IMU2 FOR A PIPA TEST AND DISPLAY". */
void torqinch_after_exit(void)
{
    tc_at(TORQINCH + 014u);
    torque();
}

/* ---- The gyrocompassing's start: GTSCPSS -- pp. 442-443 ---- */

/* SETUPGC -- p. 443. Clear the 18 words XSM to ZSM +5 (DEC17, the count
 * less one) with ZEROING, which comes back below. */
static void setupgc(void)
{
    set_A(DEC17);
    ts(E(ZERONDX1));
    set_A(XSMADR);
    tc_at(SETUPGC + 3u);
    zeroing();
}

/* GTSCPSSA -- p. 443. "THIS IS THE LEAD IN FOR COMPASS": GEOCOMP1 one,
 * the PIPA scale (1/PIPAGT), LENGTHOT BIT8, "COMPASS IS A .5 SEC LOOP"
 * (1SECXT1), PREMTRX1 one and PERFDLAY (the settling time) 1 cs, and the
 * launch azimuth LUNCHAZ1 as the new azimuth and the old. */
static void gtscpssa(void)
{
    set_A(ONE);
    ts(E(GEOCOMP1));
    set_A(ONE_PIPAGT);
    ts(ADDRESS_OF(1_PIPADT));
    set_A(BIT8); /* NXXTENN */
    ts(E(LENGTHOT));
    set_A(HALF_SEC); /* 1/2SECX */
    ts(E(1SECXT1));
    set_A(ONE);
    ts(E(PREMTRX1));
    ts(E(PERFDLAY) + 1u);
    set_A(ZERO);
    ts(E(PERFDLAY));
    dca(E(LUNCHAZ1));
    dxch(E(NEWAZ1));
    dca(E(LUNCHAZ1));
    dxch(E(OLDAZMTH));
    setupgc();
}

/* GTSCPSS -- p. 442, "CALLED BY V37" for P01. "NODO ALARM FOR P01 - P11
 * ALREADY DONE" (NOP01BIT of FLAGWRD1): POODOO 1521. */
void gtscpss(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(NOP01BIT);
    if (A_is_zero()) { /* EXTEND; BZF GTSCPSSA */
        gtscpssa();
        return;
    }
    tc_at(GTSCPSS + 4u);
    continue_at(POODOO); /* OCT 1521 */
}

/* SETUPGC +4 -- p. 443. The matrix (POSN17C), and "GO TO IMU2 FOR
 * FURTHER INITIALIZATION" (GEOIMUTT). */
void setupgc_after_zeroing(void)
{
    tc_at(SETUPGC + 4u);
    posn17c();
}

/* SETUPGC +5. */
void setupgc_after_posn17c(void)
{
    tc_at(SETUPGC + 5u);
    geoimutt();
}

/* POSN17C -- p. 443. "COMPASS POSITION Z DOWN,X DOWNRANGE": the return
 * into QPLACE, ZSM minus one half, and the interpreter: XSM and YSM from
 * the azimuth NEWAZ1 ("FROM NORTH IN REVOLUTIONS + CLOCKWISE"). */
void posn17c(void)
{
    qxch(E(QPLACE));
    set_A(negate(HALF));
    ts(E(ZSM));
    tc_at(POSN17C + 4u);
    intpret();
}

/* POSN17C +017: back through QPLACE. */
void posn17c_after_exit(void)
{
    tc_return(E(QPLACE));
}

/* ---- The gyrocompassing's torquing: ENDGTSAL to NOCHORLD --
 * pp. 445-446 ---- */

/* LASTGTS -- p. 445. The interpreter: THETAX1 the earth rate ERCOMP1,
 * ALK the time TMARK; then RESTAIER. */
static void lastgts(void)
{
    tc_at(LASTGTS);
    intpret();
}

/* ENDGTSAL +3 -- p. 445. "YES BUT ARE GYROS BUSY" (LGYRO): "BUSY-GET THEM
 * .5 SECONDS FROM NOW". */
void endgtsal_after_chkcomed(void)
{
    switch (ccs_at(ADDRESS_OF(LGYRO))) {
    case POSITIVE: sleepie_plus1(); return; /* TCF SLEEPIE +1 */
    case IS_PLUS_ZERO: lastgts(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); return; /* (VLOAD, ERCOMP1: data words) */
    }
}

/* ENDGTSAL +2: has the alignment ended (CHKCOMED)? */
void endgtsal_chkcomed(void)
{
    tc_at(ENDGTSAL + 2u);
    chkcomed();
}

/* ENDGTSAL -- p. 445. "IS 5 SEC OVER-THE TIME TO TORQ PLATFORM": not
 * yet, "SET UP NEXT WAITLIST CALL FOR .5 SEC" (SLEEPIE). */
void endgtsal(void)
{
    switch (ccs_at(E(LENGTHOT))) {
    case POSITIVE:
        tc_at(ENDGTSAL + 1u);
        sleepie();
        return;
    case IS_PLUS_ZERO: endgtsal_chkcomed(); return;
    case NEGATIVE: endgtsal_after_chkcomed(); return;
    case IS_MINUS_ZERO: sleepie_plus1(); return; /* TCF SLEEPIE +1 */
    }
}

/* RESTAIER -- p. 445. The phase (PHASCHNG, OCT 00275). */
void restaier(void)
{
    tc_at(RESTAIER);
    continue_at(PHASCHNG);
}

/* RESTAIER +2: the interpreter, "ADD COMPASS COMMANDS INTO ERATE":
 * ERCOMP1 = XSM THETAN1 + THETAX1. */
void restaier_after_phaschng(void)
{
    tc_at(RESTAIER + 2u);
    intpret();
}

/* RESTAIER +014: "TORQUE IT ALL IN" (EARTHR*). */
void restaier_after_exit(void)
{
    tc_at(RESTAIER + 014u);
    earthr_star();
}

/* RESTAIER +015: GEOSAVE1 the erection time left (ERECTIM1), and the
 * phase (PHASCHNG, OCT 00155). */
void restaier_after_earthr(void)
{
    ca(E(ERECTIM1));
    ts(E(GEOSAVE1));
    tc_at(RESTAIER + 017u);
    continue_at(PHASCHNG);
}

/* RESTEST1 -- p. 445. The interpreter clears THETAN1. */
void restest1(void)
{
    tc_at(RESTEST1);
    intpret();
}

/* RESTEST1 +7: the phase (PHASCHNG, OCT 00255), then RESTEST3. */
void restest1_phaschng(void)
{
    tc_at(RESTEST1 + 7u);
    continue_at(PHASCHNG);
}

/* RESTEST1 +5: PREMTRXC set, NOCHORLD; +0, a check of the azimuth. */
void restest1_after_exit(void)
{
    switch (ccs_at(E(PREMTRXC))) {
    case POSITIVE:
        tc_at(RESTEST1 + 6u);
        nochorld();
        return;
    case IS_PLUS_ZERO: restest1_phaschng(); return;
    case NEGATIVE: not_modelled(); return; /* (OCT 00255, data) */
    case IS_MINUS_ZERO: restest3(); return;
    }
}

/* RESTEST3 -- p. 445. The interpreter: has the launch azimuth LAUNCHAZ
 * changed from OLDAZMTH? If not, NOAZCHGE; if so NEWAZMTH it, and
 * ADERCOMP. */
void restest3(void)
{
    tc_at(RESTEST3);
    intpret();
}

/* RESTEST3 +020 -- p. 446. The matrix for the new azimuth (POSN17C). */
void restest3_after_exit(void)
{
    tc_at(RESTEST3 + 020u);
    posn17c();
}

/* RESTEST3 +021: the phase (PHASCHNG, OCT 00335). */
void restest3_after_posn17c(void)
{
    tc_at(RESTEST3 + 021u);
    continue_at(PHASCHNG);
}

/* RESCHNG -- p. 446. OLDAZMTH the new azimuth; "SPEND 320 SEC ERECTING"
 * (LENGTHOT BIT7), and the phase (PHASCHNG, OCT 00075). */
void reschng(void)
{
    dca(E(NEWAZMTH));
    dxch(E(OLDAZMTH));
    set_A(BIT7);
    ts(E(LENGTHOT));
    tc_at(RESCHNG + 5u);
    continue_at(PHASCHNG);
}

/* SPITGYRO -- p. 446. Torque the gyros by ERCOMP (IMUPULSE). */
void spitgyro(void)
{
    set_A(ERCOMPPL);
    tc_at(SPITGYRO + 1u);
    continue_at(BANKCALL); /* CADR IMUPULSE */
}

/* SPITGYRO +3: wait for them (IMUSTALL). */
void spitgyro_after_imupulse(void)
{
    tc_at(SPITGYRO + 3u);
    continue_at(BANKCALL); /* CADR IMUSTALL */
}

/* SPITGYRO +5: IMUSTALL's error return, SOMERR2 (alarm 1601). */
void spitgyro_bad_stall(void)
{
    tc_at(SPITGYRO + 5u);
    somerr2();
}

/* SPITGYRO +6: "RE-INITIALIZE" (ESTIMS). */
void spitgyro_after_imustall(void)
{
    tc_at(SPITGYRO + 6u);
    estims();
}

/* NOCHORLD -- p. 446. ERECTIM1 "COUNTS DOWN FOR ERECTION" from GEOSAVE1;
 * then ANNNNNN. */
void nochorld(void)
{
    switch (ccs_at(E(GEOSAVE1))) {
    case POSITIVE:
        ts(E(ERECTIM1));
        annnnnn();
        return;
    case IS_PLUS_ZERO: annnnnn(); return;
    case NEGATIVE: ts(E(LENGTHOT)); break; /* ANNNNNN +1 */
    case IS_MINUS_ZERO: break;             /* ANNNNNN +2 */
    }
    tc_at(ANNNNNN + 2u);
    sleepie_plus1();
}

/* NOAZCHGE +1 -- p. 446, after the EXIT: no new azimuth. PREMTRXC one,
 * and NOCHORLD. */
void noazchge_after_exit(void)
{
    set_A(ONE);
    ts(E(PREMTRXC));
    nochorld();
}

#endif
