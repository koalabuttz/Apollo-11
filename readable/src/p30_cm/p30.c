/* SPDX-License-Identifier: MIT */
/* P30-P37.agc (Comanche055 pp. 635-638 and 648): P30, the "EXTERNAL
 * DELTA-V TARGETTING PROGRAM", and P31, "GENERAL LAMBERT AIMPOINT
 * GUIDANCE" (bank 35); and AVFLAGA, AVFLAGP and P20FLGON (fixed-fixed).
 *
 * P30, as its program description has it, "ACCEPTS ASTRONAUT INPUTS OF
 * TIG,DELV(LV) AND COMPUTES,FOR DISPLAY, APOGEE,PERIGEE,DELV(MAG),MGA
 * ASSOCIATED WITH DESIRED MANEUVER". P31 computes the "REQUIRED VELOCITY
 * FOR MANEUVER" from a stored target and transfer time instead. Both
 * start "VIA JOB FROM V37" and end "VIA V37 OR GOTOPOOH". They share:
 *   P30/P31, which "DISPLAYS TIG" (verb 06 noun 33);
 *   CNTNUP30, which "DISPLAYS DELV(LV)" (verb 06 noun 81);
 *   PARAM30, which "DISPLAYS APOGEE,PERIGEE,DELV(MAG),MGA,TIME FROM TIG,
 *   MARKS SINCE LAST THRUSTING MANEUVER" (verb 06 noun 42, then +MGA by
 *   REFTEST and DISPMGA: verb 16 noun 45, with the time to go COMPTGO
 *   counts down).
 * Each saves its return (XCH Q; TS) and goes back through it: TC to the
 * erasable word, which the AGC runs as the instruction TC <return>.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, 2PHSCHNG, WAITLIST,
 * UPFLAG, DOWNFLAG) leaves Z at the routine and Q at the word after the
 * TC (after its ADRES, its 2CADR or its OCTs), where the routine comes
 * back to; ENDOFJOB and GOTOPOOH don't come back. A display through
 * GOFLASHR or GOFLASH comes back to the word after its CADR on terminate,
 * the next on proceed, the next on enter (recycle) and, for GOFLASHR, the
 * next on its immediate return. */
#include "p30_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC to an erasable word that holds a return: the AGC runs the word as an
 * instruction, a TC when the program has saved an address there. Any
 * other word (another instruction, or a register's address) is not
 * modelled. */
static void tc_return_in(unsigned address)
{
    word to = fetch(address);

    if (OPERATION_CODE(to) != 0u || to < 010u) not_modelled();
    tc_through(address);
}

static void end45(void);
static void p30_p31(void);
static void cntnup30(void);
static void param30(void);
static void reftest(void);

/* ---- DISPMGA, DISP45 and COMPTGO -- p. 635 ---- */

/* DISPMGA, "USED IN P30": its return in RGEXIT (STQ), then out of the
 * interpreter (EXIT) to COMPTGO and DISP45. */
static const word dispmga_words[] = {
    ORDERS(STQ, EXIT),
    ADDRESS(RGEXIT),
};

/* DISPMGA +2, after the EXIT: COMPTGO, which comes back to DISP45. */
void dispmga_after_exit(void)
{
    tc_at(DISPMGA + 2u);
    comptgo();
}

/* DISP45: verb 16 noun 45, flashed by GOFLASHR: the marks, the time from
 * ignition (counting, verb 16) and +MGA. */
void disp45(void)
{
    set_A(V16N45);
    tc_to(DISP45 + 1u, BANKCALL); /* CADR GOFLASHR */
}

/* DISP45 +3: terminate. */
void disp45_terminate(void)
{
    tc_to(DISP45 + 3u, GOTOPOOH);
}

/* DISP45 +4: proceed, to END45. */
void disp45_proceed(void)
{
    tc_at(DISP45 + 4u);
    end45();
}

/* DISP45 +5: recycle, the display again. */
void disp45_recycle(void)
{
    tc_at(DISP45 + 5u);
    disp45();
}

/* P30PHSI, GOFLASHR's immediate return: phase 4 of group 6 (PHASCHNG, OCT
 * 14), so that a restart shows the display again; and the job ends. */
void p30phsi(void)
{
    tc_to(P30PHSI, PHASCHNG); /* OCT 14 */
}

void p30phsi_after_phaschng(void)
{
    tc_to(P30PHSI + 2u, ENDOFJOB); /* TCR ENDOFJOB */
}

/* END45: into the interpreter, */
static void end45(void)
{
    tc_at(END45);
    intpret();
}

/* where the clock display stops (TIMRFLAG cleared) and DISPMGA returns
 * through RGEXIT. */
static const word end45_words[] = {
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, TIMRFLAG),
    ADDRESS(RGEXIT),
};

