/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 2 (Luminary099 pp. 504-512): R22 and R21.
 *
 * R22, "DATA READ ROUTINE 22 (LEM)": "TO PROCESS AUTOMATIC RR MARK DATA TO
 * UPDATE THE STATE VECTOR OF EITHER LM OR CSM AS DEFINED IN THE RENDEZVOUS
 * NAVIGATION PROGRAM (P20)". Each pass, while the rendezvous and track
 * flags stay set and the radar is still in auto track: read the radar
 * (LRS22.1, part 7), check the line of sight is within 30 degrees of +Z
 * (LRS22.2, in orbit), and give the mark to the state vector (LSR22.3,
 * part 8), asking the crew first if the update is too big (verb 06 noun
 * 49). Then hold the preferred tracking attitude (R65LEM) and go round
 * again, or on the surface wait 2 seconds. "R22 WILL CONTINUE TO RECYCLE,
 * UPDATING STATE VECTORS WITH RADAR DATA UNTIL P20 CEASES TO OPERATE
 * (RENDEZVOUS FLAG SET TO ZERO) AT WHICH TIME R22 WILL TERMINATE SELF."
 *
 * R21, "RR DESIGNATE ROUTINE (R21LEM)": "TO POINT THE RENDEZVOUS RADAR AT
 * THE CSM UNTIL AUTOMATIC ACQUISITION OF THE CSM IS ACCOMPLISHED BY THE
 * RADAR." The antenna to its mode center (or, on the surface, mode 2's),
 * then the line of sight half a second ahead (LPS20.1) to RRDESSM, the
 * designate, up to MAXTRIES passes ("ALLOW 60 PASSES (APPROX 45
 * SECONDS)"); alarm 503 if the radar does not lock on. If the line of
 * sight is out of mode 2's coverage on the surface (R21LEM4), the
 * designate looks 10 seconds ahead at a time, up to 60 times, and waits
 * (LONGCALL) to designate continuously from then; alarm 530 if it never
 * comes in. After lock-on R21DISP asks the crew to check it is the main
 * lobe's.
 *
 * As in part 1 (programs.c), each call out of the file leaves Z at the
 * routine and Q at the word after the TC, and a TC or a branch to a label
 * the faithful port keys a routine at stops there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* INDEX MPAC; TC +1 at the address at: MPAC 0, 1 (or 2) picks the TC
 * after it or the next. Not modelled: an MPAC that runs a word past the
 * table's last as an instruction. */
static void index_mpac_tc_plus1(word at, word last)
{
    word tc_plus1 = (word)(at + 1u); /* TC +1: the instruction, TC at +1 */
    word to = indexed_instruction(tc_plus1, fetch(ADDRESS_OF(MPAC)));

    if (to < at + 1u || to > last) not_modelled();
    tc_to(at, to);
}

/* ---- R22 ---- */

static void r22lem1(void);
static void r22lem44(void);
static void r22lem45(void);

/* R22LEM -- p. 504. Restart group 2 at P20LEM1. */
void r22lem(void)
{
    tc_to(R22LEM, PHASCHNG); /* OCT 04022 */
}

/* R22LEM +2 -- "IS RENDEZVOUS FLAG SET?": "NO -- EXIT R22 AND P20". "IS
 * TRACKFLAG SET?": "NO WAIT". R22LEM12: "IS RR AUTO TRACK ENABLE DISCRETE
 * STILL ON (A MONITOR REPOSITION BY R25 CLEARS IT)" (channel 12 bit 14):
 * "NO -- RETURN TO P20". "IS RR AUTO MODE DISCRETE PRESENT" (channel 33
 * bit 2, set when it is not): "NO -- SET IT", P20LEMB5. */
void r22lem_after_phaschng(void)
{
    set_A(RNDVZBIT);
    mask_at(ADDRESS_OF(STATE));
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (A_is_zero()) { /* EXTEND; BZF R22WAIT */
        continue_at(R22WAIT);
        return;
    }
    set_A(BIT14); /* R22LEM12 */
    rand_channel(CHAN12);
    if (A_is_zero()) { /* EXTEND; BZF P20LEMA */
        p20lema();
        return;
    }
    set_A(BIT2);
    rand_channel(CHAN33);
    if (A_is_zero()) { /* EXTEND; BZF +2: "YES CONTINUE" */
        continue_at(R22LEM + 025u);
        return;
    }
    tc_at(R22LEM + 024u);
    p20lemb5();
}

/* R22LEM +025 -- "ARE RR CDUS BEING ZEROED": "CDUS BEING ZEROED", R22LEM42.
 * Else restart group 2 at R22RSTRT ("IF A RESTART OCCURS, AND EXTRA RADAR
 * READING IS TAKEN, SO BAD DATA ISN'T USED"). */
void r22lem_after_p20lemb5(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(RCDU0BIT);
    if (A_is_zero()) { /* EXTEND; BZF R22LEM42 */
        continue_at(R22LEM42);
        return;
    }
    tc_to(R22LEM + 031u, PHASCHNG); /* OCT 00152 */
}

