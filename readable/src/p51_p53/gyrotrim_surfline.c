/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 8 (Luminary099 pp. 968-974, bank 15): GYROTRIM, the
 * gyro torque to a set of CDU angles; P57's star sightings (1STAR,
 * R59RET, ASTAR); and the lunar surface alignment (SURFLINE to P57POST,
 * COATRIM), coarse or fine.
 *
 * GYROTRIM "COMPUTES AND SENDS GYRO COMMANDS WHICH CAUSE THE CDUS" to
 * reach THETAD. P57 sights two stars through the AOT (R59); SURFLINE
 * takes each star's catalog vector to the desired stable member (XSMD)
 * and, with the sightings, R54 checks them and AXISGEN and CALCGTA give
 * the gyro torque angles. Under 5 degrees, GYROTRIM torques the gyros
 * (JUSTTRIM); more, COATRIM coarse aligns first. SURFDISP makes XSMD
 * REFSMMAT; P57POST asks the crew to align again, and after option 2 the
 * landing site follows from gravity (GSAV).
 *
 * Each call out of the file (BANKCALL, CURTAINS, PHASCHNG, GOTOPOOH)
 * leaves Z at the routine and Q at the word after the TC, where the
 * routine comes back to; GOTOPOOH doesn't come back. The faithful port
 * keys a routine at each such return, at each TC's return in the file,
 * and at each TC INTPRET's NEWOPS. */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void astar(void);
static void one_star_plus1(void);
static void five_deg_test(void);
static void b2f8(void);
static void b2f8_plus1(void);

/* ---- GYROTRIM -- pp. 968-969 ---- */

/* GYROTRIM: the caller's return to QMIN; the stable member at THETAD in
 * navigation base axes (TRG*NBSM, *NBSM*: its X and Y axes to STARAD
 * and STARAD +6), and the navigation base in the present stable member
 * (CDUTRIG, CALCSMSC: XNB, YNB); AXISGEN makes the one in the other, and
 * CALCGTA gives the gyro torque angles (OGC, IGC, MGC). */
static const word gyrotrim_words[] = {
    /* GYROTRIM */ ORDERS(STQ, DLOAD),
    ADDRESS(QMIN),
    ADDRESS_PLUS_ONE(THETAD),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE_AT(THETAD, 2),
    ADDRESS_PLUS_ONE_AT(THETAD, 1),
    ORDER(VDEF),
    STOVL(CDUSPOT),
    ADDRESS_PLUS_ONE(XUNIT),
    ORDER(CALL),
    ADDRESS(TRG_STAR_NBSM),
    STOVL(STARAD),
    ADDRESS_PLUS_ONE(YUNIT),
    ORDER(CALL),
    ADDRESS(STAR_NBSM_STAR),
    STCALL_AT(STARAD, 6),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(CALCSMSC),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(XNB),
    STOVL_D(6),
    ADDRESS_PLUS_ONE(YNB),
    STCALL_D(12),
    ADDRESS(AXISGEN),
    ORDER(CALL),
    ADDRESS(CALCGTA),
    /* JUSTTRIM */ ORDER(EXIT),
};

const struct interpretive_code GYROTRIM_CODE = {015u, GYROTRIM, COUNT(gyrotrim_words),
                                                gyrotrim_words};

/* JUSTTRIM +1, after the EXIT: the IMU fine aligned (IMUFINE), IMUSTALL
 * waiting (CURTAINS when it fails); the gyros torqued by OGC's angles
 * (IMUPULSE, GYRCDR), IMUSTALL waiting again; back to the caller
 * (QMINEXIT). */
void justtrim_after_exit(void)
{
    tc_to(JUSTTRIM + 1u, BANKCALL); /* CADR IMUFINE */
}

void justtrim_after_imufine(void)
{
    tc_to(JUSTTRIM + 3u, BANKCALL); /* CADR IMUSTALL */
}

void justtrim_fine_failed(void)
{
    tc_to(JUSTTRIM + 5u, CURTAINS);
}

