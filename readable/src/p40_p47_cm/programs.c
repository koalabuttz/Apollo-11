/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 1 (Comanche055 pp. 684-690, banks 24 and 27): the
 * start of the burn programs. Their basic code, and their interpretive
 * code in the listing's notation (src/interpreter/notation.h), which the
 * readable interpreter runs (src/binding/cm_p40_routines.c). The faithful
 * C is c64-port/src/cm_p40_p47.c.
 *
 * P40CSM, the SPS burn, takes the steering constant (CSTEER: ECSTEER, or
 * zero for "AN EXTERNAL DELTA V BURN", XDELVFLG set), the engine's
 * thrust (FENG, "20,000 LBS") and the time of ignition, kept as NOMTIG
 * "FOR S40.9"; runs R02 (R02BOTH, the IMU status check); S40.1 computes
 * the thrust direction and the velocity to be gained (VGTIG), S40.2,3 the
 * preferred attitude; then a narrow deadband (SETMINDB) and the maneuver
 * to it (R60CSM). P41CSM, the RCS burn, joins it at P40S/F with the jets'
 * thrust (FRCS2, twice for four jets) and no steering. P41/P40 tells the
 * two apart (MODREG) where they part: P40 shows verb 06 noun 40 and asks
 * for the gimbal test and trim (P40GMB, verb 50 noun 25, "P40CKLS2"),
 * P41 verb 06 noun 85, the velocity to be gained in body axes (P41/DSP,
 * P40CNV85). Both then start CLOKTASK, the countdown's display task, and
 * P40S/SV sets the countdown's first call, TIGBLNK, 30 seconds before
 * ignition (later if the state vector cannot be brought there in time,
 * P40SNEWM). After the burn POSTBURN shows its final values (verb 16 noun
 * 40) and TIGNOW the velocity still to be gained (verb 16 noun 85, which
 * CALCN85 keeps up to date from SERVICER); POST41 gives SERVICER back its
 * own exit and ends the program.
 *
 * Each call out of the file (BANKCALL, DOWNFLAG, UPFLAG, WAITLIST, NOVAC,
 * PHASCHNG, ...) leaves Z at the routine and Q at the word after the TC; a
 * TCF out of it (GOTOPOOH, ENDOFJOB, TASKOVER) leaves Z there. */
