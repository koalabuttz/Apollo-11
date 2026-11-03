/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 14 (Comanche055 pp. 616-618): V89CALL, verb 89, which
 * points the CSM at the LM (bank 34).
 *
 * Keyed in during P00, V89 checks the IMU (R02BOTH: it comes back only if
 * the IMU is on and its orientation known) and flashes verb 04 noun 06 for
 * the crew to choose the axis to point: R2 1 for the preferred axis (the
 * track axis 55 degrees from +X), anything else for the +X axis. The
 * choice sets or resets PRFTRKAT (RNGSCFLG, "BIT 10 FLAG 5") for R63.
 *
 * V89RECL: R63 (R63COMP) brings both state vectors to a minute from now
 * (P21TIME) and works out the gimbal angles for the axis chosen; verb 06
 * noun 18 flashes them. Recycle works them out again; proceed resets
 * 3AXISFLG and maneuvers the CSM (R60CSM); terminate ends the job
 * (ENDEXT).
 *
 * Each call out of the file (BANKCALL, UPFLAG, DOWNFLAG, ENDEXT) leaves Z
 * at the routine and Q at the word after the TC, where the routine comes
 * back to; ENDEXT doesn't come back. GOFLASH comes back to the word after
 * its CADR on terminate, the next on proceed, and the next on enter. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void v89_choose_axis(void);
static void setpaf(void);

/* V89CALL -- p. 617: "IMU STATUS CHECK. RETURNS IF ORIENTATION" "KNOWN.
 * ALARMS IF NOT." */
void v89call(void)
{
    tc_to(V89CALL, BANKCALL); /* CADR R02BOTH */
}

/* V89CALL +2: "ALLOW ASTRONAUT TO SELECT DESIRED" "TRACKING ATTITUDE
 * AXIS.": OPTION1 3, OPTION1 +1 1 (the preferred axis). (A CAF leaves no
 * overflow for a TS to skip on.) */
void v89call_after_r02both(void)
{
    set_A(THREE);
    if (ts(ADDRESS_OF(OPTION1))) not_modelled();
    set_A(ONE);
    if (ts(ADDRESS_OF(OPTION1) + 1u)) not_modelled();
    v89_choose_axis();
}

/* V89CALL +6: "V 04 N 06" flashed by GOFLASH. */
static void v89_choose_axis(void)
{
    set_A(VB04N06);
    tc_to(V89CALL + 7u, BANKCALL); /* CADR GOFLASH */
}

/* V89CALL +011: "TERMINATE". */
void v89_choose_terminate(void)
{
    tc_to(V89CALL + 011u, ENDEXT);
}

/* V89CALL +012: "PROCEED": TC +2, the choice. */
void v89_choose_proceed(void)
{
    tc_at(V89CALL + 012u);
    v89_choice();
}

/* V89CALL +013: "DATA IN.  OPTION1 +1 = 1 FOR PREF AXIS" ("= 2 FOR X
 * AXIS"): TC -5, the display again. */
void v89_choose_data_in(void)
{
    tc_at(V89CALL + 013u);
    v89_choose_axis();
}

/* V89CALL +014: "1 FOR PREF AXIS.  2 FOR X AXIS.": 1 less OPTION1 +1,
 * zero for the preferred axis, SETPAF; else RSTPAF: "RESET PREF ATT FLAG
 * FOR R63COMP" ("TO DO X AXIS.  RESET BIT 10 FLAG 5"). */
void v89_choice(void)
{
    cs(ADDRESS_OF(OPTION1) + 1u);
    ad(ONE);
    if (A_is_zero()) { /* EXTEND; BZF SETPAF */
        setpaf();
        return;
    }
    tc_to(V89CALL + 020u, DOWNFLAG); /* RSTPAF: ADRES RNGSCFLG */
}

/* V89RECL: TC INTPRET, */
void v89recl(void)
{
    tc_at(V89RECL);
    intpret();
}

/* V89RECL +1: "READ PRESENT TIME" (LOADTIME), "INTEGRATE TO 1 MIN FROM
 * NOW" (DP1MIN), "STORE TIME FOR CALL TO R63COMP.": R63COMP "LEAVES
 * DESIRED GIM ANGS IN THETAD, LOS IN" "POINTVSM, AND SELECTED AXIS IN
 * SCAXIS." */
static const word v89recl_words[] = {
    ORDERS(RTB, DAD),
    /* "READ PRESENT TIME" */
    ADDRESS(LOADTIME),
    /* "INTEGRATE TO 1 MIN FROM NOW" */
    ADDRESS_PLUS_ONE(DP1MIN),
    /* "STORE TIME FOR CALL TO R63COMP.  R63COMP" */
    STCALL(P21TIME),
    /* "LEAVES DESIRED GIM ANGS IN THETAD, LOS IN" */
    ADDRESS(R63),
    /* "POINTVSM, AND SELECTED AXIS IN SCAXIS." */
    ORDER(EXIT),
};

/* V89RECL +7 -- after the EXIT: "V 06 N 18" flashed by GOFLASH ("NOUN 18
 * REFERS TO THE DESIRED GIMBAL"). */
void v89recl_after_exit(void)
{
    set_A(VB06N18);
    tc_to(V89RECL + 010u, BANKCALL); /* CADR GOFLASH */
}

/* V89RECL +012: "TERMINATE". */
void v89recl_terminate(void)
{
    tc_to(V89RECL + 012u, ENDEXT);
}

/* V89RECL +013: "PROCEED": TC +2, the maneuver. */
void v89recl_proceed(void)
{
    tc_at(V89RECL + 013u);
    v89_maneuver();
}

/* V89RECL +014: "RECYCLE": TC V89RECL. */
void v89recl_recycle(void)
{
    tc_at(V89RECL + 014u);
    v89recl();
}

/* V89RECL +015: "RESET 3 AXIS FLAG" ("RESET BIT 6 FLAG 5"). */
void v89_maneuver(void)
{
    tc_to(V89RECL + 015u, DOWNFLAG); /* ADRES 3AXISFLG */
}

/* V89RECL +017: R60CSM, which "PERFORMS CSM MANEUVER TO ALIGN SELECTED"
 * "SPACECRAFT AXIS TO LOS." */
void v89_after_downflag(void)
{
    tc_to(V89RECL + 017u, BANKCALL); /* CADR R60CSM */
}

/* V89RECL +021: and the job ends. */
void v89_after_r60csm(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* SETPAF: "SET PREFERRED ATT FLAG FOR R63COMP" ("TO DO PREF AXIS.  SET
 * BIT 10 FLAG 5."). */
static void setpaf(void)
{
    tc_to(SETPAF, UPFLAG); /* ADRES RNGSCFLG */
}

/* SETPAF +2: TC V89RECL. */
void setpaf_after_upflag(void)
{
    tc_at(SETPAF + 2u);
    v89recl();
}

/* ---- The interpretive code ---- */

const struct interpretive_code V89RECL_CODE = {034u, V89RECL + 1u, COUNT(v89recl_words),
                                               v89recl_words};

#endif
