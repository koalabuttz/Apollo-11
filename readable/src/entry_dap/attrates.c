/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part B (Comanche055 pp. 1071-1074, bank
 * 15): the rates taken relative to the turning flight path, NOGAMDUT, and
 * ATTRATES: "CALCULATE BODY ATTITUDE RATES AND INTEGRATE TO OBTAIN
 * ATTITUDE ANGLES."
 *   "CB PHIDOT TCDU/180 = (CA PREL + SA RREL) TCDU/180"
 *   "BETADOT TCDU/180 = (-SA PREL + CA RREL) TCDU/180"
 *   "ALFADOT TCDU = (QREL + SB PHIDOT) TCDU/180"
 * The attitude angles ALFA/180, BETA/180 and ROLL/180 are kept with their
 * overflows corrected (ANGOVCOR); ALFA's error from ALFACOM drives the
 * pitch needle (AK1, QAXERR), and BETA's from BETACOM is the yaw error
 * (RAXERR). "START YAW AUTOPILOT HERE.  RATE DAMPING WITH ENFORCED
 * COORDINATED ROLL MANEUVER.": outside the atmosphere (.05GSW clear) on
 * at EXDAP; inside, on at the yaw rate damping. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- The flight path's turn -- pp. 1071-1072 ---- */

/* BODYRATE +0100: "PREL TCDU/180=(P-SALF SR GAMDOT)TCDU/180" (ADS PREL);
 * then the cosine of ROLL/180, */
void bodyrate_trim_prel(void)
{
    ads(ADDRESS_OF(PREL));
    ca(ADDRESS_OF(ROLL_180));
    tc_to(BODYRATE + 0102u, SPCOS);
}

/* BODYRATE +0103: "QREL TCDU/180=(Q-CR GAMDOT) TCDU/180"; and "B( ) = -SR
 * GAMDOT" times COSTRIM, "COS(-20)", "RREL TCDU/180=(R+CALF SR
 * GAMDOT)TCDU/180"; on at NOGAMDUT. */
void bodyrate_trim_qrel(void)
{
    com();
    mp_at(ADDRESS_OF(GAMDOT)); /* EXTEND; MP */
    ads(ADDRESS_OF(QREL));
    cs(ADDRESS_OF(JETEM) + 1u);
    mp(COSTRIM); /* EXTEND; MP */
    ads(ADDRESS_OF(RREL));
    nogamdut();
}

/* ---- NOGAMDUT -- p. 1072 ---- */

/* NOGAMDUT: CMDAPARM ("CMDAPARM = 93D BIT 12") clear, "DAP NOT ARMED."
 * (STBYDUMP: EXTEND; BZF TASKOVER). Armed: "PICK UP AT ATTRATES IN 10 MS
 * OR SO." (POSMAX to TIME5, ATDOTCAD's 2CADR ATTRATES to T5LOC), and the
 * task ends. */
void nogamdut(void)
{
    set_A(BIT12);
    mask_at(ADDRESS_OF(CM_FLAGS));
    if (A_is_zero()) {
        continue_at(TASKOVER);
        return;
    }
    set_A(POSMAX);
    ts(ADDRESS_OF(TIME5));
    dca(ATDOTCAD); /* EXTEND; DCA */
    dxch(ADDRESS_OF(T5LOC));
    tc_to(NOGAMDUT + 011u, TASKOVER);
}

/* ---- ATTRATES -- pp. 1073-1074 ---- */

/* ATTRATES: "CONTINUE HERE VIA T5", "TASK MAY BE SKIPPED AT RESTART.": the
 * interrupted program's BBANK and Q kept (LXCH BANKRUPT, QXCH QRUPT) and
 * SR, doubled, in CM/SAVE. Then ALFA/180 with QREL added, its overflow
 * corrected (ANGOVCOR). (An overflow at TS CM/SAVE skips the CA QREL.) */
void attrates(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT)); /* EXTEND; QXCH */
    ca(ADDRESS_OF(SR));
    double_A();
    if (!ts(ADDRESS_OF(CM_SAVE))) ca(ADDRESS_OF(QREL));
    ad_at(ADDRESS_OF(ALFA_180));
    tc_to(ATTRATES + 010u, ANGOVCOR);
}

/* ATTRATES +011: into ALFA/180, and its cosine. (An overflow skips the TC
 * SPCOS: on at ATTRATES +013 with +1 or -1.) */
void attrates_alfa(void)
{
    if (ts(ADDRESS_OF(ALFA_180))) {
        attrates_calfa();
        return;
    }
    tc_to(ATTRATES + 012u, SPCOS);
}

