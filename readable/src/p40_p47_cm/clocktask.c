/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 3 (Comanche055 pp. 703-709, bank 24): the countdown's
 * display and the crew's answers to it. The faithful C is
 * c64-port/src/cm_p40_p47.c.
 *
 * CLOKTASK, a task every second while TIMRFLAG is set, keeps TTOGO, the
 * time to ignition (or from it), and has CLOCKJOB show the display NVWORD1
 * names: a plain one (verb 06 noun 40, or 85 in P41, kept up to date by
 * DYNDISP), verb 99 flashing, "SPS ENGINE-ON-ENABLE" (ENGREQST), or verb
 * 97 flashing, "SPS ENGINE-FAILED" (FAILDSP). Verb 99: proceed (V99P)
 * ignites, at once if TIG-0 has come (ENDV99PI) or else when it does;
 * terminate (V99T) and enter (V99E) wipe out the TVC and end the burn.
 * Verb 97: terminate (V97T) shuts the engine down (V97TTASK, V97TRCS),
 * proceed (V97P) carries on the burn, enter (V97E) shuts it down and
 * asks again for ignition (V97ETASK, QUICKIGN, V99FLASH). E7SETTER and
 * E6SETTER set the erasable bank for the routines of the file that use
 * EBANK 7 (TIG, TGO) or 6 (the autopilot's words, the masses).
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC; a TCF out of it leaves Z there. */
#include "p40_p47_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q, a subroutine's return (as in programs.c): Z gets the address in
 * Q, which becomes 3. Not modelled: a Q that is no such address. */
static void tc_q(void)
{
    word at = overflow_corrected(Q);

    if (at < 010u || at > 07777u) not_modelled();
    Q = 3u;
    Z = at;
}

/* ---- CLOKTASK -- p. 703 ---- */

static void clockon(void);

/* CLOKTASK: "IS TIMRFLAG SET" (bit 11 of FLAGWRD7, by CCS A): yes,
 * CLOCKON; no, "KILL RESTART" (PHASCHNG, OCT 00006), */
void cloktask(void)
{
    set_A(BIT11); /* "IS TIMRFLAG SET" */
    mask_at(ADDRESS_OF(FLAGWRD7));
    if (!A_is_zero()) { /* CCS A */
        clockon();
        return;
    }
    tc_to(CLOKTASK + 4u, PHASCHNG);
}

/* and the task ends (TC TASKOVER). */
void cloktask_after_phaschng(void)
{
    tc_to(CLOKTASK + 6u, TASKOVER);
}

/* CLOCKON: TTOGO, TIME2 less TIG. SETCLOCK: CLOKTASK again in a second
 * (WAITLIST). */
static void clockon(void)
{
    dca(ADDRESS_OF(TIME2)); /* EXTEND */
    dxch(ADDRESS_OF(TTOGO));
    dcs(ADDRESS_OF(TIG)); /* EXTEND */
    das(ADDRESS_OF(TTOGO));
    set_A(ONE_SEC);                 /* SETCLOCK */
    tc_to(SETCLOCK + 1u, WAITLIST); /* 2CADR CLOKTASK */
}

static void setupdyn(void);

/* SETCLOCK +4: by NVWORD1 (CCS): +0, no display, SETTB6; verb 06 noun 85
 * ("CHECK FOR V06N85B (P41)"), "UPDATE N85 FOR DYNAMIC DISP" (SETUPDYN);
 * any other, CLOCKJOB as a job (NOVAC, PRIO27). */
void setclock_after_waitlist(void)
{
    if (ccs_at(ADDRESS_OF(NVWORD1)) == IS_PLUS_ZERO) {
        settb6();
        return;
    }
    set_A(negate(V06N85B)); /* "CHECK FOR V06N85B (P41)" */
    ad_at(ADDRESS_OF(NVWORD1));
    if (A_is_zero()) { /* EXTEND; BZF SETUPDYN */
        setupdyn();
        return;
    }
    set_A(PRIO27);
    tc_to(SETCLOCK + 015u, NOVAC); /* 2CADR CLOCKJOB */
}

/* SETTB6: "SET GROUP6 TIMEBASE" (TBASE6, -TIME1); the task ends (TCF
 * TASKOVER). */
void settb6(void)
{
    cs(ADDRESS_OF(TIME1)); /* "SET GROUP6 TIMEBASE" */
    ts(ADDRESS_OF(TBASE6));
    continue_at(TASKOVER);
}

/* SETUPDYN: "SET UP A JOB TO UPDATE N85 (FOR P41=V06)" (FINDVAC, PRIO27,
 * 2CADR DYNDISP), */
static void setupdyn(void)
{
    set_A(PRIO27);
    tc_to(SETUPDYN + 1u, FINDVAC); /* 2CADR DYNDISP */
}

/* and "CLOSE OUT CLOCKTASK" (TCF SETTB6). */
void setupdyn_after_findvac(void)
{
    settb6();
}

/* DYNDISP: "UPDATE N85 FOR A DYNAMIC V06N85 IN P41"; to the interpreter,
 * for DYNDISP_CODE. */
void dyndisp(void)
{
    tc_at(DYNDISP);
    intpret();
}

/* DYNDISP_CODE: P40CNV85, VGBODY. */
static const word dyndisp_words[] = {
    ORDER(CALL),
    ADDRESS(P40CNV85),
    ORDER(EXIT),
};

/* After the EXIT: TCF CKNVWRD1. */
void dyndisp_after_exit(void)
{
    cknvwrd1();
}

/* ---- CLOCKJOB -- p. 705 ---- */

/* CLOCKJOB: the CDUs to CDUSPOTX, CDUSPOTY and CDUSPOTZ; QUICTRIG
 * (BANKCALL), then CKNVWRD1. */
void clockjob(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUSPOTX));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUSPOTY));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(CDUSPOTZ));
    tc_to(CLOCKJOB + 6u, BANKCALL); /* CADR QUICTRIG */
}