#include "p40_p47_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC at, to a routine out of the file: Q gets the return, and the
 * routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q, a subroutine's return: the word in Q runs as the instruction TC
 * <Q>, so Z gets the address and Q becomes 3. Not modelled: a Q (taken
 * overflow-corrected, as the AGC reads a register's word) that is no such
 * address, below 010 or above 07777. */
static void tc_q(void)
{
    word at = overflow_corrected(Q);

    if (at < 010u || at > 07777u) not_modelled();
    Q = 3u;
    Z = at;
}

/* The words of the programs (P40CSM +n and so on). */
#define P40CSM_EXIT  (P40CSM + 016u)
#define P41_DSP_EXIT (P41_DSP + 5u)
#define CALCN85_EXIT (CALCN85 + 5u)

/* ---- P40CSM -- pp. 684-685 ---- */

/* P40CSM: ENG2FLAG clear (DOWNFLAG), the SPS. */
void p40csm(void)
{
    tc_to(P40CSM, DOWNFLAG); /* ADRES ENG2FLAG */
}

/* P40CSM +2: to the interpreter, for P40CSM_CODE. */
void p40csm_after_downflag(void)
{
    tc_at(P40CSM + 2u);
    intpret();
}

/* P40CSM_CODE: "IS THIS AN EXTERNAL DELTA V BURN" (XDELVFLG): "NO CSTEER
 * = ECSTEER", "YES CSTEER = ZERO" (HI6ZEROS); "SET UP THRUST FOR P40
 * 20,000 LBS" (FENG) in F (P40S/F, where "P41 ENTERS HERE"); and "SET
 * ORIGINAL TIME OF IGNITION FOR S40.9" (NOMTIG), as TIG has it: "ORIGINAL
 * TIG MAY BE SLIPPED BY P40S/SV". */
static const word p40csm_words[] = {
    ORDERS(SLOAD, BOFF),
    ADDRESS_PLUS_ONE(ECSTEER), /* "IS THIS AN EXTERNAL DELTA V BURN" */
    SWITCH(BOFF, XDELVFLG),
    ADDRESS(P40S_C), /* "NO CSTEER = ECSTEER" */
    ORDER(DLOAD),    /* "YES CSTEER = ZERO" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* P40S/C */
    STODL(CSTEER),
    ADDRESS_PLUS_ONE(FENG), /* "SET UP THRUST FOR P40 20,000 LBS" */
    /* P40S/F */
    STODL(F), /* "P41 ENTERS HERE" */
    ADDRESS_PLUS_ONE(TIG),
    STORE(NOMTIG), /* "SET ORIGINAL TIME OF IGNITION FOR S40.9" */
    ORDER(EXIT),
};

/* After the EXIT: "IMU STATUS CHECK" (BANKCALL, CADR R02BOTH). */
void p40csm_after_exit(void)
{
    tc_to(P40CSM_EXIT + 1u, BANKCALL); /* CADR R02BOTH */
}

/* P40PVA: to the interpreter, for P40PVA_CODE. */
void p40pva(void)
{
    tc_at(P40PVA);
    intpret();
}

/* P40PVA_CODE: S40.1, "COMPUTE VGTIG,UT"; S40.2,3, "COMPUTE PREFERRED
 * ATTITUDE"; PFRATFLG set. */
static const word p40pva_words[] = {
    ORDER(CALL),       ADDRESS(S40_1),   /* "COMPUTE VGTIG,UT" */
    ORDER(CALL),       ADDRESS(S40_2_3), /* "COMPUTE PREFERRED ATTITUDE" */
    ORDERS(SET, EXIT), SWITCH(SET, PFRATFLG),
};

/* SETMINDB -1 and SETMINDB: "ROUTINE FOR SETTING THE MINIMUM DEADBAND IN
 * AUTOPILOT", "SHOULD BE CALLED UNDER INTERRUPT INHIBITED": the attitude
 * to hold (THETADX, and THETADY and THETADZ, from CDUX, CDUY and CDUZ),
 * ADB the narrow deadband (MINDB), and bit 4 of DAPDATR1 clear. */
static void set_minimum_deadband(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(THETADX));
    dca(ADDRESS_OF(CDUY)); /* EXTEND */
    dxch(ADDRESS_OF(THETADY));
    set_A(MINDB); /* CA MINDB */
    ts(ADDRESS_OF(ADB));
    set_A(negate(BIT4)); /* "EBANK = E6" */
    mask_at(ADDRESS_OF(DAPDATR1));
    ts(ADDRESS_OF(DAPDATR1));
}

/* TCR SETMINDB -1 at a word: INHINT, the deadband, and TC Q back to the
 * word after the TCR. */
static void tcr_setmindb(word at)
{
    tc_at(at);
    inhint();
    set_minimum_deadband();
    tc_q();
}

/* P40SXTY, after P40PVA_CODE's EXIT: "NARROW DEADBAND FOR MANEUVER
 * (EBANK6)" (TCR SETMINDB -1); "ATTITUDE MANEUVER" (R60CSM). */
void p40sxty(void)
{
    tcr_setmindb(P40SXTY);
    p40sxty_after_setmindb();
}

/* P40SXTY +1, SETMINDB's return: RELINT, and R60CSM. */
void p40sxty_after_setmindb(void)
{
    relint();
    tc_to(P40SXTY + 2u, BANKCALL); /* CADR R60CSM */
}

/* P40SXTY +4: NBRCYCLS -1 "FOR UPDATEVG"; "ALLOW CLOCKTASK" (UPFLAG,
 * TIMRFLAG). */
void p40sxty_after_r60csm(void)
{
    set_A(negate(ONE));
    ts(ADDRESS_OF(NBRCYCLS));
    tc_to(P40SXTY + 6u, UPFLAG); /* ADRES TIMRFLAG */
}

/* P41/P40: "P41EXITS AT CALL LOC +1", "P40 EXITS AT CALL LOC +2" (bit 1
 * of MODREG: 41 odd, 40 even). True for P40. */