/* R22LEM +033 -- "READ DATA + CALCULATE LOS": LRS22.1, the "DATA READ
 * SUBROUTINE". */
void r22lem_read(void)
{
    tc_to(R22LEM + 033u, BANKCALL); /* CADR LRS22.1 */
}

/* R22LEM +035 -- LRS22.1's answer in MPAC: 0 "NORMAL RETURN (GOOD DATA)",
 * 1 "COULD NOT READ RADAR -- TRY TO REDESIGNATE", 2 "RR LOS NOT WITHIN 3
 * DEGREES (ALARM)". */
void r22lem_after_read(void)
{
    index_mpac_tc_plus1(R22LEM + 036u, R22LEM + 041u);
}

void r22lem_good_data(void)
{
    tc_to(R22LEM + 037u, R22LEM2);
}

void r22lem_no_data(void)
{
    tc_to(R22LEM + 040u, P20LEMC);
}

/* R22LEM +041 -- alarm 525. */
void r22lem_los_alarm(void)
{
    set_A(ALRM525);
    tc_to(R22LEM + 042u, BANKCALL); /* CADR PRIOLARM */
}

/* R22LEM +044 -- "TERMINATE EXITS P20 VIA V56 CODING". */
void r22lem_los_alarm_terminate(void)
{
    tc_to(R22LEM + 044u, GOTOV56);
}

/* R22LEM +045 -- "PROC (DISPLAY DELTA THETA)". */
void r22lem_los_alarm_proceed(void)
{
    tc_at(R22LEM + 045u);
    r22lem1();
}

/* R22LEM +046 -- "ENTER (ILLEGAL OPTION)": TC -5, the alarm again. */
void r22lem_los_alarm_enter(void)
{
    tc_to(R22LEM + 046u, R22LEM + 041u);
}

/* R22LEM +047 -- PRIOLARM's immediate return. */
void r22lem_los_alarm_immediate_return(void)
{
    tc_to(R22LEM + 047u, ENDOFJOB);
}

/* R22LEM1 -- p. 505. Restart group 2 at P20LEM1. */
static void r22lem1(void)
{
    tc_to(R22LEM1, PHASCHNG); /* OCT 04022 */
}

/* R22LEM1 +2 -- "DISPLAY DELTA THETA": verb 06 noun 05, by PRIODSP. */
void r22lem1_after_phaschng(void)
{
    set_A(V06N05);
    tc_to(R22LEM1 + 3u, BANKCALL); /* CADR PRIODSP */
}

void r22lem1_terminate(void)
{
    tc_to(R22LEM1 + 5u, GOTOV56);
}

/* R22LEM1 +6 -- "PROC (OK CONTINUE)". */
void r22lem1_proceed(void)
{
    tc_to(R22LEM1 + 6u, R22LEM2);
}

/* R22LEM1 +7 -- "ENTER (RECYCLE)". */
void r22lem1_enter(void)
{
    tc_to(R22LEM1 + 7u, P20LEMC);
}

/* R22LEM2 -- p. 505. Restart group 2 at P20LEM1. */
void r22lem2(void)
{
    tc_to(R22LEM2, PHASCHNG); /* OCT 04022 */
}

/* R22LEM2 +2 -- "CHECK IF ON LUNAR SURFACE (P22FLAG SET)". */
void r22lem2_after_phaschng(void)
{
    tc_at(R22LEM2 + 2u);
    lunsfchk();
}

/* R22LEM2 +3 -- "YES -- BYPASS FLAG CHECKS AND LRS22.2". */
void r22lem2_on_surface(void)
{
    tc_to(R22LEM2 + 3u, R22LEM3);
}

/* R22LEM2 +4 -- "IS TRACK FLAG SET": "NO -- WAIT". LRS22.2 "CHECKS RR
 * BORESIGHT WITHIN 30 DEG OF +Z". */
void r22lem2_in_orbit(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    if (A_is_zero()) { /* EXTEND; BZF R22WAIT */
        continue_at(R22WAIT);
        return;
    }
    tc_to(R22LEM2 + 010u, BANKCALL); /* CADR LRS22.2 */
}

/* R22LEM2 +012 -- LRS22.2's answer in MPAC: 0 "NORMAL RETURN (LOS WITHIN
 * 30 OF Z-AXIS)", 1 not: then R61LEM, the preferred tracking attitude,
 * and wait. */
void r22lem2_after_boresight(void)
{
    index_mpac_tc_plus1(R22LEM2 + 013u, R22LEM2 + 015u);
}

void r22lem2_boresight_in(void)
{
    tc_to(R22LEM2 + 014u, R22LEM3);
}

void r22lem2_boresight_out(void)
{
    tc_to(R22LEM2 + 015u, BANKCALL); /* CADR R61LEM */
}

/* R22LEM2 +017 -- "NOT WITHIN 30 DEG OF Z-AXIS". */
void r22lem2_after_r61lem(void)
{
    tc_to(R22LEM2 + 017u, R22WAIT);
}