/* CKNVWRD1: INHINT; "DETERMINE FUNCTION, INDICATED BY NVWORD1" (CCS):
 * positive, NOFLASH; +0, the job ends (ENDOFJOB); negative, ENGREQST
 * ("SPS ENGINE-ON-ENABLE V99 FLASH"); -0, FAILDSP ("SPS ENGINE-FAILED V97
 * FLASH"). FAILDSP and ENGREQST: verb 06 noun 40 flashed (CLOCPLAY). */
void cknvwrd1(void)
{
    inhint();
    switch (ccs_at(ADDRESS_OF(NVWORD1))) {
    case POSITIVE: /* NOFLASH: "DISPLAY NVWORD1 NORMALLY" */
        ca(ADDRESS_OF(NVWORD1));
        tc_to(NOFLASH + 1u, BANKCALL); /* CADR REGODSP */
        return;
    case IS_PLUS_ZERO: continue_at(ENDOFJOB); return;
    case NEGATIVE: /* ENGREQST */
        set_A(V06N40);
        tc_to(ENGREQST + 1u, BANKCALL); /* CADR CLOCPLAY */
        return;
    case IS_MINUS_ZERO: /* FAILDSP */
        set_A(V06N40);
        tc_to(FAILDSP + 1u, BANKCALL); /* CADR CLOCPLAY */
        return;
    }
}

/* CLOCPLAY's returns after FAILDSP: terminate (V97T), proceed (V97P),
 * enter (V97E); */
void faildsp_terminate(void)
{
    v97t();
}

void faildsp_proceed(void)
{
    v97p();
}

void faildsp_enter(void)
{
    v97e();
}

/* after ENGREQST: terminate (V99T), proceed (V99P), enter (V99E). */
void engreqst_terminate(void)
{
    v99t();
}

void engreqst_proceed(void)
{
    v99p();
}

void engreqst_enter(void)
{
    v99e();
}

/* E7SETTER: EBANK 7; TC Q. */
void e7setter(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_q();
}

/* E6SETTER: "SET UP EBANK6"; TC Q. */
void e6setter(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    tc_q();
}

/* ---- V99 -- pp. 705-706 ---- */

/* TCR TVCZAP -1 at a word: INHINT, TVCZAP, and back to the word after
 * the TCR. */
static void tcr_tvczap_inhint(word at)
{
    tc_at(at);
    inhint();
    tvczap();
}

/* V99E: "KILL PRE40.6/CLOKTASK PROTECTION" and V99EJOB as the restart
 * group 4 job (2PHSCHNG, OCT 00006, OCT 05024, OCT 27000), */
void v99e(void)
{
    tc_to(V99E, TWO_PHSCHNG);
}

/* V99EJOB: "WIPE OUT TVC, CLOKTASK" (TCR TVCZAP -1), */
void v99ejob(void)
{
    tcr_tvczap_inhint(V99EJOB);
    v99ejob_after_tvczap();
}

/* and "V16N85 POST-BURN OPERATIONS" (TCF P40RCS). */
void v99ejob_after_tvczap(void)
{
    p40rcs();
}

