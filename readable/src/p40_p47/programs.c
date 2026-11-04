/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 1 (Luminary099 pp. 752-759, bank 36): the burn
 * programs. Their basic code, and their interpretive code in the
 * listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs (src/binding/p40_routines.c). The faithful C is
 * c64-port/src/lm_p40_p47.c.
 *
 * P40 ("TO COMPUTE A PREFERRED IMU ORIENTATION AND A PREFERRED VEHICLE
 * ATTITUDE FOR A LM DPS THRUSTING MANEUVER") and P42, its twin for the
 * ascent engine, each set WHICH to their table of BURNBABY (BURN_BABY_BURN
 * --_MASTER_IGNITION_ROUTINE.agc), check the stage (alarm 1706, P40ALM),
 * run R02 (R02BOTH, the IMU status check), set the delta V monitor's
 * threshold (DVTHRUSH) and count, and load the engine's thrust and exhaust
 * velocity; S40.1 then computes the thrust direction and the velocity to
 * be gained (VGTIG), and S40.2,3 the preferred IMU orientation. P40SXT4
 * maneuvers the LM to the burn attitude (R60LEM), and BURNBABY counts
 * down to the burn. After it, POSTBURN shows the burn's final values
 * (verb 16 noun 40) and TIGNOW the velocity still to be gained in body
 * axes (verb 16 noun 85). TERM40 ends the program.
 *
 * P41, the RCS burn, does the same with the jets' thrust (two jets or
 * four, NJETSFLG), and keeps verb 16 noun 85 up to date until the burn
 * (DYNMDISP, a job of its own; CALCN85 from SERVICER after it). P47, the
 * thrust monitor, starts the average-G routine (MIDTOAV2) and shows the
 * velocity the accelerometers count (verb 16 noun 83, CALCN83) until the
 * crew ends it.
 *
 * Each call out of the file (PHASCHNG, BANKCALL, IBNKCALL, FINDVAC,
 * DOWNFLAG, ...) leaves Z at the routine and Q at the word after the TC;
 * a TCF out of it (P40ALM, BURNBABY, ENDOFJOB, ...) leaves Z there. */
#include "p40_p47.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC to an erasable word that holds a return (P40/RET): the word runs as
 * TC <address>. Not modelled: a word that is not such an address (bits
 * 13-15 set, or below 010). */
static void tc_return_in(unsigned address)
{
    word to = fetch(address);

    if (OPERATION_CODE(to) != 0u || to < 010u) not_modelled();
    tc_through(address);
}

/* The words of the programs (P40LM +n and so on). */
#define P40_TC_INTPRET  (P40LM + 022u)
#define P40_EXIT        (P40LM + 036u)
#define P40_TC_P40SXT4  (P40LM + 042u)
#define P41_TC_INTPRET  (P41LM + 4u)
#define P41_EXIT        (P41LM + 022u)
#define P41_TC_P40SXT4  (P41LM + 030u)
#define P41_VGBODY_EXIT (P41LM + 036u)
#define P42_TC_INTPRET  (P42STAGE + 6u)
#define CALCN83_EXIT    (CALCN83 + 010u)
#define CALCN83_EXIT2   (CALCN83 + 017u)

/* ---- P40 -- p. 753 ---- */

/* P40LM: restart group 4 (PHASCHNG, OCT 04024). */
void p40lm(void)
{
    tc_to(P40LM, PHASCHNG);
}

/* P40LM +2: "INITIALIZATION FOR BURNBABY" (WHICH, P40TABLE). With the
 * descent stage gone (APSFLAG of FLGWRD10 set), alarm 1706 (P40ALM);
 * otherwise "GO DO IMU STATUS CHECK ROUTINE" (R02BOTH). MASK leaves A
 * positive or +0: those are the two ways CCS goes on here. */
void p40lm_after_phaschng(void)
{
    set_A(P40ADRES);
    ts(ADDRESS_OF(WHICH));
    ca(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(P40ALM); /* TCF P40ALM */
        return;
    }
    tc_to(P40LM + 010u, BANKCALL); /* CADR R02BOTH */
}

/* P40LM +012: "INITIALIZE DVMON". The threshold is THRESH3, and THRESH1
 * more undocked (CSMDOCKD of DAPBOOLS clear; MASK again leaves A positive
 * or +0); the count four. Then "LOAD CONSTANTS FOR DPS BURN" (P40_CODE). */