/* R22LEM3 -- p. 505. "SHOULD WE BYPASS STATE VECTOR UPDATE (IS NO UPDATE
 * FLAG SET?)", and "IS UPDATE FLAG SET": either way R22LEM42 if not.
 * "INSURE HIGH PRIO IN RESTART", then LSR22.3 gives the mark to the state
 * vector. */
void r22lem3(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    mask(NOUPFBIT);
    if (A_is_zero()) { /* EXTEND; BZF R22LEM42: "BRANCH -- YES" */
        continue_at(R22LEM42);
        return;
    }
    ca(ADDRESS_OF(FLAGWRD1));
    mask(UPDATBIT);
    if (A_is_zero()) { /* EXTEND; BZF R22LEM42 */
        continue_at(R22LEM42);
        return;
    }
    set_A(PRIO26);
    if (ts(ADDRESS_OF(PHSPRDT2))) not_modelled(); /* (CAF leaves no overflow) */
    tc_at(R22LEM3 + 012u);
    intpret();
}

/* R22LEM3 +013 -- LSR22.3 (part 8) ends at R22LEM93, the "NORMAL EXIT FROM
 * LSR22.3", or at R22LEM96 when the update is too big to take unasked. */
static const word r22lem3_words[] = {
    ORDER(GOTO),
    ADDRESS(LSR22_3),
    /* R22LEM93 */ ORDER(EXIT),
};

/* R22LEM93 +1 -- "PHASE CHANGE TO PROTECT AGAINST CONFLICT WITH GRP2PC
 * ERASEABLE". */
void r22lem93_after_exit(void)
{
    tc_to(R22LEM93 + 1u, PHASCHNG); /* OCT 04022 */
}

void r22lem93_after_phaschng(void)
{
    r22lem44(); /* TCF R22LEM44 */
}

/* R22LEM96 */
static const word r22lem96_words[] = {
    ORDER(EXIT),
};

/* R22LEM96 +1 -- "SET N49FLAG = ZERO TO INDICATE V06 N49 DISPLAY HASN'T
 * BEEN ANSWERED", then the restart group ("TO PROTECT DISPLAY"). */
void r22lem96_after_exit(void)
{
    set_A(ZERO);
    if (ts(ADDRESS_OF(N49FLAG))) not_modelled(); /* (CAF leaves no overflow) */
    tc_to(R22LEM96 + 3u, PHASCHNG);              /* OCT 04022 */
}

/* R22LEM96 +5 -- "PROTECT DISPLAY": N49DSP as a job at priority 27. */
void r22lem96_after_phaschng(void)
{
    set_A(PRIO27);
    tc_to(R22LEM96 + 6u, NOVAC); /* 2CADR N49DSP */
}

/* R22LEM96 +011 -- then wait for the crew's answer. */
void r22lem96_after_novac(void)
{
    tc_at(R22LEM96 + 011u);
    intpret();
}

/* R22LEM96 +012 -- "LOOP TO CHECK IF FLAG SETTING CHANGED -- BRANCH --
 * NO": N49DSP leaves N49FLAG -1 for proceed (R22LEM7), positive for
 * recycle. */
static const word r22lem96_wait_words[] = {
    ORDER(SLOAD),     ADDRESS_PLUS_ONE(N49FLAG),
    ORDERS(BZE, BMN), ADDRESS_AT(R22LEM7, -012), /* -3: the SLOAD */
    ADDRESS(R22LEM7), ORDER(EXIT),               /* "DISPLAY ANSERED BY RECYCLE" */
};

/* R22LEM96 +020 -- "ARE WE ON LUNAR SURFACE". */
void r22lem96_recycle(void)
{
    tc_at(R22LEM96 + 020u);
    lunsfchk();
}

/* R22LEM96 +021 -- "YES -- 15 SECOND DELAY". */
void r22lem96_on_surface(void)
{
    tc_to(R22LEM96 + 021u, R22WAIT);
}

/* R22LEM96 +022 -- "NO -- SET R65COUNTER = 0, DO FINE TRACKING TAKE
 * ANOTHER RADAR READING". */
void r22lem96_in_orbit(void)
{
    set_A(ZERO); /* CA ZERO */
    tc_at(R22LEM96 + 023u);
    r22lem45();
}

/* R22LEM7 -- "PROCEED": "PHASE CHANGE AND GO TO INCOPORATE DATA." */
static const word r22lem7_words[] = {
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(GOTO),
    ADDRESS(ASTOK),
};

/* R22LEM44 -- p. 506. "INCREMENT COUNT OF MARKS INCORPORATED.", then "ARE
 * WE ON LUNAR SURFACE". */
static void r22lem44(void)
{
    incr(ADDRESS_OF(MARKCTR));
    tc_at(R22LEM44 + 1u);
    lunsfchk();
}

/* R22LEM44 +2 -- "YES -- WAIT 2 SECONDS". */
void r22lem44_on_surface(void)
{
    tc_to(R22LEM44 + 2u, R22LEM46);
}