/* V99T, "(ENTRY FROM V97T FLOW TOO)": the same, with V99TJOB, */
void v99t(void)
{
    tc_to(V99T, TWO_PHSCHNG);
}

/* V99TJOB: TCR TVCZAP -1, */
void v99tjob(void)
{
    tcr_tvczap_inhint(V99TJOB);
    v99tjob_after_tvczap();
}

/* and POST41: "AVEGEXIT, SETMAXDB, GOTOPOOH". */
void v99tjob_after_tvczap(void)
{
    post41();
}

/* V99P/TIG: "CLEAR THE V99 FLASH AND WAIT FOR TIG-0" (NVWORD1, verb 06
 * noun 40); ENDV99P, the job ends (TCF ENDOFJOB). */
void v99p_tig(void)
{
    set_A(V06N40);
    ts(ADDRESS_OF(NVWORD1));
    continue_at(ENDOFJOB);
}

/* V99P: INHINT; "CHECK ASTN FLAG FOR PRIOR V99P" (bit 12 of FLAGWRD7, by
 * CCS A): set, "THIS MUST BE A RESTART ENTRY", V99P/TIG. ASTNV99P: "SET
 * ASTN FLAG"; "CHECK IGN FLAG FOR TIG-0 ARRIVAL" (bit 13): not yet, "CLEAR
 * THE V99 AND WAIT FOR TIG-0" (V99P/TIG). ENDV99PI: "TIG-0 HAS COME
 * ALREADY", "SET UP IGNITION HERE" (WAITLIST, 2CADR IGNITION), and on at
 * V99P/TIG. */
void v99p(void)
{
    inhint();
    ca(ADDRESS_OF(FLAGWRD7)); /* "CHECK ASTN FLAG FOR PRIOR V99P" */
    mask(BIT12);
    if (!A_is_zero()) { /* CCS A */
        v99p_tig();
        return;
    }
    set_A(BIT12); /* ASTNV99P: "SET ASTN FLAG" */
    ads(ADDRESS_OF(FLAGWRD7));
    ca(ADDRESS_OF(FLAGWRD7)); /* "CHECK IGN FLAG FOR TIG-0 ARRIVAL" */
    mask(BIT13);
    if (A_is_zero()) { /* EXTEND; BZF V99P/TIG */
        v99p_tig();
        return;
    }
    set_A(BIT1);                    /* ENDV99PI: "TIG-0 HAS COME ALREADY" */
    tc_to(ENDV99PI + 1u, WAITLIST); /* 2CADR IGNITION */
}

/* ---- V97 -- pp. 706-709 ---- */

/* V97T: "KILL GROUP 6 (CLOKTASK)" and "4.67 = V97TTASK (-0 CS), TBASE NOW"
 * (2PHSCHNG, OCT 00006, OCT 40674), */
void v97t(void)
{
    tc_to(V97T, TWO_PHSCHNG);
}

/* V97TTASK in a centisecond (TWIDDLE, "KEEP EBANK6 FOR MASSES, SPSOFF,
 * ETC."), */
void v97t_after_2phschng(void)
{
    set_A(BIT1);
    tc_to(V97T + 4u, TWIDDLE); /* ADRES V97TTASK */
}

/* and the job ends (TCF ENDOFJOB). */
void v97t_after_twiddle(void)
{
    continue_at(ENDOFJOB);
}

/* V97TTASK: "DISABLE CLOCKJOB" (NVWORD1 +0); MASSTMP, "3 SECONDS OF MDOT"
 * on CSMMASS, "NOT LOST BECAUSE THRUST FAILED", the "COPYCYCLE FOR
 * MASSBACK" (an overflow would skip into the PHASCHNG's OCT, data: not
 * modelled); restart group 4 (PHASCHNG, OCT 05014, the task below at
 * once). */
void v97ttask(void)
{
    set_A(ZERO); /* "DISABLE CLOCKJOB" */
    ts(ADDRESS_OF(NVWORD1));
    set_A(THREE_MDOT);
    ad_at(ADDRESS_OF(CSMMASS));
    if (ts(ADDRESS_OF(MASSTMP))) not_modelled();
    tc_to(V97TTASK + 5u, PHASCHNG);
}

/* V97TTASK +010: "SHUTDOWN SPS ENGINE, MASS UPDATE, ETC." (TCR SPSOFF), */
void v97ttask_after_phaschng(void)
{
    tc_at(V97TTASK + 010u);
    spsoff();
    v97ttask_after_spsoff();
}

/* "4.71 = V97TRCS (250 CS)" (PHASCHNG, OCT 00714), */
void v97ttask_after_spsoff(void)
{
    tc_to(V97TTASK + 011u, PHASCHNG);
}