void p40lm_after_r02both(void)
{
    cs(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(THRESH1);
    ad(THRESH3);
    ts(ADDRESS_OF(DVTHRUSH));
    set_A(FOUR);
    ts(ADDRESS_OF(DVCNTR));
    tc_at(P40_TC_INTPRET);
    intpret();
}

/* P40_CODE: "LOAD F, MDOT, TDECAY" (FDPS, a vector of three DP words),
 * NOTHROTL clear; the exhaust velocity, DPSVEX, made positive and halved
 * (P40IN, where P42 comes in with APSVEX) "FOR TGO COMP." (VEX). S40.1
 * "COMPUTES UT AND VGTIG", S40.2,3 "COMPUTES PREFERRED IMU ORIENTATION". */
static const word p40_words[] = {
    ORDERS(VLOAD, CLEAR),
    ADDRESS_PLUS_ONE(FDPS),
    SWITCH(CLEAR, NOTHROTL),
    STORE(F),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(DPSVEX),
    /* P40IN */
    ORDERS(DCOMP, SR1),
    STCALL(VEX),
    ADDRESS(S40_1),
    ORDER(CALL),
    ADDRESS(S40_2_3),
    ORDER(EXIT),
};

/* After the EXIT: "ZERO ATTITUDE ERRORS, SET DB TO ONE DEG." (PFLITEDB). */
void p40lm_after_exit(void)
{
    inhint();
    tc_to(P40_EXIT + 2u, IBNKCALL); /* CADR PFLITEDB */
}

/* IBNKCALL's return: the maneuver to the burn attitude (P40SXT4). */
static void p40sxt4(void);

void p40lm_after_pflitedb(void)
{
    tc_at(P40_TC_P40SXT4);
    p40sxt4();
}

/* P40SXT4's return: on to the countdown (TCF BURNBABY). */
void p40lm_burn(void)
{
    continue_at(BURNBABY);
}

/* P40SXT4: the caller's return kept in P40/RET (QXCH). At P41MANU, the
 * interrupts on again, and "CLEAR 3AXISFLG -- R60 USE VECPOINT."
 * (DOWNFLAG). */
static void p40sxt4(void)
{
    qxch(ADDRESS_OF(P40_RET));     /* EXTEND; QXCH */
    relint();                      /* P41MANU */
    tc_to(P40SXT4 + 3u, DOWNFLAG); /* ADRES 3AXISFLG */
}

/* P40SXT4 +5: "DO ATTITUDE MANEUVER ROUTINE" (R60LEM). */
void p40sxt4_after_downflag(void)
{
    tc_to(P40SXT4 + 5u, BANKCALL); /* CADR R60LEM */
}

/* P40SXT4 +7: back to the caller (TC P40/RET). */
void p40sxt4_after_r60lem(void)
{
    tc_at(P40SXT4 + 7u);
    tc_return_in(ADDRESS_OF(P40_RET));
}

/* ---- After the burn -- pp. 754-755 ---- */

/* POSTBURN: DISPDEX the address of the next word (CA Z: positive, the
 * display up); SERVICER's exit to CALCN85 (AVEGEXIT gets the 2CADR at
 * ACADN85); and the final values, verb 16 noun 40 (TGO, VG, DELTAVM),
 * flashed by GOFLASHR. */
void postburn(void)
{
    set_A(POSTBURN + 1u); /* CA Z */
    ts(ADDRESS_OF(DISPDEX));
    dca(ACADN85); /* EXTEND; DCA */
    dxch(ADDRESS_OF(AVEGEXIT));
    set_A(V16N40);
    tc_to(POSTBURN + 6u, BANKCALL); /* CADR GOFLASHR */
}

static void term40(void);

/* GOFLASHR's returns: terminate (TC TERM40), */
void postburn_terminate(void)
{
    tc_at(POSTBURN + 010u);
    term40();
}

/* proceed (TCF TIGNOW), */
void postburn_proceed(void)
{
    tignow();
}

/* enter (TC POSTBURN: the display again), */
void postburn_enter(void)
{
    tc_at(POSTBURN + 012u);
    postburn();
}

/* and the immediate return, P40PHS1: restart group 4's phase 1 (PHASCHNG,
 * OCT 00014), and the job ends. */
void p40phs1(void)
{
    tc_to(P40PHS1, PHASCHNG);
}

void p40phs1_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* TIGNOW: the attitude errors zeroed (ZATTEROR) and the minimum deadband
 * set (SETMINDB), with interrupts inhibited, */
void tignow(void)
{
    inhint();
    tc_to(TIGNOW + 1u, IBNKCALL); /* CADR ZATTEROR */
}

void tignow_after_zatteror(void)
{
    tc_to(TIGNOW + 3u, IBNKCALL); /* CADR SETMINDB */
}

/* then verb 16 noun 85, the velocity to be gained in body axes, flashed
 * by REFLASHR. */
static void tignow_display(void)
{
    set_A(V16N85B);
    tc_to(TIGNOW + 7u, BANKCALL); /* CADR REFLASHR */
}

void tignow_after_setmindb(void)
{
    relint();
    tignow_display();
}

/* REFLASHR's returns: terminate (TC TERM40), */
void tignow_terminate(void)
{
    tc_at(TIGNOW + 011u);
    term40();
}

/* proceed (TCF TERM40), */
void tignow_proceed(void)
{
    term40();
}

/* enter (TC -5: the display again), */
void tignow_enter(void)
{
    tc_at(TIGNOW + 013u);
    tignow_display();
}

/* and the immediate return (TCF P40PHS1). */
void tignow_immediate(void)
{
    p40phs1();
}

/* TERM40: SERVICER's exit back to its own (AVEGEXIT gets SERVCADR's
 * 2CADR); "ZERO RENDZVS CNTERS" (TRKMKCNT); DISPDEX the address of the
 * next word (CA Z); and the deadband the DAP had (RESTORDB). */
static void term40(void)
{
    dca(SERVCADR); /* EXTEND; DCA */
    dxch(ADDRESS_OF(AVEGEXIT));
    set_A(ZERO);
    ts(ADDRESS_OF(TRKMKCNT));
    set_A(TERM40 + 6u); /* CA Z */
    ts(ADDRESS_OF(DISPDEX));
    inhint();
    tc_to(TERM40 + 010u, IBNKCALL); /* CADR RESTORDB */
}

/* TERM40 +012: the program ends (GOTOPOOH). */
void term40_after_restordb(void)
{
    relint();
    tc_to(TERM40 + 013u, GOTOPOOH);
}

/* ---- P41 -- pp. 755-757 ---- */

/* P41LM: "INITIALIZATION FOR BURNBABY" (WHICH, P41TABLE -5), and R02. */
void p41lm(void)
{
    set_A(P41ADRES);
    ts(ADDRESS_OF(WHICH));
    tc_to(P41LM + 2u, BANKCALL); /* CADR R02BOTH */
}

/* P41LM +4: to the interpreter, for P41_CODE. */
void p41lm_after_r02both(void)
{
    tc_at(P41_TC_INTPRET);
    intpret();
}

/* P41_CODE: the thrust of two jets (FRCS2, at P41FJET1) with NJETSFLG
 * set, of four (FRCS4) with it clear, into F (P41FJET). Then S40.1 and
 * S40.2,3 (P41IN, P41NORM), as P40's. */
static const word p41_words[] = {
    ORDERS(BON, DLOAD),
    SWITCH(BON, NJETSFLG),
    ADDRESS(P41FJET1),
    ADDRESS_PLUS_ONE(FRCS4),
    /* P41FJET */
    STCALL(F),
    ADDRESS(P41IN),
    /* P41FJET1 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(FRCS2),
    STORE(F),
    /* P41IN */
    ORDER(CALL),
    ADDRESS(S40_1),
    /* P41NORM */
    ORDER(CALL),
    ADDRESS(S40_2_3),
    ORDER(EXIT),
};

/* After the EXIT: "ZERO ATTITUDE ERRORS" (ZATTEROR), */
void p41lm_after_exit(void)
{
    inhint();
    tc_to(P41_EXIT + 2u, IBNKCALL); /* CADR ZATTEROR */
}

/* "SET 0.3 DEGREE DEADBAND" (SETMINDB), */
void p41lm_after_zatteror(void)
{
    tc_to(P41_EXIT + 4u, IBNKCALL); /* CADR SETMINDB */
}

/* and the maneuver (P40SXT4), */
void p41lm_after_setmindb(void)
{
    tc_at(P41_TC_P40SXT4);
    p40sxt4();
}

/* which comes back to the TC INTPRET for P41_VGBODY_CODE. */
void p41lm_after_p40sxt4(void)
{
    tc_at(P41_TC_P40SXT4 + 1u);
    intpret();
}

/* P41_VGBODY_CODE: "TRANSFORM VELOCITY-TO-BE-GAINED AT TIG", "FROM
 * REFERENCE COORDINATES TO LM BODY-", "AXIS COORDINATES FOR V16N85
 * DISPLAY." (S41.1). */
static const word p41_vgbody_words[] = {
    ORDERS(VLOAD, CALL), ADDRESS_PLUS_ONE(VGTIG), ADDRESS(S41_1), STORE(VGBODY), ORDER(EXIT),
};

/* After the EXIT: verb 16 noun 85 (GODSPRET, which returns at once), */
void p41lm_after_vgbody(void)
{
    set_A(V16N85B);
    tc_to(P41_VGBODY_EXIT + 2u, BANKCALL); /* CADR GODSPRET */
}

/* then DYNMDISP as a job of priority 5 to keep it up to date, DISPDEX
 * "FOR SAFETY ONLY", */
void p41lm_after_godspret(void)
{
    set_A(PRIO5);
    ts(ADDRESS_OF(DISPDEX));
    tc_to(P41_VGBODY_EXIT + 6u, FINDVAC); /* 2CADR DYNMDISP */
}

/* "GROUP 6 RESTARTS AT REDO6.7", "GROUP 4 RESTARTS HERE" (2PHSCHNG, OCT
 * 00076, OCT 04024), */
void p41lm_after_findvac(void)
{
    tc_to(P41_VGBODY_EXIT + 011u, TWO_PHSCHNG);
}

/* and on to the countdown (TCF B*RNB*B*). */
void p41lm_burn(void)
{
    continue_at(B_STAR_RNB_STAR_B_STAR);
}

/* REDO6.7: "ON A RESTART, DO NOT PUT UP DISPLAY IF BLANKING (BETWEEN
 * TIG-35 AND TIG-30)", DISPDEX -2 then: BLNKWAIT waits a second
 * (DELAYJOB), and comes back here. Otherwise verb 16 noun 85 again. */
void redo6_7(void)
{
    ca(ADDRESS_OF(DISPDEX));
    ad(TWO);
    if (A_is_zero()) { /* EXTEND; BZF BLNKWAIT */
        set_A(ONE_SEC);
        tc_to(BLNKWAIT + 1u, BANKCALL); /* CADR DELAYJOB */
        return;
    }
    set_A(V16N85B);
    tc_to(REDO6_7 + 5u, BANKCALL); /* CADR GODSPRET */
}

/* REDO6.7 +7: DYNMDISP's priority (PRIOCHNG), and on at DYNMDISP. */
void redo6_7_after_godspret(void)
{
    set_A(PRIO5);
    tc_to(REDO6_7 + 010u, PRIOCHNG);
}

/* DYNMDISP: "A NON-POSITIVE DISPDEX INDICATES PAST TIG-35, SO SERVICER
 * WILL BE DOING THE UPDATING OF NOUN 85.  STOP DYNMDISP." (ENDOFJOB).
 * Otherwise noun 85 from VGPREV (DYNMDISP_CODE). */
void dynmdisp(void)
{
    ca(ADDRESS_OF(DISPDEX));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    tc_at(DYNMDISP + 3u);
    intpret();
}

/* DYNMDISP_CODE: VGBODY from VGPREV (S41.1). */
static const word dynmdisp_words[] = {
    ORDERS(VLOAD, CALL), ADDRESS_PLUS_ONE(VGPREV), ADDRESS(S41_1), STORE(VGBODY), ORDER(EXIT),
};

/* After the EXIT: a second's wait (DELAYJOB), */
void dynmdisp_after_exit(void)
{
    set_A(ONE_SEC);
    tc_to(DYNMDISP + 012u, BANKCALL); /* CADR DELAYJOB */
}

/* and round again (TCF DYNMDISP). */
void dynmdisp_after_delayjob(void)
{
    dynmdisp();
}

/* CALCN85: SERVICER's exit (AVEGEXIT) after the burn. CALCN85_CODE. */
void calcn85(void)
{
    tc_at(CALCN85);
    intpret();
}

/* CALCN85_CODE: the velocity to be gained brought up to date (UPDATEVG),
 * and VGBODY from VGPREV (S41.1). */
static const word calcn85_words[] = {
    ORDER(CALL),    ADDRESS(UPDATEVG), ORDERS(VLOAD, CALL), ADDRESS_PLUS_ONE(VGPREV),
    ADDRESS(S41_1), STORE(VGBODY),     ORDER(EXIT),
};

/* After the EXIT: back to SERVICER (POSTJUMP, CADR SERVEXIT). */
void calcn85_after_exit(void)
{
    tc_to(CALCN85 + 010u, POSTJUMP);
}

/* ---- P42 -- pp. 757-758 ---- */

/* P42LM: restart group 4 (PHASCHNG, OCT 04024). */
void p42lm(void)
{
    tc_to(P42LM, PHASCHNG);
}

/* P42LM +2: "INITIALIZATION FOR BURNBABY." (WHICH, P42TABLE). With the
 * descent stage still on (APSFLAG clear), P40ALM's alarm (TC P40ALM);
 * otherwise P42STAGE. MASK leaves A positive or +0. */
void p42lm_after_phaschng(void)
{
    set_A(P42ADRES);
    ts(ADDRESS_OF(WHICH));
    cs(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_to(P42LM + 7u, P40ALM);
        return;
    }
    p42stage();
}

/* P42STAGE: R02. */
void p42stage(void)
{
    tc_to(P42STAGE, BANKCALL); /* CADR R02BOTH */
}

/* P42STAGE +2: "INITIALIZE DVMON" (THRESH2, the count four), and to the
 * interpreter, for P42_CODE. */
void p42stage_after_r02both(void)
{
    set_A(THRESH2);
    ts(ADDRESS_OF(DVTHRUSH));
    set_A(FOUR);
    ts(ADDRESS_OF(DVCNTR));
    tc_at(P42_TC_INTPRET);
    intpret();
}

/* P42_CODE: "LOAD FAPS, MDOTAPS, AND ATDECAY INTO F, MDOT, AND TDECAY BY
 * VECTOR LOAD." (AVFLAG set), and on with APSVEX at P40IN. */
static const word p42_words[] = {
    ORDERS(SET, VLOAD),  SWITCH(SET, AVFLAG),      ADDRESS_PLUS_ONE(FAPS), STORE(F),
    ORDERS(SLOAD, GOTO), ADDRESS_PLUS_ONE(APSVEX), ADDRESS(P40IN),
};

/* ---- P47 -- pp. 758-759 ---- */

/* P47LM: R02, */
void p47lm(void)
{
    tc_to(P47LM, BANKCALL); /* CADR R02BOTH */
}

/* then to the interpreter, for P47_CODE, */
void p47lm_after_r02both(void)
{
    tc_at(P47LM + 2u);
    intpret();
}

/* P47_CODE: the average-G routine started (CALRB MIDTOAV2), which comes
 * back to basic code with a delay in MPAC +1. */
static const word p47_words[] = {
    ORDER(CALRB),
    ADDRESS(MIDTOAV2),
};

/* MIDTOAV2's return: STARTP47 after that wait (TWIDDLE), */
void p47lm_after_midtoav2(void)
{
    ca(ADDRESS_OF(MPAC) + 1u);
    tc_to(P47LM + 6u, TWIDDLE); /* ADRES STARTP47 */
}

/* and the job ends. */
void p47lm_after_twiddle(void)
{
    continue_at(ENDOFJOB);
}

/* STARTP47: restart group 5 (PHASCHNG, OCT 05014, OCT 77777), */
void startp47(void)
{
    tc_to(STARTP47, PHASCHNG);
}

/* STARTP47 +3: SERVICER's exit to CALCN83 (AVEGEXIT gets ACADN83's
 * 2CADR), and P47BODY as a job of priority 20; */
void startp47_after_phaschng(void)
{
    dca(ACADN83); /* EXTEND; DCA */
    dxch(ADDRESS_OF(AVEGEXIT));
    set_A(PRIO20);
    tc_to(STARTP47 + 7u, FINDVAC); /* 2CADR P47BODY */
}

/* then REDO4.2, which "CHECKS PHASE 5 AND GOES TO PREREAD". */
void startp47_after_findvac(void)
{
    continue_at(REDO4_2);
}

/* CALCN83: SERVICER's exit, each pass. CALCN83_CODE. */
void calcn83(void)
{
    tc_at(CALCN83);
    intpret();
}

/* CALCN83_CODE: the velocity counted (DELVCTL plus DELVREF) to DELVSIN,
 * "TEMP STORAGE FOR RESTARTS", and in body axes (S41.1) to DELVIMU. */
static const word calcn83_words[] = {
    ORDERS(VLOAD, VAD),
    ADDRESS_PLUS_ONE(DELVCTL),
    ADDRESS_PLUS_ONE(DELVREF),
    STORE(DELVSIN),
    ORDER(CALL),
    ADDRESS(S41_1),
    STORE(DELVIMU),
    ORDER(EXIT),
};

/* After the EXIT: "REREADAC AND HERE" (PHASCHNG, OCT 10035), */
void calcn83_after_exit(void)
{
    tc_to(CALCN83_EXIT + 1u, PHASCHNG);
}

/* and to the interpreter, for CALCN83_DELVCTL_CODE. */
void calcn83_after_phaschng(void)
{
    tc_at(CALCN83_EXIT + 3u);
    intpret();
}

/* CALCN83_DELVCTL_CODE: DELVCTL from DELVSIN. */
static const word calcn83_delvctl_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(DELVSIN),
    STORE(DELVCTL),
    ORDER(EXIT),
};