/* COMPTGO, "USED TO COMPUTE TTOGO": its return in PHSPRDT6 ("GROUP 6
 * TEMPORARY USED .. BEWARE"), then TIMRFLAG set. */
void comptgo(void)
{
    qxch(ADDRESS_OF(PHSPRDT6));  /* EXTEND; QXCH */
    tc_to(COMPTGO + 2u, UPFLAG); /* ADRES TIMRFLAG */
}

/* COMPTGO +4: NVWORD1 +0, and CLOKTASK, the clock's task, in 0.01
 * second (WAITLIST; A from a CAF has no overflow for the TS to skip on). */
void comptgo_after_timrflag(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(NVWORD1));
    set_A(ONE);
    tc_to(COMPTGO + 7u, WAITLIST); /* 2CADR CLOKTASK */
}

/* COMPTGO +012: "6.3SPOT FOR CLOKTASK" and "GROUP 4 CONTINUES HERE"
 * (2PHSCHNG, OCT 40036, OCT 05024, OCT 13000), */
void comptgo_after_waitlist(void)
{
    tc_to(COMPTGO + 012u, TWO_PHSCHNG);
}

/* COMPTGO +016: and back through PHSPRDT6. */
void comptgo_after_2phschng(void)
{
    tc_at(COMPTGO + 016u);
    tc_return_in(ADDRESS_OF(PHSPRDT6));
}

/* ---- P30 -- pp. 636-637 ---- */

/* P30: TIG, then DELV(LV), the crew's. */
void p30(void)
{
    tc_at(P30);
    p30_p31();
}

void p30_after_p30_p31(void)
{
    tc_at(P30 + 1u);
    cntnup30();
}

/* P30 +2: "RESET UPDATFLG". */
void p30_after_cntnup30(void)
{
    tc_to(P30 + 2u, DOWNFLAG); /* ADRES UPDATFLG */
}

/* P30 +4: to the interpreter, for P30_CODE. */
void p30_after_updatflg(void)
{
    tc_at(P30 + 4u);
    intpret();
}

/* P30 +5: S30.1, and out of the interpreter. */
static const word p30_words[] = {
    ORDER(CALL),
    ADDRESS(S30_1),
    ORDER(EXIT),
};

/* P30 +010, after the EXIT: the results (PARAM30), */
void p30_after_exit(void)
{
    tc_at(P30 + 010u);
    param30();
}

/* P30 +011: then XDELVFLG set ("SET XDELVFLG BIT 8 FLAG 2", for P40's
 * external delta V), */
void p30_after_param30(void)
{
    tc_to(P30 + 011u, UPFLAG); /* ADRES XDELVFLG */
}

/* P30 +013: and the program ends. */
void p30_after_xdelvflg(void)
{
    continue_at(GOTOPOOH); /* TCF */
}

/* ---- P31 -- p. 637 ---- */

/* P31: TIG. */
void p31(void)
{
    tc_at(P31);
    p30_p31();
}

/* P31 +1: "RESET UPDATFLG", */
void p31_after_p30_p31(void)
{
    tc_to(P31 + 1u, DOWNFLAG); /* ADRES UPDATFLG */
}

/* P31 +3: "RESET NORMSW", */
void p31_after_updatflg(void)
{
    tc_to(P31 + 3u, DOWNFLAG); /* ADRES NORMSW */
}

/* P31 +5: and to the interpreter, for P31_CODE. */
void p31_after_normsw(void)
{
    tc_at(P31 + 5u);
    intpret();
}

/* P31 +6: S31.1, and out of the interpreter. */
static const word p31_words[] = {
    ORDER(CALL),
    ADDRESS(S31_1),
    ORDER(EXIT),
};

/* P31 +011, after the EXIT: DELV(LV) shown (CNTNUP30), */
void p31_after_exit(void)
{
    tc_at(P31 + 011u);
    cntnup30();
}

/* P31 +012: the results (PARAM30), */
void p31_after_cntnup30(void)
{
    tc_at(P31 + 012u);
    param30();
}

/* P31 +013: XDELVFLG reset, */
void p31_after_param30(void)
{
    tc_to(P31 + 013u, DOWNFLAG); /* ADRES XDELVFLG */
}