void justtrim_torque(void)
{
    set_A(GYRCDR);
    tc_to(JUSTTRIM + 7u, BANKCALL); /* CADR IMUPULSE */
}

void justtrim_after_imupulse(void)
{
    tc_to(JUSTTRIM + 011u, BANKCALL); /* CADR IMUSTALL */
}

void justtrim_torque_failed(void)
{
    tc_to(JUSTTRIM + 013u, CURTAINS);
}

void justtrim_return(void)
{
    qminexit(); /* TCF QMINEXIT */
}

/* ---- The star sightings -- p. 970 ---- */

/* 2STARS: "INITIALIZE STARIND", "ZERO FOR 1ST STAR, ONE FOR 2ND STAR";
 * 1STAR: STARIND 1, the second star. Then a restart point (PHASCHNG,
 * 04024), and R59, "GO DO STAR ACQUIRE AND AOTMARK". (A is a constant:
 * TS does not skip.) */
void two_stars(void)
{
    set_A(ZERO);
    one_star_plus1(); /* TCF +2 */
}

void one_star(void)
{
    set_A(BIT1);
    one_star_plus1();
}

static void one_star_plus1(void)
{
    ts(ADDRESS_OF(STARIND));
    tc_to(ONE_STAR + 2u, PHASCHNG); /* OCT 04024 */
}

void one_star_r59(void)
{
    r59(); /* TCF R59 */
}

/* R59RET, "BACK FROM SURFACE MARKING": after the first star, ASTAR; after
 * the second, a restart point (PHASCHNG, 04024), and its catalog vector
 * at the time of its mark (PLANET) to VEC2, then SURFLINE. */
void r59ret(void)
{
    ca(ADDRESS_OF(STARIND));
    if (A_is_zero()) { /* EXTEND; BZF ASTAR */
        astar();
        return;
    }
    tc_to(R59RET + 3u, PHASCHNG); /* OCT 04024 */
}

void r59ret_second_star(void)
{
    tc_at(R59RET + 5u);
    intpret();
}

static const word r59ret_words[] = {
    /* R59RET +6 */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT), /* "TIME OF 2ND MARK" */
    ADDRESS(PLANET),
    STCALL(VEC2), /* "STORE 2ND CATALOG VEC (REF)" */
    ADDRESS(SURFLINE),
};

const struct interpretive_code R59RET_CODE = {015u, R59RET + 6u, COUNT(r59ret_words), r59ret_words};

/* ASTAR: the first star as sighted (STARAD +6) to STARSAV1, its catalog
 * vector at the time of its mark (PLANET) to VEC1; then the second star
 * (1STAR). */
static void astar(void)
{
    tc_at(ASTAR);
    intpret();
}

static const word astar_words[] = {
    /* ASTAR +1 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(STARAD, 6),
    STORE(STARSAV1), /* "1ST OBSERVED STAR (SM)" */
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT), /* "TIME OF 1ST MARK" */
    ADDRESS(PLANET),
    STORE(VEC1), /* "STORE 1ST CATALOG VEC (REF)" */
    ORDER(EXIT),
};

const struct interpretive_code ASTAR_CODE = {015u, ASTAR + 1u, COUNT(astar_words), astar_words};

void astar_after_exit(void)
{
    one_star(); /* TCF 1STAR */
}

/* ---- The surface alignment -- pp. 971-973 ---- */

/* SURFLINE: for each star (X2 from 12D, by S2, 6), its catalog vector in
 * the desired stable member (VEC1 by XSMD, to STARAD +12D) and its
 * sighting in the present one (STARSAV1, to the work area's 18D). On the
 * initial pass (INITALGN) "BYPASS R54"; else R54 checks the pair, and
 * without FREEFLAG back to P57POST. Then AXISGEN and CALCGTA: the gyro
 * torque angles (OGC, IGC, MGC). */