static bool p41_p40(word at)
{
    tc_at(at);
    cs(ADDRESS_OF(MODREG));
    mask(ONE);
    if (!A_is_zero()) incr(Q_ADDRESS); /* EXTEND; BZF +2 */
    tc_q();
    return Z == (word)(at + 2u);
}

/* P41/DSP: "SET UP FOR NONFLASH V 06 N85 BY CLOCKJOB" (NVWORD1); to the
 * interpreter, for P41_DSP_CODE. */
static void p41_dsp(void)
{
    set_A(V06N85B);
    ts(ADDRESS_OF(NVWORD1));
    tc_at(P41_DSP + 2u);
    intpret();
}

/* P40TTOG: "INITIALIZE FOR CLOCKTASK WHICH IS CALLED" below (NVWORD1,
 * verb 06 noun 40); to the interpreter, for P40TTOG_CODE. */
void p40ttog(void)
{
    set_A(V06N40);
    ts(ADDRESS_OF(NVWORD1));
    tc_at(P40TTOG + 2u);
    intpret();
}

/* P40SXTY +011, where P41/P40 comes back in P41: TC P41/DSP. */
void p40sxty_p41(void)
{
    tc_at(P40SXTY + 011u);
    p41_dsp();
}

/* P40SXTY +010: TC P41/P40, which comes back to the TC P41/DSP (P41) or
 * to P40TTOG (P40). */
void p40sxty_after_upflag(void)
{
    if (p41_p40(P40SXTY + 010u))
        p40ttog();
    else
        p40sxty_p41();
}

/* P40TTOG_CODE: "FOR R2" the magnitude of VGTIG (VGDISP); DVTOTAL +0. */
static const word p40ttog_words[] = {
    ORDERS(VLOAD, ABVAL), /* "FOR R2" */
    ADDRESS_PLUS_ONE(VGTIG), STODL(VGDISP), ADDRESS_PLUS_ONE(HI6ZEROS), STORE(DVTOTAL), ORDER(EXIT),
};

/* P40GMB: "(4.1 PROTECTION)": verb 50 noun 25, checklist 204 (P40CKLS2),
 * the gimbal drive test (GOPERF1). */
static void p40gmb(void)
{
    set_A(P40CKLS2);
    tc_to(P40GMB + 1u, BANKCALL); /* CADR GOPERF1 */
}

/* After P40TTOG_CODE's EXIT: "SET FOR UPDATEVG AND TEST FOR STEERING AFTER
 * AVERAGE G" (AVEGEXIT gets STEERADS's 2CADR), and P40GMB. */
void p40ttog_after_exit(void)
{
    dca(STEERADS); /* EXTEND */
    dxch(ADDRESS_OF(AVEGEXIT));
    p40gmb();
}

/* POST41: SERVICER's exit back to its own (AVEGEXIT gets SERVCADR's
 * 2CADR); the program ends (TCF GOTOPOOH). */
static void post41(void)
{
    dca(SERVCADR); /* EXTEND */
    dxch(ADDRESS_OF(AVEGEXIT));
    continue_at(GOTOPOOH);
}

/* P40GMB +3, "V34": POST41. */
void p40gmb_terminate(void)
{
    post41();
}

/* TRIMONLY +1, where TST,TRIM comes in with A +1: "SET MRKRTEMP FOR GIMBAL
 * TRIM" (MRKRTMP); CNTR "+0 FOR RESTART LOGIC IN S40.6", "+0 SAYS NORMAL
 * ENTRY"; S40.6 in 10 ms (WAITLIST, 2CADR S40.6). */
static void trimonly_plus_1(void)
{
    ts(ADDRESS_OF(MRKRTMP));
    set_A(ZERO);
    ts(ADDRESS_OF(CNTR));
    set_A(ONE);
    tc_to(TRIMONLY + 5u, WAITLIST); /* 2CADR S40.6 */
}

/* P40GMB +4, "V33": TST,TRIM, "SET UP FOR GIMB DRIVE TEST AND TRIM (+1)",
 * on at TRIMONLY +1. */
void p40gmb_proceed(void)
{
    set_A(BIT1); /* TST,TRIM: CAF BIT1 */
    trimonly_plus_1();
}