/* P31 +015: and the program ends. */
void p31_after_xdelvflg(void)
{
    continue_at(GOTOPOOH); /* TCF */
}

/* ---- P30/P31, CNTNUP30 and PARAM30 -- pp. 637-638 ---- */

/* P30/P31: the return in P30/31RT (XCH Q; TS: on an overflow in Q the TS
 * would skip into UPFLAG's ADRES, which is not modelled), then "SET
 * UPDATFLG", */
static void p30_p31(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(P30_31RT))) not_modelled();
    tc_to(P30_P31 + 2u, UPFLAG); /* ADRES UPDATFLG */
}

/* P30/P31 +4: "SET TRACKFLG", */
void p30_p31_after_updatflg(void)
{
    tc_to(P30_P31 + 4u, UPFLAG); /* ADRES TRACKFLG */
}

/* P30/P31 +6: and "T OF IGN", verb 06 noun 33, flashed by GOFLASHR. */
void p30_p31_after_trackflg(void)
{
    set_A(V06N33);
    tc_to(P30_P31 + 7u, BANKCALL); /* CADR GOFLASHR */
}

/* P30/P31 +011: terminate. */
void p30_p31_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF */
}

/* P30/P31 +012: proceed, back through P30/31RT. */
void p30_p31_proceed(void)
{
    tc_at(P30_P31 + 012u);
    tc_return_in(ADDRESS_OF(P30_31RT));
}

/* P30/P31 +013: recycle (TCF P30/P31 +4), the flags and the display
 * again. */
void p30_p31_recycle(void)
{
    p30_p31_after_updatflg();
}

/* P30/P31 +014: the immediate return: phase 4 of group 6 (PHASCHNG, OCT
 * 00014), */
void p30_p31_immediate_return(void)
{
    tc_to(P30_P31 + 014u, PHASCHNG); /* OCT 00014 */
}

/* P30/P31 +016: and the job ends. */
void p30_p31_after_phaschng(void)
{
    tc_to(P30_P31 + 016u, ENDOFJOB);
}

/* CNTNUP30: the return in P30/RET (XCH Q; TS: on an overflow in Q the TS
 * skips the CAF, and A, +1 or -1, goes to GOFLASH), then DELV(LV), verb
 * 06 noun 81, flashed by GOFLASH. */
static void cntnup30(void)
{
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(P30_RET))) set_A(V06N81);
    tc_to(CNTNUP30 + 3u, BANKCALL); /* CADR GOFLASH */
}

/* CNTNUP30 +5: terminate. */
void cntnup30_terminate(void)
{
    continue_at(GOTOPOOH); /* TCF */
}

/* CNTNUP30 +6: proceed, back through P30/RET. */
void cntnup30_proceed(void)
{
    tc_at(CNTNUP30 + 6u);
    tc_return_in(ADDRESS_OF(P30_RET));
}

/* CNTNUP30 +7: recycle (TCF CNTNUP30 +2), the display again. */
void cntnup30_recycle(void)
{
    set_A(V06N81);
    tc_to(CNTNUP30 + 3u, BANKCALL); /* CADR GOFLASH */
}

/* PARAM30: the return in P30/31RT (as CNTNUP30's in P30/RET), then
 * apogee, perigee and delta V, verb 06 noun 42, flashed by GOFLASH. */
static void param30(void)
{
    xch(Q_ADDRESS);
    if (!ts(ADDRESS_OF(P30_31RT))) set_A(V06N42);
    tc_to(PARAM30 + 3u, BANKCALL); /* CADR GOFLASH */
}

/* PARAM30 +5: "ON TERMINATION GOTOPOOH". */
void param30_terminate(void)
{
    tc_to(PARAM30 + 5u, GOTOPOOH);
}

/* PARAM30 +6: "ON PROCEED GO DO REFTEST". */
void param30_proceed(void)
{
    reftest();
}

/* PARAM30 +7: recycle (TCF PARAM30 +2), the display again. */
void param30_recycle(void)
{
    set_A(V06N42);
    tc_to(PARAM30 + 3u, BANKCALL); /* CADR GOFLASH */
}

/* REFTEST: REFSMFLAG (BIT13 of STATE +3). Set: +MGA from DELVSIN
 * (REFTEST_CODE, GET+MGA); "=0 , THEN BRANCH TO NOTSET": "+MGA, +MGA+1
 * CONTAINS (-00001)" (DCS MARSDP, DXCH), and on at FLASHMGA. */