static const word surfline_words[] = {
    /* SURFLINE */ ORDERS(SSP, AXT_2),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(6),
    NUMBER(12),
    /* WRTDESIR */ ORDERS(VLOAD_STAR, MXV),
    ADDRESS_PLUS_ONE_AT_X2(VEC1, 12),
    ADDRESS_PLUS_ONE(XSMD),
    ORDER(UNIT),
    STORE_AT_X2(STARAD, 12), /* "VEC IN SM" */
    ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(STARSAV1, 12), /* "PICK UP VEC IN PRESENT SM" */
    STORE_X2_D(18),
    ORDERS(TIX_2, BON),
    ADDRESS(WRTDESIR),
    SWITCH(BON, INITALGN),
    ADDRESS(INITBY),
    /* DOALIGN */ ORDER(CALL),
    ADDRESS(R54), /* "DO CHKSDATA" */
    ORDER(BOFF),
    SWITCH(BOFF, FREEFLAG),
    ADDRESS(P57POST), /* "ASTRO DOES NOT LIKE DATA TEST RESULTS" */
    /* INITBY */ ORDER(CALL),
    ADDRESS(AXISGEN),
    ORDER(CALL),
    ADDRESS(CALCGTA), /* "GET GYRO TORQ ANGLES, OGC,IGC,MGC" */
    ORDER(EXIT),
};

const struct interpretive_code SURFLINE_CODE = {015u, SURFLINE, COUNT(surfline_words),
                                                surfline_words};

/* INITBY +5, after the EXIT: "IF INITIAL PASS BYPASS NOUN 93 DISPLAY";
 * else the angles flashed (DISPGYRO, V06N93): terminate, proceed (5DEGTEST)
 * or recycle ("MAYBE RE-ALIGN", P57POST +1). (After the MASK, A is +0
 * or the bit: CCS takes its first two ways only.) */
void initby_after_exit(void)
{
    set_A(INITABIT);
    mask_at(ADDRESS_OF(FLAGWRD8));
    switch (ccs_register(A_ADDRESS)) {     /* CCS A */
    case POSITIVE: five_deg_test(); break; /* TCF 5DEGTEST */
    case IS_PLUS_ZERO:
        set_A(DISPGYRO);
        tc_to(INITBY + 012u, BANKCALL); /* CADR GOFLASH */
        break;
    default: not_modelled();
    }
}

void initby_terminate(void)
{
    tc_to(INITBY + 014u, GOTOPOOH); /* "V34 -- TERMINATE" */
}

void initby_proceed(void)
{
    five_deg_test(); /* TCF 5DEGTEST */
}

void initby_recycle(void)
{
    p57post_plus1(); /* TCF P57POST +1 */
}

/* 5DEGTEST: "IF ANGLES GREATER THAN 5 DEGS, DO COARSE" (COATRIM): the
 * angles (OGC) to OGCT, scaled by 5DEGREES; less, and with SURFDISP as
 * the return (QMIN), the gyros torqued (JUSTTRIM). */
static void five_deg_test(void)
{
    tc_at(FIVE_DEG_TEST);
    intpret();
}

static const word five_deg_test_words[] = {
    /* 5DEGTEST +1 */ ORDERS(VLOAD, BOV),
    ADDRESS_PLUS_ONE(OGC),
    ADDRESS(SURFSUP),
    /* SURFSUP */ STORE(OGCT),
    ORDERS(V_SC, BOV),
    ADDRESS_PLUS_ONE(5DEGREES),
    ADDRESS(COATRIM),
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(QMIN),
    ADDRESS(SURFDISP),
    ADDRESS(JUSTTRIM), /* "ANGLES LESS THAN 5 DEG, DO GYRO TORQ" */
    /* SURFDISP */ ORDER(EXIT),
};

const struct interpretive_code FIVE_DEG_TEST_CODE = {
    015u, FIVE_DEG_TEST + 1u, COUNT(five_deg_test_words), five_deg_test_words};

/* SURFDISP +1, after the EXIT: a restart point (PHASCHNG, 04024); the
 * desired stable member (XSMD) to REFSMMAT (MATMOVE), REFSMFLG set. */