/* ATTRATES +013: CALFA, and PHIDOT. "CA PREL" into PHIDOT, "CA RREL" into
 * BETADOT; then the sine of ALFA/180. (An overflow at TS CALFA skips the
 * TS PHIDOT.) */
void attrates_calfa(void)
{
    if (!ts(ADDRESS_OF(CALFA))) ts(ADDRESS_OF(PHIDOT));
    mp_at(ADDRESS_OF(PREL)); /* EXTEND; MP */
    xch(ADDRESS_OF(PHIDOT));
    mp_at(ADDRESS_OF(RREL)); /* EXTEND; MP */
    ts(ADDRESS_OF(BETADOT));
    ca(ADDRESS_OF(ALFA_180));
    tc_to(ATTRATES + 024u, SPSIN);
}

/* ATTRATES +025: "SIN(ALFA)" (SALFA); "SA RREL" added into PHIDOT, "CB
 * PHIDOT, SAVED."; -SALFA times PREL added into BETADOT, "SAVE BETADOT
 * TCDU/180", and into BETA/180, "BETA DONE."; then the sine of BETA/180.
 * (An overflow at TS SALFA skips the EXTEND: MP RREL runs as MASK RREL.) */
void attrates_salfa(void)
{
    if (ts(ADDRESS_OF(SALFA)))
        mask_at(ADDRESS_OF(RREL));
    else
        mp_at(ADDRESS_OF(RREL));
    ads(ADDRESS_OF(PHIDOT));
    cs(ADDRESS_OF(SALFA));
    mp_at(ADDRESS_OF(PREL)); /* EXTEND; MP */
    ads(ADDRESS_OF(BETADOT));
    ads(ADDRESS_OF(BETA_180));
    tc_to(ATTRATES + 036u, SPSIN);
}

/* ATTRATES +037: times PHIDOT, "NEGLECT CB IN CB PHIDOT", added to
 * ALFA/180, its overflow corrected. */
void attrates_sb_phidot(void)
{
    mp_at(ADDRESS_OF(PHIDOT)); /* EXTEND; MP */
    ad_at(ADDRESS_OF(ALFA_180));
    tc_to(ATTRATES + 042u, ANGOVCOR);
}

/* ATTRATES +043: "ALFA DONE." (ALFA/180); its error from ALFACOM,
 * corrected, "JUST IN CASE ...". (An overflow at the TS skips the COM.) */
void attrates_alfa_done(void)
{
    if (!ts(ADDRESS_OF(ALFA_180))) com();
    ad_at(ADDRESS_OF(ALFACOM));
    tc_to(ATTRATES + 046u, ANGOVCOR);
}

/* ATTRATES +047: AK1, and QAXERR, "FOR PITCH FDAI AND EDIT." Then
 * "PHIDOT TCDU/180, NEGLECTING CB" added to ROLL/180, corrected. (An
 * overflow at TS AK1 skips the TS QAXERR.) */
void attrates_ak1(void)
{
    if (!ts(ADDRESS_OF(AK1))) ts(ADDRESS_OF(QAXERR));
    ca(ADDRESS_OF(PHIDOT));
    ad_at(ADDRESS_OF(ROLL_180));
    tc_to(ATTRATES + 053u, ANGOVCOR);
}

/* ATTRATES +054: "ROLL/180 FOR TM." (ROLLTM), "ROLL DONE." (ROLL/180).
 * The yaw error, BETACOM less BETA/180, in RAXERR: "IF IN ATM, SAVE
 * 'RAXERR' FOR TM DNLST." Then .05GSW ("SW=0, LESS .05G"): clear, on at
 * EXDAP, "IF G LESS THAN .05"; set, -1 (CS ONE) and on at ATTRATES +066,
 * "IF G GEQ  THAN .05". (An overflow at TS ROLLTM skips the TS ROLL/180;
 * one at TS RAXERR, the CA BIT3: the MASK takes +1 or -1.) */
void attrates_rolltm(void)
{
    if (!ts(ADDRESS_OF(ROLLTM))) ts(ADDRESS_OF(ROLL_180));
    cs(ADDRESS_OF(BETA_180));
    ad_at(ADDRESS_OF(BETACOM));
    if (!ts(ADDRESS_OF(RAXERR))) set_A(BIT3);
    mask_at(ADDRESS_OF(CM_FLAGS));
    if (A_is_zero()) { /* EXTEND; BZF EXDAP */
        continue_at(EXDAP);
        return;
    }
    set_A(negate(ONE)); /* CS ONE */
    continue_at(ATTRATES + 066u);
}

#endif