/* TRIMONLY: MRKRTMP -1, "FOR GIMBAL TRIM". */
void trimonly(void)
{
    set_A(negate(BIT1));
    trimonly_plus_1();
}

/* TRIMONLY +010: "TEST TO FIND TIME TO WAIT FOR GIMBAL TEST": "PLUS, DELAY
 * FOR 18 SECONDS" (18SEC); minus, "DELAY FOR TRIM ONLY TASK" (5SEC); a
 * zero leaves +0 in A (the +0 one by "HOLE", TCF +2). DELAYJOB. */
void trimonly_after_waitlist(void)
{
    switch (ccs_at(ADDRESS_OF(MRKRTMP))) {
    case POSITIVE: set_A(EIGHTEEN_SEC); break;
    case NEGATIVE: set_A(FIVE_SEC); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    tc_to(TRIMONLY + 014u, BANKCALL); /* CADR DELAYJOB */
}

/* TRIMONLY +016: restart groups 6 and 4 (2PHSCHNG, OCT 40026, "6.2 =
 * PRE40.6(-0CS), CLOKTASK(100CS)", and OCT 00234, "4.23 = P40S/SV
 * (PRIO12)"). */
void trimonly_after_delayjob(void)
{
    tc_to(TRIMONLY + 016u, TWO_PHSCHNG);
}

/* P40S/RS: CLOKTASK in 10 ms (WAITLIST, 2CADR CLOKTASK). */
void p40s_rs(void)
{
    set_A(ONE);
    tc_to(P40S_RS + 1u, WAITLIST); /* 2CADR CLOKTASK */
}

/* P40S/SV: a job, which "4.23 PRETECTS" (E7SETTER: EBANK 7, "TC Q"); to
 * the interpreter, for P40S_SV_CODE. */
void p40s_sv(void)
{
    tc_at(P40S_SV);
    set_A(EBANK7); /* E7SETTER */
    ts(EBANK_ADDRESS);
    tc_q();
    p40s_sv_after_e7setter();
}

/* P40S/SV +1, E7SETTER's return: to the interpreter. */
void p40s_sv_after_e7setter(void)
{
    tc_at(P40S_SV + 1u);
    intpret();
}

/* P40S/RS +4: RELINT, and P40S/SV. */
void p40s_rs_after_waitlist(void)
{
    relint();
    p40s_sv();
}

/* P40S_SV_CODE: TDEC1 30 seconds before ignition (SEC29.96), and the
 * state vector integrated to it (CALRB MIDTOAV1, "RETURN IN BASIC"). */
static const word p40s_sv_words[] = {
    ORDERS(DLOAD, DSU), ADDRESS_PLUS_ONE(TIG), ADDRESS_PLUS_ONE(SEC29_96),
    STORE(TDEC1),       ORDER(CALL), /* CALRB, which assembles as CALL */
    ADDRESS(MIDTOAV1),
};

/* P40SET: "DELTA TIME TO PREREAD (INT.INIT.)" (MPAC) less 5 seconds
 * (5SECDP) "FOR TIGBLNK", in P40TMP; TIGBLNK then (LONGCALL, 2CADR
 * TIGBLNK). */
void p40set(void)
{
    dca(ADDRESS_OF(MPAC)); /* EXTEND */
    dxch(ADDRESS_OF(P40TMP));
    dcs(FIVE_SECDP); /* EXTEND */
    das(ADDRESS_OF(P40TMP));
    dca(ADDRESS_OF(P40TMP));        /* EXTEND */
    tc_to(P40SET + 010u, LONGCALL); /* 2CADR TIGBLNK */
}

/* P40S/SV +010, MIDTOAV1's return in basic: TCF +2, P40SET. */
void p40s_sv_after_midtoav1(void)
{
    p40set();
}

/* P40S/SV +011, "INTEGRATION TIME GREATER THAN ALLOWED": TC P40SNEWM, "SET
 * NEW TIG FOR 06 40" (PIPTIME1 plus SEC29.96), and P40SET "FOR LONGCALL
 * OF TIG-30 (OR -35)". */
void p40s_sv_after_midtoav1_fail(void)
{
    tc_at(P40S_SV + 011u);
    dca(ADDRESS_OF(PIPTIME1)); /* P40SNEWM: EXTEND */
    dxch(ADDRESS_OF(TIG));
    dca(SEC29_96); /* EXTEND */
    das(ADDRESS_OF(TIG));
    p40set(); /* TCF P40SET */
}

/* P40SET +013: restart group 4 (PHASCHNG, OCT 20214, "4.21 = TIGBLNK
 * (P40TMP CS)"). */
void p40set_after_longcall(void)
{
    tc_to(P40SET + 013u, PHASCHNG);
}

/* P40SET +015: the job ends (TCF ENDOFJOB). */
void p40set_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* P40BLNKR: "REMOVE RESIDUE" (BANKCALL, CADR CLEANDSP). */
void p40blnkr(void)
{
    tc_to(P40BLNKR, BANKCALL); /* CADR CLEANDSP */
}

/* P40BLNKR +2: the job ends. */
void p40blnkr_after_cleandsp(void)
{
    continue_at(ENDOFJOB);
}

/* ---- After the burn -- pp. 686-687 ---- */

/* POSTBURN: verb 16 noun 40, flashing (BANKCALL, CADR REFLASH). */
void postburn(void)
{
    set_A(V16N40);
    tc_to(POSTBURN + 1u, BANKCALL); /* CADR REFLASH */
}

/* POSTBURN +3, "V34 GO FINISH": POST41. */
void postburn_terminate(void)
{
    post41();
}

/* POSTBURN +4, "PROCEED": P40RCS, where "V99N40 ENTERS HERE ON A P40
 * BYPASS SPS": SERVICER's exit to CALCN85 (ACADN85), "WAIT FOR CALCN85 VIA
 * AVEGEXIT" (DELAYJOB, 2SECS). */
void postburn_proceed(void)
{
    dca(ACADN85); /* P40RCS: EXTEND */
    dxch(ADDRESS_OF(AVEGEXIT));
    set_A(TWO_SECS);
    tc_to(P40RCS + 4u, BANKCALL); /* CADR DELAYJOB */
}

/* POSTBURN +5, "RECYCLE": POSTBURN. */
void postburn_recycle(void)
{
    postburn();
}

/* TIGNOW: "TYPE C GROUP 4 BELOW FOR NOUN 85" (PHASCHNG, OCT 05024, OCT
 * 20000, "PRIO 20"). */
void tignow(void)
{
    tc_to(TIGNOW, PHASCHNG);
}

/* P40MINDB, DELAYJOB's return: TCR SETMINDB -1, RELINT, TIGNOW. */
void p40mindb(void)
{
    tcr_setmindb(P40MINDB);
    p40mindb_after_setmindb();
}

/* P40MINDB +1, SETMINDB's return: RELINT, TIGNOW. */
void p40mindb_after_setmindb(void)
{
    relint();
    tignow();
}

/* TIGNOW +3: verb 16 noun 85, flashing (BANKCALL, CADR REFLASH). */
void tignow_after_phaschng(void)
{
    set_A(V16N85B);
    tc_to(TIGNOW + 4u, BANKCALL); /* CADR REFLASH */
}

/* TIGNOW +6: "FINISH P40/P41" (POST41). */
void tignow_terminate(void)
{
    post41();
}

/* TIGNOW +7, "V03 PROCEED WITH REST OF THE CLEAN-UP": POST41. */
void tignow_proceed(void)
{
    post41();
}

/* TIGNOW +010, "V32 NOT GSOP RESPONSE BUT REDISPLAY N85": TIGNOW. */
void tignow_recycle(void)
{
    tignow();
}

/* SETMINDB: the deadband (above), and TC Q. */
void setmindb(void)
{
    set_minimum_deadband();
    tc_q();
}

/* SETMAXDB: "ROUTINE FOR SETTING THE MAXIMUM DEADBAND IN AUTOPILOT" (ADB
 * MAXDB, bit 4 of DAPDATR1 set), "SHOULD BE CALLED UNDER INTERRUPT
 * INHIBITED"; TC Q. */
void setmaxdb(void)
{
    set_A(MAXDB); /* CA MAXDB */
    ts(ADDRESS_OF(ADB));
    cs(ADDRESS_OF(DAPDATR1));
    mask(BIT4);
    ads(ADDRESS_OF(DAPDATR1));
    tc_q(); /* "EBANK = E6" */
}

/* SETMAXDB -1: INHINT, and SETMAXDB. */
void setmaxdb_inhint(void)
{
    inhint();
    setmaxdb();
}

/* ---- P41 -- pp. 688-690 ---- */

/* TTG/0: a "TASK (4.4 PROTECTS IN P41)": TIGNOW as a job (NOVAC, PRIO20,
 * 2CADR TIGNOW). */
void ttg_0(void)
{
    set_A(PRIO20);
    tc_to(TTG_0 + 1u, NOVAC); /* 2CADR TIGNOW */
}

/* P40CLK: TIMRFLAG clear (DOWNFLAG). */
void p40clk(void)
{
    tc_to(P40CLK, DOWNFLAG); /* ADRES TIMRFLAG */
}

/* P40CLK +2: the task ends (TCF TASKOVER). */
void p40clk_after_downflag(void)
{
    continue_at(TASKOVER);
}

/* P41_DSP_CODE: P40CNV85, "COMPUTE VGTIG IN CTRL COORDS". */
static const word p41_dsp_words[] = {
    ORDER(CALL), /* "COMPUTE" */
    ADDRESS(P40CNV85),
    ORDER(EXIT),
};

/* After P41_DSP_CODE's EXIT: "DO CONTROL COORD CALCULATION AFTER AVEG"
 * (AVEGEXIT gets ACADN85's 2CADR); restart groups 6 and 4 (2PHSCHNG, OCT
 * 40036, "6.3=CLOKTASK(100CS)", and OCT 234, "4.23=P40S/SV(PRIO12)"). */
void p41_dsp_after_exit(void)
{
    dca(ACADN85); /* EXTEND */
    dxch(ADDRESS_OF(AVEGEXIT));
    tc_to(P41_DSP_EXIT + 4u, TWO_PHSCHNG);
}

/* P41/DSP +014: TCF P40S/RS. */
void p41_dsp_after_2phschng(void)
{
    p40s_rs();
}

/* TTGPHS: restart group 4 (NEWPHASE, OCT 4), with A as the phase: "ENTRY
 * FROM P41REDSP (P41) WITH A=-4, OR FROM TIGON (P47) WITH A=-1". */
void ttgphs(void)
{
    tc_to(TTGPHS, NEWPHASE);
}

/* P41REDSP +6: A -4 (CS BIT3), TCF TTGPHS. */
void p41redsp_after_waitlist(void)
{
    set_A(negate(BIT3));
    ttgphs();
}

/* P40CNV85_CODE: the return in QTEMP1 (STQ); VGPREV, which "EQUALS VGTIG
 * (TARGETTING INPUT)", in body axes (S41.1), VGBODY; back through
 * QTEMP1. */
static const word p40cnv85_words[] = {
    ORDERS(STQ, SETPD),  ADDRESS(QTEMP1),          ADDRESS_PLUS_ONE_D(0),
    ORDERS(VLOAD, PUSH), ADDRESS_PLUS_ONE(VGPREV), /* "EQUALS VGTIG (TARGETTING INPUT)" */
    ORDER(CALL),         ADDRESS(S41_1),           STCALL(VGBODY),
    ADDRESS(QTEMP1),
};

/* CALCN85: SERVICER's exit (AVEGEXIT) in P41 and after the burn; to the
 * interpreter, for CALCN85_CODE. */
void calcn85(void)
{
    tc_at(CALCN85);
    intpret();
}

/* CALCN85_CODE: UPDATEVG, "NEW VG, S40.8 (+MAYBE S40.9)"; P40CNV85,
 * "COMPUTE VGBODY". */
static const word calcn85_words[] = {
    ORDER(CALL), ADDRESS(UPDATEVG), /* "NEW VG, S40.8 (+MAYBE S40.9)" */
    ORDER(CALL), ADDRESS(P40CNV85), /* "COMPUTE VGBODY" */
    ORDER(EXIT),
};

/* SERVXT: back to SERVICER (POSTJUMP, CADR SERVEXIT). */
void servxt(void)
{
    tc_to(SERVXT, POSTJUMP); /* CADR SERVEXIT */
}

/* After CALCN85_CODE's EXIT: TC SERVXT. */
void calcn85_after_exit(void)
{
    tc_at(CALCN85_EXIT + 1u);
    servxt();
}

/* TIGBLNK: "CALL TIGAVEG IN FIVE SEC AT TIG-30" (WAITLIST, 2CADR
 * TIGAVEG). */
void tigblnk(void)
{
    set_A(FIVE_SEC);
    tc_to(TIGBLNK + 1u, WAITLIST); /* 2CADR TIGAVEG */
}

/* TIGBLNK +4: NVWORD1 +0, "DISABLE HERE, NOT IN P40BLNKR"; P40BLNKR as a
 * job (NOVAC, PRIO14), "DON'T PROTECT -- RESTARTS BLANK DSKY". */
void tigblnk_after_waitlist(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(NVWORD1));
    set_A(PRIO14);
    tc_to(TIGBLNK + 7u, NOVAC); /* 2CADR P40BLNKR */
}