void surfdisp_after_exit(void)
{
    tc_to(SURFDISP + 1u, PHASCHNG); /* OCT 04024 */
}

void surfdisp_after_phaschng(void)
{
    tc_at(SURFDISP + 3u);
    intpret();
}

static const word surfdisp_words[] = {
    /* SURFDISP +4 */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XSMD),
    ADDRESS(REFSMMAT),
    ORDERS(SET, CALL),
    SWITCH(SET, REFSMFLG),
    ADDRESS(MATMOVE),
    ORDER(EXIT),
};

const struct interpretive_code SURFDISP_CODE = {015u, SURFDISP + 4u, COUNT(surfdisp_words),
                                                surfdisp_words};

/* After the EXIT: "IF OPTION ZERO DO FINISH" (P57POST +1); else B2F8.
 * (CCS of -0 leaves A +0 at B2F8 +1, after the CAF.) */
void surfdisp_option(void)
{
    switch (ccs_at(ADDRESS_OF(OPTION2))) {
    case POSITIVE: b2f8(); break; /* TCF B2F8 */
    case IS_PLUS_ZERO: p57post_plus1(); break;
    case NEGATIVE: b2f8(); break;
    case IS_MINUS_ZERO: b2f8_plus1(); break;
    }
}

/* B2F8: "IF INITIAL FLAG SET, RE-CYCLE" (P57JUMP); else the LM's attitude
 * in moon-fixed axes (REFMF), then P57POST. (After the MASK, A is +0 or
 * the bit: CCS takes its first two ways only.) */
static void b2f8(void)
{
    set_A(INITABIT);
    b2f8_plus1();
}

static void b2f8_plus1(void)
{
    mask_at(ADDRESS_OF(FLAGWRD8));
    switch (ccs_register(A_ADDRESS)) { /* CCS A */
    case POSITIVE: p57jump(); break;   /* TCF P57JUMP */
    case IS_PLUS_ZERO:
        tc_at(B2F8 + 4u);
        intpret();
        break;
    default: not_modelled();
    }
}

static const word b2f8_words[] = {
    /* B2F8 +5 */ ORDER(CALL),
    ADDRESS(REFMF), /* "GO GET ATTITUDE VEC IN MF(YNBSAV,XNBSAV)" */
    /* P57POST */ ORDER(EXIT),
};

const struct interpretive_code B2F8_CODE = {015u, B2F8 + 5u, COUNT(b2f8_words), b2f8_words};

/* P57POST +1, after the EXIT: "DISPLAY V50N25 CHK CODE 14" (GOPERF1):
 * terminate, proceed ("TO RE-ALIGN", P57JUMP) or enter: after option 2,
 * a restart point (PHASCHNG, 04024) and the landing site; else out of
 * P57. */
void p57post_plus1(void)
{
    set_A(OCT14);
    tc_to(P57POST + 2u, BANKCALL); /* CADR GOPERF1 */
}

void p57post_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF GOTOPOOH */
}

void p57post_proceed(void)
{
    p57jump(); /* TCF P57JUMP */
}

void p57post_enter(void)
{
    set_A(negate(BIT2)); /* CS BIT2 */
    ad_at(ADDRESS_OF(OPTION2));
    if (A_is_zero()) {                   /* EXTEND; BZF +2 */
        tc_to(P57POST + 013u, PHASCHNG); /* OCT 04024 */
        return;
    }
    continue_at(GOTOPOOH); /* "NO -- EXIT P57" */
}

void p57post_site(void)
{
    tc_at(P57POST + 015u);
    intpret();
}

/* The landing site: gravity (GSAV, in navigation base axes) in the
 * stable member (CDU*NBSM) and in the reference frame (REFSMMAT), times
 * the landing site's radius (RLS) to ALPHAV; N89DISP then gives the
 * landing site (RLS, to RN), and R-TO-RP turns it to moon-fixed axes at
 * N89DISP's time (to PIPTIME), "RLS IN MOON-FIXED COORDS". */