/* R22LEM44 +3 -- "NOT ON LUNAR SURFACE": "R65COUNTER = 5". */
void r22lem44_in_orbit(void)
{
    set_A(FIVE); /* CA FIVE */
    tc_at(R22LEM44 + 4u);
    r22lem45();
}

/* R22LEM42 -- p. 506. No mark taken: "CHECK IF ON LUNAR SURFACE (P22FLAG
 * SET)". */
void r22lem42(void)
{
    tc_at(R22LEM42);
    lunsfchk();
}

/* R22LEM42 +1 -- "YES -- WAIT 2 SECONDS". */
void r22lem42_on_surface(void)
{
    tc_to(R22LEM42 + 1u, R22LEM46);
}

/* R22LEM42 +2 -- "NO -- SET R65COUNTER = 2". */
void r22lem42_in_orbit(void)
{
    set_A(TWO); /* CA TWO */
    r22lem45();
}

/* R22LEM45 -- pp. 506-507. R65CNTR, then R65LEM, the "FINE PREFERRED
 * TRACKING ATTITUDE". */
static void r22lem45(void)
{
    if (ts(ADDRESS_OF(R65CNTR))) not_modelled(); /* (CA of a constant: no overflow) */
    tc_to(R22LEM45 + 1u, BANKCALL);              /* CADR R65LEM */
}

/* R22LEM45 +3 -- and R22 again. */
void r22lem45_after_r65lem(void)
{
    tc_to(R22LEM45 + 3u, R22LEM);
}

/* R22WAIT -- p. 507. 15 seconds (1500DEC) by P20LEMWT's TWIDDLE. */
void r22wait(void)
{
    set_A(DEC_1500);
    tc_at(R22WAIT + 1u); /* TC P20LEMWT +1 */
    p20lemwt_plus1();
}

/* R22LEM46 -- p. 507. "WAIT 2 SECONDS AND TAKE ANOTHER MARK". */
void r22lem46(void)
{
    set_A(TWO_SECS);
    tc_to(R22LEM46 + 1u, BANKCALL); /* CADR DELAYJOB */
}

void r22lem46_after_delayjob(void)
{
    tc_to(R22LEM46 + 3u, R22LEM);
}

/* N49DSP -- p. 507, a job. "EXCESSIVE STATE VECTOR UPDATE -- FLASH VERB 06
 * NOUN 49 R1=DELTA R, R2=DELTA V" by PRIODSP. */
void n49dsp(void)
{
    set_A(V06N49NB);
    tc_to(N49DSP + 1u, BANKCALL); /* CADR PRIODSP */
}

/* N49DSP +3 -- "TERMINATE -- EXIT R22 AND P20". */
void n49dsp_terminate(void)
{
    tc_to(N49DSP + 3u, GOTOV56);
}

/* N49DSP +4 -- "PROCEED -- N49FLAG = -1". (The recycle return, N49DSP
 * +5, leaves N49FLAG the display's positive word.) */
void n49dsp_proceed(void)
{
    set_A(negate(ONE));                          /* CS ONE */
    if (ts(ADDRESS_OF(N49FLAG))) not_modelled(); /* (CS of a constant: no overflow) */
    tc_to(N49DSP + 6u, ENDOFJOB);
}

/* R22RSTRT -- p. 507. "IF A RESTART OCCURS WHILE READING RADAR COME HERE
 * TO TAKE A RANGE-RATE READING WHICH ISN'T USED TO PREVENT TAKING A BAD
 * READING AND TRYING TO INCORPORATE THE BAD DATA". */
void r22rstrt(void)
{
    tc_to(R22RSTRT, PHASCHNG); /* OCT 00152 */
}

void r22rstrt_after_phaschng(void)
{
    tc_to(R22RSTRT + 2u, BANKCALL); /* CADR RRRDOT */
}

/* R22RSTRT +4 -- "WAIT FOR READ COMPLETE". */
void r22rstrt_after_rrrdot(void)
{
    tc_to(R22RSTRT + 4u, BANKCALL); /* CADR RADSTALL */
}

/* R22RSTRT +6 -- "COULD NOT READ RADAR -- TRY TO REDISGNATE". */
void r22rstrt_bad_read(void)
{
    tc_to(R22RSTRT + 6u, P20LEMC);
}

/* R22RSTRT +7 -- "READ SUCCESSFUL -- CONTINUE AT R22". */
void r22rstrt_good_read(void)
{
    tc_to(R22RSTRT + 7u, R22LEM);
}

/* LUNSFCHK -- p. 507. "LUNSFCHECK -- CLOSED SUBROUTINE TO CHECK IF ON
 * LUNAR SURFACE (P22FLAG) RETURNS TO CALLER +1 IF P22FLAG SET TO CALLER +2
 * IF P22FLAG NOT SET": the flag is FLAGWRD8's SURFFLAG. (The masked
 * complement is never negative: not modelled.) */
