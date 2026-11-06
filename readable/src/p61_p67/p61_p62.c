/* SPDX-License-Identifier: MIT */
/* P61-P67.agc of the CM, part 1 (Comanche055 pp. 789-794, bank 26): P61
 * and P62.
 *
 * P61, "TO CALCULATE AND DISPLAY EMS INITIALIZATION DATA": the crew loads
 * the splash point and the lift direction (V06N61: LAT(SPL), LNG(SPL),
 * HEADSUP), S61.2 predicts the entry (GMAX, VPRED and GAMMAEI at 400K FT,
 * shown with V06N60; RTGO, VIO and TTE at EMSALT, with V06N63); then on
 * into P62. P62, "TO NOTIFY CREW WHEN GNC SYSTEM IS PREPARED FOR CM/SM
 * SEPARATION" and "TO ORIENT THE CM TO THE CORRECT ATTITUDE FOR
 * ATMOSPHERIC ENTRY": the entry DAP starts (CM/DAPIC, CM/DAPON), the crew
 * is asked to separate (GOPERF1R, code 41), the splash point shown again,
 * and the commands for the entry attitude set (ROLLC, ALFACOM, BETACOM);
 * then P63.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, NEWMODEX, UPFLAG, NOVAC,
 * POSTJUMP, ENDOFJOB, GOTOPOOH, TASKOVER) leaves Z at the routine and Q at
 * the word after the TC, where the routine comes back to; ENDOFJOB,
 * GOTOPOOH and TASKOVER don't come back. The faithful port keys a routine
 * at each such return, at each TC's return in the file, and at each TC
 * INTPRET's NEWOPS. */
#include "p61_p67.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p61_4(void);
static void p61_2(void);
static void p62_2_separation(void);
static void p62_2_plus3(void);

/* ---- P61 -- pp. 789-791 ---- */

/* P61: "LOCK OUT EXTENDED VERBS SO CAN USE TFF ROUTINES" (EXTVBACT gets
 * BIT14); HEADSUP preloaded to -1, lift up; then S61.1, "CHECK STATE
 * VECTOR AND IMU ORIENTATION". (A is a constant: neither TS skips.) */
void p61(void)
{
    set_A(BIT14);
    ts(ADDRESS_OF(EXTVBACT));
    set_A(negate(ONE));
    ts(ADDRESS_OF(HEADSUP));
    s61_1(P61 + 4u);
}

/* P61 +5, S61.1's return (TC 60GENRET): the splash point and the lift
 * direction flashed for the crew to load (V06N61, GOFLASHR). */
void p61_display(void)
{
    set_A(V06N61);
    tc_to(P61 + 6u, BANKCALL); /* CADR GOFLASHR */
}

void p61_terminate(void)
{
    tc_to(P61 + 010u, GOTOPOOH);
}

/* P61 +011: proceed (TC P61.4). */
void p61_proceed(void)
{
    tc_at(P61 + 011u);
    p61_4();
}

/* P61 +012: the crew loaded new data (TC -5): flash it again. */
void p61_enter(void)
{
    tc_at(P61 + 012u);
    p61_display();
}

/* P61.3: the display's immediate return: a restart point (PHASCHNG, OCT
 * 00014), then ENDOFJOB. P62 and P65.1 come here too. */
void p61_3(void)
{
    tc_to(P61_3, PHASCHNG); /* OCT 00014 */
}

void p61_3_after_phaschng(void)
{
    tc_to(P61_3 + 2u, ENDOFJOB);
}

/* P61.4: ROLLC for S62.3, "GIM ANG AT .05G": "IF HEADSUP POS,ROLLC =180
 * DEG.(LIFT DWN)" (BIT14, a half revolution), "IF HEADSUP NEG,ROLLC =0
 * (LIFT UP)" (CCS leaves +0 in A); L, the second word, +0 (ZL). With
 * HEADSUP +0, CCS also leaves +0 (the NOOP's way); with -0 it skips the
 * DXCH, and ROLLC is left as it was. Then the interpretive code at
 * NEWRNVN. */
static void p61_4(void)
{
    enum sign headsup;

    zl();
    headsup = ccs_at(ADDRESS_OF(HEADSUP));
    if (headsup == POSITIVE) set_A(BIT14); /* then the NOOP */
    if (headsup != IS_MINUS_ZERO) dxch(ADDRESS_OF(ROLLC));
    tc_at(P61_4 + 5u);
    intpret();
}

/* NEWRNVN: the state vector for S61.2: PIPTIME, the time of RN and VN,
 * kept in MM, "TO DETERMINE IF AN UPDATE HAS OCCURRED", and STARTEN1 run
 * ("INITIALIZE"); RN to RONE, its unit vector to URONE, VN to VONE, and
 * UNI, the unit normal to the orbit (VN cross URONE). DUMPP61: if PIPTIME
 * moved on meanwhile ("UPDATED... GO TRY AGAIN"), again; else S61.2,
 * "GET DISPLAY DATA FOR N60 AND N63", which returns in basic at P61.1. */