/* "DELAY 2.5 SECONDS FOR (POSSIBLE)" tail-off (FIXDELAY, DEC 250), then
 * V97TRCS. */
void v97ttask_tailoff(void)
{
    tc_to(V97TTASK + 013u, FIXDELAY);
}

/* V97TRCS: "RCS DAP IN 0.6SEC" (IBNKCALL, CADR RCSDAPON), */
void v97trcs(void)
{
    tc_to(V97TRCS, IBNKCALL); /* CADR RCSDAPON */
}

/* "SET UP V99T FOR TVCZAP AND POST41" (NOVAC, PRIO27, 2CADR V99T), */
void v97trcs_after_rcsdapon(void)
{
    set_A(PRIO27);
    tc_to(V97TRCS + 3u, NOVAC); /* 2CADR V99T */
}

/* ENDV97T: the task ends (TCF TASKOVER). */
void endv97t(void)
{
    continue_at(TASKOVER);
}

/* V97P: "4.73 = V97PTASK (-0 CS), TBASE NOW" (PHASCHNG, OCT 40734), */
void v97p(void)
{
    tc_to(V97P, PHASCHNG);
}

/* V97PTASK in a centisecond (TWIDDLE), */
void v97p_after_phaschng(void)
{
    set_A(BIT1);
    tc_to(V97P + 3u, TWIDDLE); /* ADRES V97PTASK */
}

/* and the job ends (TCF ENDOFJOB). */
void v97p_after_twiddle(void)
{
    continue_at(ENDOFJOB);
}

/* V97PTASK: "GET MASS UPDATES (TVCEXEC) GOING AGAIN" (VCNTR from
 * V97VCNTR); "REDISPLAY V06N40"; IDLEFAIL set, "TO ALLOW R41-BYPASS"
 * (UPFLAG), */
void v97ptask(void)
{
    ca(ADDRESS_OF(V97VCNTR)); /* "GET MASS UPDATES (TVCEXEC) GOING AGAIN" */
    ts(ADDRESS_OF(VCNTR));
    set_A(V06N40); /* "REDISPLAY V06N40" */
    ts(ADDRESS_OF(NVWORD1));
    tc_to(V97PTASK + 4u, UPFLAG); /* ADRES IDLEFAIL */
}

/* "SET STEERSW TO RE-ENABLE STEERING" (UPFLAG), */
void v97ptask_after_idlefail(void)
{
    tc_to(V97PTASK + 6u, UPFLAG); /* ADRES STEERSW */
}

/* "4.13 = R40ENABL (200 CS)" (PHASCHNG, OCT 00134), */
void v97ptask_after_steersw(void)
{
    tc_to(V97PTASK + 010u, PHASCHNG);
}

/* "WAIT 2 SECONDS" (FIXDELAY, DEC 200), then R40ENABL. */
void v97ptask_after_phaschng(void)
{
    tc_to(V97PTASK + 012u, FIXDELAY);
}

/* R40ENABL: "RE-ENABLE R40 BY CLEARING IDLEFAIL" (DOWNFLAG), */
void r40enabl(void)
{
    tc_to(R40ENABL, DOWNFLAG); /* ADRES IDLEFAIL */
}

/* "KILL GROUP 4" (PHASCHNG, OCT 00004), */
void r40enabl_after_downflag(void)
{
    tc_to(R40ENABL + 2u, PHASCHNG);
}

/* ENDV97P: the task ends (TCF TASKOVER). */
void endv97p(void)
{
    continue_at(TASKOVER);
}

/* V97E: "4.53 = V97ETASK (-0 CS), TBASE NOW" (PHASCHNG, OCT 40534), */
void v97e(void)
{
    tc_to(V97E, PHASCHNG);
}

/* V97ETASK in a centisecond (WAITLIST), */
void v97e_after_phaschng(void)
{
    set_A(BIT1);
    tc_to(V97E + 3u, WAITLIST); /* 2CADR V97ETASK */
}

/* and the job ends (TCF ENDOFJOB). */
void v97e_after_waitlist(void)
{
    continue_at(ENDOFJOB);
}

/* V97ETASK: "FORCE R1 OF V06N40 TO READ 59X59" (TIG -OCT24); "REDISPLAY
 * V06N40"; "RETURN TO EBANK6 FOR REST OF V97ETASK" (TCR E6SETTER). */
void v97etask(void)
{
    set_A(negate(OCT24));
    ts(ADDRESS_OF(TIG));
    set_A(V06N40); /* "REDISPLAY V06N40" */
    ts(ADDRESS_OF(NVWORD1));
    tc_at(V97ETASK + 4u); /* TCR E6SETTER */
    e6setter();
    v97etask_after_e6setter();
}