static void reftest(void)
{
    set_A(BIT13);
    mask_at(ADDRESS_OF(STATE) + 3u);
    if (!A_is_zero()) { /* EXTEND; BZF NOTSET */
        tc_at(REFTEST + 4u);
        intpret();
        return;
    }
    dcs(MARSDP); /* NOTSET: EXTEND; DCS */
    dxch(ADDRESS_OF(PMGA));
    tc_at(NOTSET + 3u);
    intpret();
}

/* REFTEST +5: the delta V's +MGA (GET+MGA, with DELVSIN pushed), then
 * FLASHMGA. */
static const word reftest_words[] = {
    ORDERS(VLOAD, PUSH), ADDRESS_PLUS_ONE(DELVSIN), ORDER(CALL), ADDRESS(GETPMGA),
    ORDER(GOTO),         ADDRESS(FLASHMGA),
};

/* FLASHMGA: the clock display (DISPMGA), then back out of the
 * interpreter. */
static const word flashmga_words[] = {
    ORDER(CALL),
    ADDRESS(DISPMGA),
    ORDER(EXIT),
};

/* FLASHMGA +3, after the EXIT: back through P30/31RT. */
void flashmga_after_exit(void)
{
    tc_at(FLASHMGA + 3u);
    tc_return_in(ADDRESS_OF(P30_31RT));
}

/* ---- AVFLAGA, AVFLAGP and P20FLGON -- p. 648 ---- */

/* AVFLAGA: "AVFLAG = CSM": the return in SUBEXIT, then AVFLAG reset, */
void avflaga(void)
{
    qxch(ADDRESS_OF(SUBEXIT));     /* EXTEND; QXCH */
    tc_to(AVFLAGA + 2u, DOWNFLAG); /* ADRES AVFLAG */
}

/* AVFLAGA +4: "SET ECSTEER = 1" (BIT13, in E bank 7), and back through
 * SUBEXIT (in E bank 4). A from a CAF has no overflow for a TS to skip
 * on. */
void avflaga_after_avflag(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    set_A(BIT13);
    ts(ADDRESS_OF(ECSTEER));
    set_A(EBANK4);
    ts(EBANK_ADDRESS);
    tc_at(AVFLAGA + 012u);
    tc_return_in(ADDRESS_OF(SUBEXIT));
}

/* AVFLAGP: "AVFLAG = LEM": the return in SUBEXIT, AVFLAG set, */
void avflagp(void)
{
    qxch(ADDRESS_OF(SUBEXIT));   /* EXTEND; QXCH */
    tc_to(AVFLAGP + 2u, UPFLAG); /* ADRES AVFLAG */
}

/* AVFLAGP +4: and back through SUBEXIT. */
void avflagp_after_avflag(void)
{
    tc_at(AVFLAGP + 4u);
    tc_return_in(ADDRESS_OF(SUBEXIT));
}

/* P20FLGON: the return in SUBEXIT, TRACKFLG set, */
void p20flgon(void)
{
    qxch(ADDRESS_OF(SUBEXIT));    /* EXTEND; QXCH */
    tc_to(P20FLGON + 2u, UPFLAG); /* ADRES TRACKFLG */
}

/* P20FLGON +4: UPDATFLG set, */
void p20flgon_after_trackflg(void)
{
    tc_to(P20FLGON + 4u, UPFLAG); /* ADRES UPDATFLG */
}

/* P20FLGON +6: and back through SUBEXIT. */
void p20flgon_after_updatflg(void)
{
    tc_at(P20FLGON + 6u);
    tc_return_in(ADDRESS_OF(SUBEXIT));
}

/* ---- The interpretive code ---- */

const struct interpretive_code DISPMGA_CODE = {035u, DISPMGA, COUNT(dispmga_words), dispmga_words};
const struct interpretive_code END45_CODE = {035u, END45 + 1u, COUNT(end45_words), end45_words};
const struct interpretive_code P30_CODE = {035u, P30 + 5u, COUNT(p30_words), p30_words};
const struct interpretive_code P31_CODE = {035u, P31 + 6u, COUNT(p31_words), p31_words};
const struct interpretive_code REFTEST_CODE = {035u, REFTEST + 5u, COUNT(reftest_words),
                                               reftest_words};
const struct interpretive_code FLASHMGA_CODE = {035u, FLASHMGA, COUNT(flashmga_words),
                                                flashmga_words};

#endif