/* TIGBLNK +012: "4.37 = TIGAVEG (500CS)" (CS OCT37), P40TSK: restart group
 * 4 (NEWPHASE, OCT 4). */
void tigblnk_after_novac(void)
{
    set_A(negate(OCT37));
    tc_to(P40TSK, NEWPHASE);
}

/* P40TSK +2: the task ends (TC TASKOVER). */
void p40tsk_after_newphase(void)
{
    tc_to(P40TSK + 2u, TASKOVER);
}

/* ---- P41CSM -- p. 688 ---- */

/* P41CSM: "SET FOR RCS" (UPFLAG, ENG2FLAG). */
void p41csm(void)
{
    tc_to(P41CSM, UPFLAG); /* ADRES ENG2FLAG */
}

/* P41CSM +2: to the interpreter, for P41CSM_CODE. */
void p41csm_after_upflag(void)
{
    tc_at(P41CSM + 2u);
    intpret();
}

/* P41CSM_CODE: "FOR P41 CSTEER =0" (HI6ZEROS); "2JET THRUST FOR S40.1"
 * (FRCS2), "NJETS = 1 2-JET" (NJETSFLG set), or twice it, "NJETS = 0
 * 4-JET"; on at P40S/F. */
static const word p41csm_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS), /* "FOR P41 CSTEER =0" */
    STORE(CSTEER),
    ORDERS(DLOAD, BON),
    ADDRESS_PLUS_ONE(FRCS2), /* "2JET THRUST FOR S40.1" */
    SWITCH(BON, NJETSFLG),
    ADDRESS(P40S_F),   /* "NJETS = 1 2-JET" */
    ORDERS(DAD, GOTO), /* "NJETS = 0 4-JET" */
    ADDRESS_PLUS_ONE(FRCS2),
    ADDRESS(P40S_F),
};

/* ---- The interpretive code ---- */

const struct interpretive_code P40CSM_CODE = {024u, P40CSM + 3u, COUNT(p40csm_words), p40csm_words};
const struct interpretive_code P40PVA_CODE = {024u, P40PVA + 1u, COUNT(p40pva_words), p40pva_words};
const struct interpretive_code P40TTOG_CODE = {024u, P40TTOG + 3u, COUNT(p40ttog_words),
                                               p40ttog_words};
const struct interpretive_code P40S_SV_CODE = {024u, P40S_SV + 2u, COUNT(p40s_sv_words),
                                               p40s_sv_words};
const struct interpretive_code P41_DSP_CODE = {024u, P41_DSP + 3u, COUNT(p41_dsp_words),
                                               p41_dsp_words};
const struct interpretive_code P40CNV85_CODE = {024u, I_P40CNV85 % 02000u + 02000u,
                                                COUNT(p40cnv85_words), p40cnv85_words};
const struct interpretive_code CALCN85_CODE = {024u, CALCN85 + 1u, COUNT(calcn85_words),
                                               calcn85_words};
const struct interpretive_code P41CSM_CODE = {027u, P41CSM + 3u, COUNT(p41csm_words), p41csm_words};

#endif