/* V97ETASK +5, E6SETTER's return: MASSTMP, "3 SECONDS OF MDOT" on CSMMASS,
 * as in V97TTASK (an overflow would skip into data: not modelled);
 * "4.75 = SPSOFF97 (-0 CS)" (PHASCHNG, OCT 00754), which runs on into
 * SPSOFF97. */
void v97etask_after_e6setter(void)
{
    set_A(THREE_MDOT);
    ad_at(ADDRESS_OF(CSMMASS));
    if (ts(ADDRESS_OF(MASSTMP))) not_modelled();
    tc_to(V97ETASK + 010u, PHASCHNG);
}

/* SPSOFF97: TCR SPSOFF, */
void spsoff97(void)
{
    tc_at(SPSOFF97);
    spsoff();
    spsoff97_after_spsoff();
}

/* "4.11 = V97E40.6 (250 CS)" (PHASCHNG, OCT 00114), */
void spsoff97_after_spsoff(void)
{
    tc_to(SPSOFF97 + 1u, PHASCHNG);
}

/* "DELAY 2.5 SECONDS FOR (POSSIBLE)" tail-off (FIXDELAY, DEC 250), then
 * V97E40.6. */
void spsoff97_after_phaschng(void)
{
    tc_to(SPSOFF97 + 3u, FIXDELAY);
}

/* V97E40.6: PRE40.6 in a centisecond (WAITLIST), "USE S40.6 RESTART
 * ENTRY TO TRIM ENGINE", */
void v97e40_6(void)
{
    set_A(BIT1);
    tc_to(V97E40_6 + 1u, WAITLIST); /* 2CADR PRE40.6 */
}

/* "RCS DAP IN 0.6SEC" (IBNKCALL, CADR RCSDAPON), "LEAVE NARROW DEADBAND
 * FOR REIGNITE", */
void v97e40_6_after_waitlist(void)
{
    tc_to(V97E40_6 + 4u, IBNKCALL); /* CADR RCSDAPON */
}

/* restart groups 6 and 4 (2PHSCHNG, OCT 00026, "6.2 = PRE40.6 (-0 CS),
 * CLOKTASK (1 SEC)", and OCT 05014, the task below at once). */
void v97e40_6_after_rcsdapon(void)
{
    tc_to(V97E40_6 + 6u, TWO_PHSCHNG);
}

/* QUICKIGN: "CLEAR ASTNFLAG AND SET IGNFLAG FOR IMMEDIATE V99 RESPONSE"
 * (bits 12 and 13 of FLAGWRD7; an overflow would skip into data: not
 * modelled); "DELAY TO ALLOW TIME FOR PRE40.6" (FIXDELAY, DEC 30), then
 * V99FLASH. */
void quickign(void)
{
    set_A(negate(PRIO14)); /* "CLEAR ASTNFLAG AND SET IGNFLAG FOR" */
    mask_at(ADDRESS_OF(FLAGWRD7));
    ad(BIT13);
    if (ts(ADDRESS_OF(FLAGWRD7))) not_modelled();
    tc_to(QUICKIGN + 4u, FIXDELAY);
}

/* V99FLASH: "CAUSE V99 TO FLASH" (NVWORD1 -BIT9); restart groups 4 and 3
 * (2PHSCHNG, OCT 40774, "4.77 = TIG-0 (-0 CS)", and OCT 00033, "3.3 =
 * S40.13 (PRIO 20)"), */
void v99flash(void)
{
    set_A(negate(BIT9)); /* "CAUSE V99 TO FLASH" */
    ts(ADDRESS_OF(NVWORD1));
    tc_to(V99FLASH + 2u, TWO_PHSCHNG);
}

/* "SET UP TIMEBURN" (FINDVAC, PRIO20, 2CADR S40.13), */
void v99flash_after_2phschng(void)
{
    set_A(PRIO20);
    tc_to(V99FLASH + 6u, FINDVAC); /* 2CADR S40.13 */
}

/* and ENDV97E: the task ends (TCF TASKOVER), to "WAIT FOR CLOCKJOB
 * (IMMEDIATE) REACTION TO FLASHING V99 RESPONSE". */
void endv97e(void)
{
    continue_at(TASKOVER);
}

/* ---- The interpretive code ---- */

const struct interpretive_code DYNDISP_CODE = {024u, DYNDISP + 1u, COUNT(dyndisp_words),
                                               dyndisp_words};

#endif