static const word newrnvn_words[] = {
    /* NEWRNVN */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(PIPTIME),
    STCALL(MM),
    ADDRESS(STARTEN1),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RN),
    STORE(RONE),
    ORDER(UNIT),
    STOVL(URONE),
    ADDRESS_PLUS_ONE(VN),
    STORE(VONE),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(URONE),
    STORE(UNI),
    /* DUMPP61 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(MM), /* "INITIAL VALUE OF PIPTIME" */
    ADDRESS_PLUS_ONE(PIPTIME),
    ORDERS(BMN, CALRB),
    ADDRESS(NEWRNVN),
    ADDRESS(S61_2),
};

const struct interpretive_code NEWRNVN_CODE = {026u, P61_4 + 6u, COUNT(newrnvn_words),
                                               newrnvn_words};

/* P61.1: S61.2's return: the marks cleared (CLEARMRK); GMAX, VPRED and
 * GAMMAEI flashed (V06N60, GOFLASH). */
void p61_1(void)
{
    tc_to(P61_1, CLEARMRK);
}

void p61_1_display(void)
{
    set_A(V06N60);
    tc_to(P61_1 + 2u, BANKCALL); /* CADR GOFLASH */
}

void p61_1_terminate(void)
{
    tc_to(P61_1 + 4u, GOTOPOOH);
}

/* P61.1 +5: proceed (TC P61.2). */
void p61_1_proceed(void)
{
    tc_at(P61_1 + 5u);
    p61_2();
}

/* P61.1 +6: recycle (TC -5): flash it again. */
void p61_1_enter(void)
{
    tc_at(P61_1 + 6u);
    p61_1_display();
}

/* P61.2: "CORRECT TTE FOR TIME LAPSE DURING ABOVE DISPLAY": TTE is the
 * time from now (LOADTIME, "CURRENT TIME.") less PIPTIME (in MM), plus
 * TTE1, the "NEGATIVE OF FREE FALL TIME." */
static void p61_2(void)
{
    tc_at(P61_2);
    intpret();
}

static const word p61_2_words[] = {
    /* P61.2 +1 */ ORDERS(RTB, DSU),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(MM),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(TTE1),
    STORE(TTE),
    ORDER(EXIT),
};

const struct interpretive_code P61_2_CODE = {026u, P61_2 + 1u, COUNT(p61_2_words), p61_2_words};

/* P61.2 +010: RTGO, VIO and TTE flashed (V06N63, GOFLASH). */
void p61_2_display(void)
{
    set_A(V06N63);
    tc_to(P61_2 + 011u, BANKCALL); /* CADR GOFLASH */
}

void p61_2_terminate(void)
{
    tc_to(P61_2 + 013u, GOTOPOOH);
}

/* P61.2 +014: proceed (TC +2), ".... THEN FALL INTO P62": the major mode
 * changed to 62 (NEWMODEX, MM 62). */
void p61_2_proceed(void)
{
    tc_at(P61_2 + 014u);
    tc_to(P62 - 4u, NEWMODEX); /* MM 62 */
}

/* P61.2 +015: "REDO" (TC P61.2). */
void p61_2_redo(void)
{
    tc_at(P61_2 + 015u);
    p61_2();
}

/* ---- P62 -- pp. 792-794 ---- */

/* P62 -2, NEWMODEX's return: the downlist code to 1 (DNLSTCOD), and P62.
 * (A is a constant: TS does not skip.) */
void p62_from_p61(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(DNLSTCOD));
    p62();
}

/* P62: S61.1, "CHECK STATE VECTOR AND IMU ORIENTATION." */
void p62(void)
{
    s61_1(P62);
}

void p62_after_s61_1(void)
{
    tc_at(P62 + 1u);
    intpret();
}

/* P62 +2: P62.3 as POSEXIT, where CM/POSE goes on ("CALCULATE DESIRED
 * .05G GIMBAL ANGLES"), and CM/DAPIC, "START CM/POSE AND BODY RATE CALC",
 * which "RETURNS IN BASIC TO P62.2." */
static const word p62_words[] = {
    /* P62 +2 */ ORDERS(SSP, RTB),
    ADDRESS_PLUS_ONE(POSEXIT),
    ADDRESS(P62_3),
    ADDRESS(CM_DAPIC),
};

const struct interpretive_code P62_CODE = {026u, P62 + 2u, COUNT(p62_words), p62_words};

/* P62.2: "CONTINUE WITH CM/POSE AFTER AV G." (POSECADR to AVEGEXIT); then
 * "REQUEST SEPARATION" (GOPERF1R, code 41). */
void p62_2(void)
{
    dca_2cadr(POSECADR);
    dxch(ADDRESS_OF(AVEGEXIT));
    p62_2_separation();
}

static void p62_2_separation(void)
{
    set_A(OCT41);
    tc_to(P62_2 + 4u, BANKCALL); /* CADR GOPERF1R */
}

void p62_2_terminate(void)
{
    tc_to(P62_2 + 6u, GOTOPOOH);
}

/* P62.2 +7: proceed (TC +3). */
void p62_2_proceed(void)
{
    tc_at(P62_2 + 7u);
    p62_2_plus3();
}