/* After the EXIT: back to SERVICER (POSTJUMP, CADR SERVEXIT). */
void calcn83_after_delvctl(void)
{
    tc_to(CALCN83_EXIT2 + 1u, POSTJUMP);
}

/* P47BOD: verb 16 noun 83 (DELVIMU), flashed by GOFLASHR. */
static void p47bod(void)
{
    set_A(V1683);
    tc_to(P47BOD + 1u, BANKCALL); /* CADR GOFLASHR */
}

/* GOFLASHR's returns: terminate and proceed end the program (GOTOPOOH), */
void p47bod_terminate(void)
{
    tc_to(P47BOD + 3u, GOTOPOOH);
}

void p47bod_proceed(void)
{
    tc_to(P47BOD + 4u, GOTOPOOH);
}

/* enter starts again (TCF P47BODY), */
void p47bod_enter(void)
{
    p47body();
}

/* and the immediate return is P40PHS1's (TCF P40PHS1). */
void p47bod_immediate(void)
{
    p40phs1();
}

/* P47BODY: to the interpreter, for P47BODY_CODE. */
void p47body(void)
{
    tc_at(P47BODY);
    intpret();
}

/* P47BODY_CODE: DELVIMU and DELVCTL zero. */
static const word p47body_words[] = {
    ORDER(VLOAD), ADDRESS_PLUS_ONE(HI6ZEROS), STORE(DELVIMU), STORE(DELVCTL), ORDER(EXIT),
};