void lunsfchk(void)
{
    cs(ADDRESS_OF(FLAGWRD8)); /* "CHECK IF ON LUNAR SURFACE" */
    mask(SURFFBIT);           /* "IS SURFFLAG SET?" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        incr(Q_ADDRESS);
        tc_q();
        return;                        /* "NOT SET" */
    case IS_PLUS_ZERO: tc_q(); return; /* "P22FLAG SET" */
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* ---- R21 ---- */

static void r21lem4(void);
static void r21lem6(void);
static void r21lem11(void);
static void r21lem13(void);
static void r21end(void);
static void r21disp(void);

/* R21LEM -- p. 508. "REMOVE RR SELF TRACK ENABLE" (channel 12 bit 14),
 * and on the surface or not? */
void r21lem(void)
{
    set_A(negate(BIT14)); /* CS BIT14 */
    wand_channel(CHAN12);
    tc_at(R21LEM + 3u);
    lunsfchk();
}

/* R21LEM +4 -- on the surface. */
void r21lem_on_surface(void)
{
    tc_to(R21LEM + 4u, R21LEM5);
}

/* R21LEM +5 -- "COMMAND ANTENNA TO MODE CENTER IF NOT ON SURFACE -- MODE 1
 * -- (T=0,S=0)". */
void r21lem_in_orbit(void)
{
    set_A(ZERO);
    if (ts(ADDRESS_OF(TANG))) not_modelled(); /* (CAF leaves no overflow) */
    if (ts(ADDRESS_OF(TANG) + 1u)) not_modelled();
    tc_at(R21LEM + 010u);
    r21lem6();
}

/* R21LEM5 -- p. 508. On the surface: RADMODES bit 12 set goes straight to
 * R21LEM10; else the antenna to mode 2's center (T = BIT15, S = -HALF).
 * (A masked bit 12 is never negative: not modelled.) */
void r21lem5(void)
{
    set_A(BIT12);
    mask_at(ADDRESS_OF(RADMODES));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_to(R21LEM5 + 3u, R21LEM10); return;
    case IS_PLUS_ZERO: continue_at(R21LEM5 + 4u); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* R21LEM5 +4 */
void r21lem5_mode2_center(void)
{
    set_A(BIT15);
    if (ts(ADDRESS_OF(TANG))) not_modelled(); /* (CAF leaves no overflow) */
    set_A(negate(HALF));                      /* CS HALF */
    if (ts(ADDRESS_OF(TANG) + 1u)) not_modelled();
    r21lem6();
}

/* R21LEM6 -- p. 509. LOKONSW cleared, then RRDESNB: the designate with no
 * bank switch... */
static void r21lem6(void)
{
    tc_to(R21LEM6, DOWNFLAG); /* ADRES LOKONSW */
}

void r21lem6_after_downflag(void)
{
    tc_to(R21LEM6 + 2u, BANKCALL); /* CADR RRDESNB */
}

void r21lem6_after_rrdesnb(void)
{
    tc_to(R21LEM6 + 4u, R21LEM6 + 5u); /* TC +1 */
}

/* R21LEM6 +5 -- ...and RADSTALL waits for it. */
void r21lem6_stall(void)
{
    tc_to(R21LEM6 + 5u, BANKCALL); /* CADR RADSTALL */
}

/* R21LEM6 +7 -- "BAD RETURN FROM DESIGNATE -- ISSUE ALARM". */
void r21lem6_bad_return(void)
{
    tc_to(R21LEM6 + 7u, R21_503);
}

/* R21LEM10 -- p. 509. LOSCMFLG: "EVERY FOURTH PASS THRU DODES". */
void r21lem10(void)
{
    tc_to(R21LEM10, UPFLAG); /* ADRES LOSCMFLG */
}

/* R21LEM10 +2 -- "ALLOW 60 PASSES (APPROX 45 SECONDS) TO DESIGNATE AND
 * LOCK ON". */
void r21lem10_after_upflag(void)
{
    set_A(MAXTRIES);
    if (ts(ADDRESS_OF(DESCOUNT))) not_modelled(); /* (CAF leaves no overflow) */
    continue_at(R21LEM2);
}

/* R21LEM2 -- p. 509. LOSCOUNT 3, then R21LEM1: "EXTRAPOLATE TO PRESENT TIME
 * + .5 SEC.", LPS20.1, the "LOS DETERMINATION ROUTINE". */
void r21lem2(void)
{
    set_A(THREE);
    if (ts(ADDRESS_OF(LOSCOUNT))) not_modelled(); /* (CAF leaves no overflow) */
    tc_at(R21LEM1);
    intpret();
}

/* R21LEM1 +1 */
static const word r21lem1_words[] = {
    ORDERS(RTB, DAD), ADDRESS(LOADTIME), ADDRESS_PLUS_ONE(HALFSEC),
    STCALL(TDEC1),    ADDRESS(LPS20_1),  ORDER(EXIT),
};

/* R21LEM3 -- p. 509. "SET LOKONSW TO RADAR -- ON DESIRED", NORRMON
 * cleared, and RRDESSM, the "DESIGNATE ROUTINE" ("INPUT (RRTARGET UPDATED
 * BY LPS20.1)"). */
void r21lem3(void)
{
    tc_to(R21LEM3, UPFLAG); /* ADRES LOKONSW */
}

void r21lem3_after_upflag(void)
{
    tc_to(R21LEM3 + 2u, DOWNFLAG); /* ADRES NORRMON */
}

void r21lem3_after_downflag(void)
{
    tc_at(R21LEM3 + 4u);
    intpret();
}

/* R21LEM3 +5 */
static const word r21lem3_words[] = {
    ORDER(CALL),
    ADDRESS(RRDESSM),
    ORDER(EXIT),
};

/* R21LEM3 +010 -- RRDESSM's returns: "LOS NOT IN MODE 2 COVERAGE ON LUNAR
 * SURFACE"... */
void r21lem3_not_in_coverage(void)
{
    tc_at(R21LEM3 + 010u);
    r21lem4();
}

/* R21LEM3 +011 -- "VEHICLE MANEUVER REQUIRED."... */
void r21lem3_maneuver(void)
{
    tc_at(R21LEM3 + 011u);
    p20lema();
}

/* R21LEM3 +012 -- "NO VEHICLE MANEUVER REQUIRED": "WAIT FOR DESIGNATE
 * COMPLETE -- LOCKON OR BAD END -- LOCKON NOT ACHIEVED IN 60 TRIES". */
void r21lem3_designating(void)
{
    tc_to(R21LEM3 + 012u, BANKCALL); /* CADR RADSTALL */
}

/* R21LEM3 +014 -- the bad end: TC +2, R21-503. */
void r21lem3_bad_end(void)
{
    tc_to(R21LEM3 + 014u, R21_503);
}

/* R21LEM3 +015 -- "EXIT ROUTINE RETURN TO P20 (LOCK-ON)". */
void r21lem3_lockon(void)
{
    tc_at(R21LEM3 + 015u);
    r21end();
}

/* R21-503 -- p. 509. "ISSUE ALARM 503". */
void r21_503(void)
{
    set_A(ALRM503);
    tc_to(R21_503 + 1u, BANKCALL); /* CADR PRIOLARM */
}

/* R21-503 +3 -- "TERMINATE EXITS P20 VIA V56 CODING". */
void r21_503_terminate(void)
{
    tc_to(R21_503 + 3u, GOTOV56);
}

/* R21-503 +4 -- "PROC": the search, R24. */
void r21_503_proceed(void)
{
    tc_to(R21_503 + 4u, R21SRCH);
}

/* R21-503 +5 -- enter: designate again. */
void r21_503_enter(void)
{
    tc_to(R21_503 + 5u, P20LEMC3);
}

/* R21-503 +6 -- PRIOLARM's immediate return. */
void r21_503_immediate_return(void)
{
    tc_to(R21_503 + 6u, ENDOFJOB);
}

/* R21END -- p. 509. "RESET LOSCMFLG". */
static void r21end(void)
{
    tc_to(R21END, DOWNFLAG); /* ADRES LOSCMFLG */
}

/* R21END +2 -- "PUT UP VERIFY MAIN LOBE LOCKON DISPLAY". */
void r21end_after_downflag(void)
{
    tc_at(R21END + 2u);
    r21disp();
}

/* R21SRCH -- p. 509. Restart group 2 at P20LEM1, then R24, the "SEARCH
 * ROUTINE". */
void r21srch(void)
{
    tc_to(R21SRCH, PHASCHNG); /* OCT 04022 */
}

void r21srch_after_phaschng(void)
{
    tc_at(R21SRCH + 2u);
    r24lem();
}

/* R21LEM4 -- p. 510. "SET UP COUNTER FOR 60 PASSES (APPROX 600 SECS.)",
 * "SET FIRST PASS FLAG" and "RESET LOS BEING COMPUTED FLAG". */
static void r21lem4(void)
{
    set_A(MAXTRIES);
    if (ts(ADDRESS_OF(REPOSCNT))) not_modelled(); /* (CAF leaves no overflow) */
    tc_to(R21LEM4 + 2u, UPFLAG);                  /* ADRES FSPASFLG */
}

void r21lem4_after_upflag(void)
{
    tc_to(R21LEM4 + 4u, DOWNFLAG); /* ADRES LOSCMFLG */
}

void r21lem4_after_downflag(void)
{
    tc_at(R21LEM4 + 6u);
    intpret();
}

/* R21LEM12 -- p. 510. "TIME T = T + 10 SECS.", "SAVE FOR LONGCALL AND
 * UPPSV", "COMPUTE LOS AT TIME T" and designate (RRDESSM). */
static const word r21lem12_words[] = {
    /* R21LEM12 */ ORDER(RTB),
    ADDRESS(LOADTIME),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(TENSEC),
    STORE(REPOSTM),
    STCALL(TDEC1),
    ADDRESS(LPS20_1),
    ORDER(CALL),
    ADDRESS(RRDESSM),
    ORDER(EXIT),
};

/* R21LEM12 +012 -- "LOS NOT IN MODE 2 COVERAGE". */
void r21lem12_not_in_coverage(void)
{
    tc_at(R21LEM12 + 012u);
    r21lem13();
}

/* R21LEM12 +013 -- "VEHICLE MANEUVER REQUIRED". */
void r21lem12_maneuver(void)
{
    tc_to(R21LEM12 + 013u, ENDOFJOB);
}

/* R21LEM12 +014 -- in coverage: BEGDES's task killed... */
void r21lem12_in_coverage(void)
{
    tc_to(R21LEM12 + 014u, KILLTASK); /* CADR BEGDES */
}

void r21lem12_after_killtask(void)
{
    tc_at(R21LEM12 + 016u);
    intpret();
}

/* R21LEM12 +017 -- "FIRST PASS THRU REPOSITION": "NO -- GO TO CONTINUOUS
 * DESIGNATE"; "YES -- RESET FIRST PASS FLAG" and R21LEM7 +1. */
static const word r21lem12_first_pass_words[] = {
    ORDER(BOF),   SWITCH(BOF, FSPASFLG),   ADDRESS(R21LEM8),
    ORDER(CLRGO), SWITCH(CLRGO, FSPASFLG), ADDRESS_AT(R21LEM7, 1),
};

/* R21LEM13 -- p. 510. "HAVE WE TRIED 60 TIMES?": "NO -- ADD 10 SECS.
 * RECOMPUTE LOS"; "YES -- PUT OUT ALARM 530". (CCS leaves REPOSCNT - 1 in
 * A for R21LEM7. A negative count goes on at R21LEM7, -0 at R21LEM7 +1.) */
static void r21lem13(void)
{
    switch (ccs_at(ADDRESS_OF(REPOSCNT))) {
    case POSITIVE: tc_to(R21LEM13 + 1u, R21LEM7); return;
    case IS_PLUS_ZERO: continue_at(R21LEM13 + 2u); return;
    case NEGATIVE: continue_at(R21LEM7); return;
    case IS_MINUS_ZERO: tc_at(R21LEM7 + 1u); intpret();
    }
}

/* R21LEM13 +2 -- 60 times. */
void r21lem13_tried_enough(void)
{
    tc_at(R21LEM13 + 2u);
    r21lem11();
}

/* R21LEM7 -- p. 510. The count, and 10 seconds more (R21LEM12 +2). (A
 * count with an overflow is not modelled.) */
void r21lem7(void)
{
    if (ts(ADDRESS_OF(REPOSCNT))) not_modelled();
    tc_at(R21LEM7 + 1u);
    intpret();
}

/* R21LEM7 +1: the TC INTPRET, as the interpreter reads it when CLRGO
 * branches here (R21LEM12 +017): TC INTPRET's word, 06037, is positive, so
 * a store code, STORE,1 (bits 12-14 of 004000), and a pass ends at it.
 * Then 10 seconds more (R21LEM12 +2). R21LEM8: the state vectors to
 * REPOSTM (UPPSV), then...
 * (This copy of the listing spells the label R21LEMB; the assembled
 * listing the ropes come from has R21LEM8, as the scan does.) */
static const word r21lem7_words[] = {
    INTPRET, /* TC INTPRET */
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(REPOSTM),
    ADDRESS_AT(R21LEM12, 2),
    /* R21LEM8 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(REPOSTM),
    STCALL(TDEC1),
    ADDRESS(UPPSV),
    ORDER(EXIT),
};

/* R21LEM8 +5 -- "SET RADMODES BIT 15 FOR CONTINUOUS DESIGNATION"
 * (CDESFLAG), LOKONSW cleared, NORRMON set, and RRDESNB. */
void r21lem8_after_exit(void)
{
    tc_to(R21LEM8 + 5u, UPFLAG); /* ADRES CDESFLAG */
}

void r21lem8_after_cdesflag(void)
{
    tc_to(R21LEM8 + 7u, DOWNFLAG); /* ADRES LOKONSW */
}

void r21lem8_after_lokonsw(void)
{
    tc_to(R21LEM8 + 011u, UPFLAG); /* ADRES NORRMON */
}

void r21lem8_after_norrmon(void)
{
    tc_to(R21LEM8 + 013u, BANKCALL); /* CADR RRDESNB */
}

void r21lem8_after_rrdesnb(void)
{
    tc_to(R21LEM8 + 015u, R21LEM8 + 016u); /* TC +1 */
}

/* R21LEM8 +016 -- "COMPUTE DELTA TIME FOR LONGCALL". */
void r21lem8_delta_time(void)
{
    tc_at(R21LEM8 + 016u);
    intpret();
}

static const word r21lem8_words[] = {
    ORDERS(RTB, BDSU), ADDRESS(LOADTIME), ADDRESS_PLUS_ONE(REPOSTM), STORE(DELTATM), ORDER(EXIT),
};

/* R21LEM8 +024 -- R21LEM9 when the time comes. */
void r21lem8_after_delta_time(void)
{
    dca(ADDRESS_OF(DELTATM));        /* EXTEND; DCA DELTATM */
    tc_to(R21LEM8 + 026u, LONGCALL); /* 2CADR R21LEM9 */
}

void r21lem8_after_longcall(void)
{
    tc_to(R21LEM8 + 031u, ENDOFJOB);
}

/* R21LEM9 -- p. 511, a task: STDESIG's task killed, CLRADMOD, and R21LEM10
 * as a job at priority 26. */
void r21lem9(void)
{
    tc_to(R21LEM9, KILLTASK); /* CADR STDESIG */
}

void r21lem9_after_killtask(void)
{
    tc_to(R21LEM9 + 2u, CLRADMOD);
}

void r21lem9_after_clradmod(void)
{
    set_A(PRIO26);
    tc_to(R21LEM9 + 4u, FINDVAC); /* 2CADR R21LEM10 */
}

void r21lem9_after_findvac(void)
{
    tc_to(R21LEM9 + 7u, TASKOVER);
}

/* R21LEM11 -- p. 511. "ALARM 530 -- LOS NOT IN COVERAGE AFTER TRYING TO
 * DESIGNATE FOR 600 SECS.": each answer ends P20 (GOTOV56). */
static void r21lem11(void)
{
    set_A(ALRM530);
    tc_to(R21LEM11 + 1u, BANKCALL); /* CADR PRIOLARM */
}

void r21lem11_terminate(void)
{
    tc_to(R21LEM11 + 3u, GOTOV56);
}

void r21lem11_proceed(void)
{
    tc_to(R21LEM11 + 4u, GOTOV56);
}

void r21lem11_enter(void)
{
    tc_to(R21LEM11 + 5u, GOTOV56);
}

void r21lem11_immediate_return(void)
{
    tc_to(R21LEM11 + 6u, ENDOFJOB);
}

/* R21DISP -- p. 511. Restart group 2 at P20LEM1. */
static void r21disp(void)
{
    tc_to(R21DISP, PHASCHNG); /* OCT 04022 */
}

/* R21DISP +2 -- "FLASH V 50 N 72 -- PLEASE PERFORM RR MAIN LOBE LOCKON
 * VERIFICATION" (V06N72PV, by GOPERF2R). */
void r21disp_after_phaschng(void)
{
    set_A(V06N72PV);
    tc_to(R21DISP + 3u, BANKCALL); /* CADR GOPERF2R */
}

/* R21DISP +5 -- "TERMINATE EXITS VIA V 56". */
void r21disp_terminate(void)
{
    tc_to(R21DISP + 5u, GOTOV56);
}

/* R21DISP +6 -- "PROCEED CONTINUES TO R22". */
void r21disp_proceed(void)
{
    tc_to(R21DISP + 6u, P20LEMWT);
}

/* R21DISP +7 -- "ENTER ILLEGAL": TC -5, the display again. */
void r21disp_enter(void)
{
    tc_to(R21DISP + 7u, R21DISP + 2u);
}

/* R21DISP +010 -- GOPERF2R's immediate return: "SET BITS TO MAKE THIS A
 * PRIORITY DISPLAY" (LINUS). */
void r21disp_immediate_return(void)
{
    set_A(BIT7);
    tc_to(R21DISP + 011u, LINUS);
}

void r21disp_after_linus(void)
{
    tc_to(R21DISP + 012u, ENDOFJOB);
}

/* ---- The interpretive code ---- */

const struct interpretive_code R22LEM3_CODE = {024u, R22LEM3 + 013u, COUNT(r22lem3_words),
                                               r22lem3_words};
const struct interpretive_code R22LEM96_CODE = {024u, R22LEM96, COUNT(r22lem96_words),
                                                r22lem96_words};
const struct interpretive_code R22LEM96_WAIT_CODE = {
    024u, R22LEM96 + 012u, COUNT(r22lem96_wait_words), r22lem96_wait_words};
const struct interpretive_code R22LEM7_CODE = {024u, R22LEM7, COUNT(r22lem7_words), r22lem7_words};
const struct interpretive_code R21LEM1_CODE = {024u, R21LEM1 + 1u, COUNT(r21lem1_words),
                                               r21lem1_words};
const struct interpretive_code R21LEM3_CODE = {024u, R21LEM3 + 5u, COUNT(r21lem3_words),
                                               r21lem3_words};
const struct interpretive_code R21LEM12_CODE = {024u, R21LEM12, COUNT(r21lem12_words),
                                                r21lem12_words};
const struct interpretive_code R21LEM12_FIRST_PASS_CODE = {
    024u, R21LEM12 + 017u, COUNT(r21lem12_first_pass_words), r21lem12_first_pass_words};
const struct interpretive_code R21LEM7_CODE = {024u, R21LEM7 + 1u, COUNT(r21lem7_words),
                                               r21lem7_words};
const struct interpretive_code R21LEM8_CODE = {024u, R21LEM8 + 017u, COUNT(r21lem8_words),
                                               r21lem8_words};

#endif