/* P62.2 +010: enter (TC -5): ask again. */
void p62_2_enter(void)
{
    tc_at(P62_2 + 010u);
    p62_2_separation();
}

/* P62.2 +011: the immediate return, "FOR PHASCHNG AND ENDOFJOB" (TC
 * P61.3). */
void p62_2_immediate_return(void)
{
    tc_at(P62_2 + 011u);
    p61_3();
}

/* P62.2 +012 (+3): CM/DAPON, to "DISABLE RCS DAP, ENABLE ENTRY DAP AND DO
 * ATTITUDE HOLD." (POSTJUMP); it goes on at P62.1. */
static void p62_2_plus3(void)
{
    tc_to(P62_2 + 012u, POSTJUMP); /* CADR CM/DAPON */
}

/* P62.1: the splash point and the lift direction flashed again (V06N61,
 * GOFLASH). */
void p62_1(void)
{
    set_A(V06N61);
    tc_to(P62_1 + 1u, BANKCALL); /* CADR GOFLASH */
}

/* P62.1 +3: terminate (TC -3): flash it again. */
void p62_1_terminate(void)
{
    tc_at(P62_1 + 3u);
    p62_1();
}

/* P62.1 +4: proceed (TC +2): a restart point (PHASCHNG, OCT 04024): "USE
 * ENTRYVN FOR DISPLAY BELOW." */
void p62_1_proceed(void)
{
    tc_at(P62_1 + 4u);
    tc_to(P62_1 + 6u, PHASCHNG); /* OCT 04024 */
}

/* P62.1 +5: enter (TC -5): flash it again. */
void p62_1_enter(void)
{
    tc_at(P62_1 + 5u);
    p62_1();
}

/* P62.1 +010: "TERMINATE ATTITUDE HOLD. SET UP COMMANDS: ROLLC, ALFACOM,
 * BETACOM. BEGIN MANEUVER TO ENTRY ATTITUDE." ROLLC: "IF HEADSUP POS,
 * ROLLC=180 DEG (LIFT DWN)" (BIT14), "IF HEADSUP NEG, ROLLC=0 DEG (LIFT
 * UP)" (+0 from CCS, as is HEADSUP +0's); with -0 the TS is skipped.
 * ALFACOM gets ALFAPAD, the "NOMINAL ALFATRIM PAD LOADED", and BETACOM
 * +0; P63FLAG +1, which "STARTS UP P63." (EXDAP2 changes it); ENTRYVN
 * V06N22, the "DISPLAY FOR CDU DESIRED VALUES"; then "TURN ON ENTRY
 * DISPLAY" (UPFLAG ENTRYDSP). (No TS here finds an overflow in A: none
 * skips.) */
void p62_1_after_phaschng(void)
{
    enum sign headsup = ccs_at(ADDRESS_OF(HEADSUP));

    if (headsup == POSITIVE) set_A(BIT14); /* then the NOOP */
    if (headsup != IS_MINUS_ZERO) ts(ADDRESS_OF(ROLLC));
    ca(ADDRESS_OF(ALFAPAD));
    zl();
    dxch(ADDRESS_OF(ALFACOM));
    set_A(ONE);
    ts(ADDRESS_OF(P63FLAG));
    set_A(V06N22);
    ts(ADDRESS_OF(ENTRYVN));
    tc_to(P62_1 + 023u, UPFLAG); /* ADRES ENTRYDSP */
}

/* P62.1 +025: "GO DIRECTLY TO P63 IF BODY ATTITUDE IS SUCH THAT THE DELAY
 * TASK: WAKEP62 WILL BE OMITTED.": bit 1 of CMDAPMOD's complement clear
 * (BZF) goes to P63.1, which ends the job; "(I.E., CONTINUE IF CMDAPMOD =
 * -1, OR +0)" to P63. */
void p62_1_after_upflag(void)
{
    cs(ADDRESS_OF(CMDAPMOD));
    mask(ONE);
    if (A_is_zero()) {
        p63_1(); /* BZF P63.1 */
        return;
    }
    tc_at(P62_1 + 031u); /* TC P63 */
    p63();
}

/* WAKEP62, "CALLED FROM ENTRY DAP": the task that starts P63 as a job
 * (NOVAC, priority 13). */
void wakep62(void)
{
    set_A(PRIO13);
    tc_to(WAKEP62 + 1u, NOVAC); /* 2CADR P63 */
}

void wakep62_after_novac(void)
{
    tc_to(WAKEP62 + 4u, TASKOVER);
}

/* P62.3: each 2 seconds, the gimbal angles for the entry conditions "THAT
 * WILL HOLD IF REORIENTATION WERE MADE AT PRESENT RN, VN.": ENDEXIT as
 * the return (QPRET), so that S62.3 "GOES DIRECTLY TO ENTRY GUIDANCE
 * EXIT". */
static const word p62_3_words[] = {
    /* P62.3 */ ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(QPRET),
    ADDRESS(ENDEXIT),
    ADDRESS(S62_3),
};

const struct interpretive_code P62_3_CODE = {026u, WAKEP62 + 5u, COUNT(p62_3_words), p62_3_words};

#endif