/* After the EXIT: the display (TC P47BOD). */
void p47body_after_exit(void)
{
    tc_at(P47BODY + 6u);
    p47bod();
}

/* ---- The interpretive code ---- */

const struct interpretive_code P40_CODE = {036u, P40_TC_INTPRET + 1u, COUNT(p40_words), p40_words};
const struct interpretive_code P41_CODE = {036u, P41_TC_INTPRET + 1u, COUNT(p41_words), p41_words};
const struct interpretive_code P41_VGBODY_CODE = {036u, P41_TC_P40SXT4 + 2u,
                                                  COUNT(p41_vgbody_words), p41_vgbody_words};
const struct interpretive_code DYNMDISP_CODE = {036u, DYNMDISP + 4u, COUNT(dynmdisp_words),
                                                dynmdisp_words};
const struct interpretive_code CALCN85_CODE = {036u, CALCN85 + 1u, COUNT(calcn85_words),
                                               calcn85_words};
const struct interpretive_code P42_CODE = {036u, P42_TC_INTPRET + 1u, COUNT(p42_words), p42_words};
const struct interpretive_code P47_CODE = {036u, P47LM + 3u, COUNT(p47_words), p47_words};
const struct interpretive_code CALCN83_CODE = {036u, CALCN83 + 1u, COUNT(calcn83_words),
                                               calcn83_words};
const struct interpretive_code CALCN83_DELVCTL_CODE = {
    036u, CALCN83_EXIT + 4u, COUNT(calcn83_delvctl_words), calcn83_delvctl_words};
const struct interpretive_code P47BODY_CODE = {036u, P47BODY + 1u, COUNT(p47body_words),
                                               p47body_words};

#endif