static const word p57post_site_words[] = {
    /* P57POST +016 */ ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(GSAV),
    ADDRESS(CDU_STAR_NBSM), /* "GO TO SM COORDS" */
    ORDERS(VXM, SET),
    ADDRESS_PLUS_ONE(REFSMMAT),
    SWITCH(SET, LUNAFLAG),
    ORDERS(PDVL, ABVAL),
    ADDRESS_PLUS_ONE(RLS),
    ORDERS(VXSC, STADR),
    AFTER_STADR(STORE(ALPHAV)), /* "ALPHAV = RLSMAG * G(REF)" */
    ORDERS(CLEAR, RTB),
    SWITCH(CLEAR, ERADFLAG),
    ADDRESS(LOADTIME),
    ORDER(CALL),
    ADDRESS(N89DISP),
    STORE(RN), /* "RN=RLS B-29 = LM POSITION" */
    ORDERS(VSL2, PDDL),
    ADDRESS_PLUS_ONE_AT(GDT_2, 4), /* "TIME TEMP STORED IN N89DISP" */
    ORDER(PUSH),
    STCALL(PIPTIME), /* "PIPTIME = LM STATE TIME" */
    ADDRESS(R_TO_RP),
    STORE(RLS),
    ORDER(EXIT),
};

const struct interpretive_code P57POST_SITE_CODE = {015u, P57POST + 016u, COUNT(p57post_site_words),
                                                    p57post_site_words};

void p57post_exit(void)
{
    continue_at(GOTOPOOH); /* "EXIT P57" */
}

/* ---- COATRIM -- p. 974 ---- */

/* COATRIM, "COARSE AND FINE ALIGN IMU": the desired orientation (XDC) to
 * XSM, and its gimbal angles (CDUTRIG, CALCSMSC, CALCGA); on the initial
 * pass straight on to CORSIT; else the angles flashed first. */
static const word coatrim_words[] = {
    /* COATRIM */ ORDERS(AXC_1, AXC_2),
    ADDRESS(XDC),
    ADDRESS(XSM),
    ORDER(CALL),
    ADDRESS(MATMOVE),
    ORDER(CALL),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(CALCSMSC),
    ORDER(CALL),
    ADDRESS(CALCGA),
    ORDERS(BOFF, EXIT),
    SWITCH(BOFF, INITALGN),
    ADDRESS(CORSIT),
};

const struct interpretive_code COATRIM_CODE = {015u, COATRIM, COUNT(coatrim_words), coatrim_words};

/* After the EXIT: the gimbal angles flashed (V06N22): terminate, proceed
 * (a restart point, PHASCHNG 04024, then CORSIT) or enter (again). */
void coatrim_after_exit(void)
{
    set_A(V06N22);
    tc_to(COATRIM + 017u, BANKCALL); /* CADR GOFLASH */
}

void coatrim_terminate(void)
{
    tc_to(COATRIM + 021u, GOTOPOOH);
}

void coatrim_proceed(void)
{
    tc_to(COATRIM + 024u, PHASCHNG); /* TCF +2; OCT 04024 */
}

void coatrim_enter(void)
{
    coatrim_after_exit(); /* TCF -5 */
}

void coatrim_after_phaschng(void)
{
    tc_at(COATRIM + 026u);
    intpret();
}

/* CORSIT: the coarse alignment (COARSE), the drift (NCOARSE) and the
 * gyro torque (GYROTRIM); then SURFDISP. */
static const word corsit_words[] = {
    /* CORSIT */ ORDER(CALL),
    ADDRESS(COARSE),
    ORDER(CALL),
    ADDRESS(NCOARSE),
    ORDER(CALL),
    ADDRESS(GYROTRIM),
    ORDER(GOTO),
    ADDRESS(SURFDISP),
};

const struct interpretive_code CORSIT_CODE = {015u, CORSIT, COUNT(corsit_words), corsit_words};

#endif
