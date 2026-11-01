/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 1 (Luminary099 pp. 492-503): the programs.
 *
 * P20, "RENDEZVOUS NAVIGATION PROGRAM 20": "THE PURPOSE OF THIS PROGRAM IS
 * TO CONTROL THE RENDEZVOUS RADAR FROM STARTUP THROUGH ACQUISITION AND
 * LOCKON TO THE CSM AND TO UPDATE EITHER THE LM OR CSM STATE VECTOR (AS
 * SPECIFIED BY THE ASTRONAUT BY DSKY ENTRY) ON THE BASIS OF THE RR
 * TRACKING DATA." P22 is P20 on the lunar surface (PROG22 = PROG20): it
 * updates the CSM's state vector, after the crew has a chance to give the
 * CSM's orbit change (ORBCHGO to ORBCHG3). Then (PROG20A) the flags that
 * start tracking; the range test (P20LEM1, LPS20.1 and LPS20.2 in part
 * 7); in orbit, the preferred tracking attitude (R61LEM); then, while the
 * track flag is set, the radar: automatic acquisition when the radar is in
 * its auto mode (P20LEMB3, R21's designate), else a request to the crew to
 * select it, or manual acquisition (R23, when they key enter). P20LEMC
 * and the 2.5-second task P20LEMC1 keep the radar read job (R22) going;
 * UPPSV brings both permanent state vectors up to the present time first.
 *
 * P25, "PREFERRED TRACKING ATTITUDE PROGRAM P25": "THE PURPOSE OF THIS
 * PROGRAM IS TO COMPUTE THE PREFERRED TRACKING ATTITUDE OF THE LM TO
 * CONTINUOUSLY POINT THE LM TRTACKING BEACON AT THE CSM AND TO PERFORM THE
 * MANEUVER TO THE PREFERRED TRACKING ATTITUDE AND CONTINUOUSLY MAINTAIN
 * THIS ATTITUDE WITHIN PRESCRIBED LIMITS": R65 once a minute.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, 2PHSCHNG, PRIOCHNG,
 * UPFLAG, DOWNFLAG, FALTON, TWIDDLE, FIXDELAY, FINDVAC) leaves Z at the
 * routine and Q at the word after the TC, where the routine comes back to;
 * ENDOFJOB, TASKOVER, GOTOPOOH and GOTOV56 don't come back. A TC or a
 * branch to a label of the file that the faithful port keys a routine at
 * (R21LEM, CLRADMOD, P20LEMB, ...) stops there too: that routine runs
 * next, as the faithful port's does. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P20 (P22) ---- */

static void orbchgo_display(void);
static void orbchg1_display(void);
static void prog20a_minus2(void);
static void p20lemb6(void);
static void p20lemc2(void);
static void p20lmwt1(void);

/* PROG20 -- p. 492. "ASTRONAUT REQUEST THROUGH DSKY V37E20E" (or V37E22E:
 * "PROG22 = PROG20"). 2PHSCHNG: "OCT 4" makes group 4 inactive, and OCT
 * 05022 starts group 2 at PROG20 +4's phase "PRIORITY 26" (OCT 26000). */
void prog20(void)
{
    tc_to(PROG20, TWO_PHSCHNG);
}

/* PROG20 +4 -- "CHECK IF ON LUNAR SURFACE": LUNSFCHK comes back to
 * PROG20 +5 if so, PROG20 +6 if not. */
void prog20_after_2phschng(void)
{
    tc_at(PROG20 + 4u);
    lunsfchk();
}

/* PROG20 +5 -- "YES": P22. */
void prog20_on_surface(void)
{
    tc_to(PROG20 + 5u, ORBCHGO);
}

/* PROG20 +6 -- "NO -- CONTINUE WITH P20". */
void prog20_in_orbit(void)
{
    tc_at(PROG20 + 6u);
    prog20a_minus2();
}

/* ORBCHGO -- p. 493. "SET VEHUPFLG -- CSM STATE VECTOR TO BE UPDATED". */
void orbchgo(void)
{
    tc_to(ORBCHGO, UPFLAG); /* ADRES VEHUPFLG */
}

/* ORBCHGO +2 -- "SET R2 FOR OPTION CSM WILL NOT CHANGE PRESENT ORBIT":
 * OPTION2 is 1. (CAF ONE leaves no overflow for the TS to skip on.) */
void orbchgo_after_upflag(void)
{
    set_A(ONE);
    if (ts(ADDRESS_OF(OPTION2))) not_modelled();
    orbchgo_display();
}

/* ORBCHGO +4 -- "DISPLAY ASSUMED CSM ORBIT OPTION": GOPERF4, through
 * BANKCALL, shows OCT00012's option code (option 12, R2 the orbit option)
 * for the crew to accept, or load. */
static void orbchgo_display(void)
{
    set_A(OCT00012);
    tc_to(ORBCHGO + 5u, BANKCALL); /* CADR GOPERF4 */
}

/* ORBCHGO +7 -- "TERMINATE". */
void orbchgo_terminate(void)
{
    tc_to(ORBCHGO + 7u, GOTOPOOH);
}

/* ORBCHGO +010 -- "PROCEED VALUE OF ASSUMED OPTION OK". */
void orbchgo_proceed(void)
{
    tc_to(ORBCHGO + 010u, ORBCHG1);
}

/* ORBCHGO +011 -- "R2 LOADED THRU DSKY": TC -5, the display again. */
void orbchgo_enter(void)
{
    tc_at(ORBCHGO + 011u);
    orbchgo_display();
}

/* ORBCHG1 -- p. 493. OPTION2 1, the CSM keeps its orbit: on to PROG20A.
 * Otherwise "FLASH VERB-NOUN TO REQUEST ESTIMATED TIME OF LAUNCH". */
void orbchg1(void)
{
    set_A(negate(P22ONE)); /* CS P22ONE */
    ad_at(ADDRESS_OF(OPTION2));
    if (A_is_zero()) { /* EXTEND; BZF PROG20A */
        continue_at(PROG20A);
        return;
    }
    orbchg1_display();
}

/* ORBCHG1 +4 -- verb 06 noun 33 (V06N33*) flashed by GOFLASH. Noun 33 is
 * "TIME OF IGNITION" in the noun list (ASSEMBLY_AND_OPERATION_INFORMATION):
 * TIG, here the estimated launch time. */
static void orbchg1_display(void)
{
    set_A(V06N33_STAR);
    tc_to(ORBCHG1 + 5u, BANKCALL); /* CADR GOFLASH */
}

/* ORBCHG1 +7 -- "TERMINATE". */
void orbchg1_terminate(void)
{
    tc_to(ORBCHG1 + 7u, GOTOPOOH);
}

/* ORBCHG1 +010 -- "PROCEED VALUES OK". */
void orbchg1_proceed(void)
{
    tc_to(ORBCHG1 + 010u, ORBCHG2);
}

/* ORBCHG1 +011 -- "TIME LOADED THRU DSKY": TC -5, the display again. */
void orbchg1_enter(void)
{
    tc_at(ORBCHG1 + 011u);
    orbchg1_display();
}

/* ORBCHG2 -- p. 493. */
void orbchg2(void)
{
    tc_at(ORBCHG2);
    intpret();
}

/* ORBCHG2 +1 -- to ORBCHG3, in bank 32. */
static const word orbchg2_words[] = {
    ORDER(GOTO),
    ADDRESS(ORBCHG3),
};

/* ORBCHG3 -- pp. 493-495, bank 32. The CSM's orbit change: both vehicles
 * are integrated to the launch time TIG ("ESTIMATED LAUNCH TIME"), the LM
 * first (RSUBL, "SAVE LM POSITION"), then the CSM, with its W matrix if
 * RENDWFLG says it is valid. The CSM's orbital plane is turned to hold the
 * LM's position: UCSM is "UNIT(R(LM) CROSS NSUB1)", NSUB1 the plane's
 * normal, and TIMETHET gives the transfer time T and the CSM's position
 * and velocity at the end of its turn through the angle between UCSM and
 * its position (CSTH and SNTH its cosine and sine). That velocity, turned
 * into the plane of NEWPOS and the LM's position, is the "NEW CSM
 * VELOCITY", NCSMVEL, at "LAUNCH TIME -- TRANSFER TIME", TET. MINIRECT
 * starts the CSM's state vector there (RRECT, VRECT), ATOPCSM makes it the
 * CSM's (X2 2) and INTWAKE0 ends the integration. */
static const word orbchg3_words[] = {
    /* ORBCHG3 */ ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TIG),
    STORE(LNCHTM),
    STORE(TDEC1), /* "ESTIMATED LAUNCH TIME" */
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, VINTFLAG), /* "LM INTEGRATION" */
    SWITCH(CLEAR, INTYPFLG), /* "PRECISION -- ENCKE" */
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DIM0FLAG), /* "NO W-MATRIX" */
    SWITCH(CLEAR, D6OR9FLG),
    ORDER(CALL),
    ADDRESS(INTEGRV), /* "PLANETARY INERTIAL ORIENTATION" */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT1),
    STODL(RSUBL), /* "SAVE LM POSITION" */
    ADDRESS_PLUS_ONE(TAT),
    STCALL(TDEC1),
    ADDRESS(INTSTALL),
    ORDERS(SET, CLEAR),
    SWITCH(SET, VINTFLAG), /* "CSM INTEGRATION" */
    SWITCH(CLEAR, INTYPFLG),
    ORDERS(CLEAR, BOFF),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(BOF, RENDWFLG), /* "W MATRIX VALID" */
    ADDRESS(NOWMATX),      /* "NO" */
    ORDERS(SET, SET),      /* "YES -- SET FOR W MATRIX" */
    SWITCH(SET, DIM0FLAG),
    SWITCH(SET, D6OR9FLG),
    /* NOWMATX */ ORDER(CALL),
    ADDRESS(INTEGRV), /* "CSM INTEGRATION" */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VATT1),
    STOVL(VSUBC), /* "SAVE CSM POSITION" (its velocity) */
    ADDRESS_PLUS_ONE(RATT1),
    STORE(RSUBC),            /* "SAVE CSM POSITION" */
    ORDERS(VXV, UNIT),       /* "COMPUTE NORMAL TO CSM ORBITAL PLANE" */
    ADDRESS_PLUS_ONE(VSUBC), /* "NSUB1=UNIT(R(CM) CROSS V(CM))" */
    STOVL_D(20),             /* "SAVE NSUB1" */
    ADDRESS_PLUS_ONE(RSUBL), /* "COMPUTE ESTIMATED ORBITAL */
    ORDERS(VXV, UNIT),       /* PLANE CHANGE" */
    ADDRESS_PLUS_ONE_D(20),  /* "UCSM = UNIT(R(LM) CROSS NSUB1)" */
    STOVL(UCSM),
    ADDRESS_PLUS_ONE(RSUBC), /* "COMPUTE ANGLE BETWEEN UCSM */
    ORDERS(UNIT, DOT),       /* AND RSUBC" */
    ADDRESS_PLUS_ONE(UCSM),  /* "COS A = UCSM DOT UNIT (R(CM))" */
    ORDER(SL1),
    STORE(CSTH),       /* "SAVE DOE TIME-THETA SUBROUTINE" */
    ORDERS(DSQ, BDSU), /* "COMPUTE SINE A" */
    ADDRESS_PLUS_ONE(ONEB_2),
    ORDER(SQRT),
    STOVL(SNTH),             /* "SAVE FOR TIME-THETA SUBROUTINE" */
    ADDRESS_PLUS_ONE(RSUBC), /* "POSITION OF CSM AT EST. LAUNCH" */
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(VSUBC), /* "VELOCITY OF CSM AT EST. LAUNCH." */
    ORDER(VCOMP),
    STORE(VVEC),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, RVSW),
    ADDRESS(TIMETHET),
    ORDER(VCOMP),
    STORE(NEWVEL), /* "TERMINAL VELOCITY OF CSM" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(T),
    STOVL(TRANSTM), /* "TRANSFER TIME" */
    ADDRESS_PLUS_ONE(NEWVEL),
    ORDER(ABVAL),
    STOVL_D(20),
    ADDRESS_PLUS_ONE_D(0),
    STORE(NEWPOS),            /* "TERMINAL POSITION OF CSM" */
    ORDERS(VXV, UNIT),        /* "COMPUTE NORMAL TO SCM ORBITAL PLANE" */
    ADDRESS_PLUS_ONE(RSUBL),  /* "NSUB2 = UNIT(NEWPOS CROSS R(LM))" */
    ORDERS(VXV, UNIT),        /* "ROTATE TERMINAL VEL INTO DESIRED */
    ADDRESS_PLUS_ONE(NEWPOS), /* ORBITAL PLANE" */
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE_D(20),
    STCALL(NCSMVEL), /* "NEW CSM VELOCITY" */
    ADDRESS(GRP2PC),
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(DLOAD, BDSU),
    ADDRESS_PLUS_ONE(TRANSTM), /* "LAUNCH TIME -- TRANSFER TIME" */
    ADDRESS_PLUS_ONE(LNCHTM),
    STOVL(TET),
    ADDRESS_PLUS_ONE(NEWPOS),
    STORE(RCV),
    STOVL(RRECT),
    ADDRESS_PLUS_ONE(NCSMVEL),
    STCALL(VRECT),
    ADDRESS(MINIRECT),
    ORDERS(AXT_2, CALL),
    NUMBER(2),
    ADDRESS(ATOPCSM),
    ORDER(CALL),
    ADDRESS(INTWAKE0),
    ORDER(EXIT),
};

/* ORBCHG3 +0145 -- back to bank 24, P20 proper. */
void orbchg3_after_exit(void)
{
    tc_to(ORBCHG3 + 0145u, BANKCALL); /* CADR PROG20A */
}

/* PROG20A -2 -- p. 495. "RESET VEHUPFLG -- LM STATE VECTOR TO BE
 * UPDATED". */
static void prog20a_minus2(void)
{
    tc_to(PROG20A - 2u, DOWNFLAG); /* ADRES VEHUPFLG */
}

/* PROG20A -- R02BOTH, "IMU STATUS CHECK". */
void prog20a(void)
{
    tc_to(PROG20A, BANKCALL); /* CADR R02BOTH */
}

/* PROG20A +2 to +022 -- the flags, one call each. */
void prog20a_after_r02both(void)
{
    tc_to(PROG20A + 2u, UPFLAG); /* ADRES UPDATFLG: "SET UPDATE FLAG" */
}

void prog20a_after_updatflg(void)
{
    tc_to(PROG20A + 4u, UPFLAG); /* ADRES TRACKFLG: "SET TRACK FLAG" */
}

void prog20a_after_trackflg(void)
{
    tc_to(PROG20A + 6u, UPFLAG); /* ADRES RNDVZFLG: "SET RENDEZVOUS FLAG" */
}

void prog20a_after_rndvzflg(void)
{
    tc_to(PROG20A + 010u, DOWNFLAG); /* ADRES SRCHOPTN: "INSURE SEARCH OPTION OFF" */
}

void prog20a_after_srchoptn(void)
{
    tc_to(PROG20A + 012u, DOWNFLAG); /* ADRES ACMODFLG: "ALSO MANUAL ACQUISITION FLAG RESET" */
}

/* "TURN OFF R04FLAG TO ENSURE GETTING ALARM 521 IF CAN'T READ RADAR". */
void prog20a_after_acmodflg(void)
{
    tc_to(PROG20A + 014u, DOWNFLAG); /* ADRES R04FLAG */
}

/* "ENSURE R25 GIMBAL MONITOR IS ENABLED (RESET NORRMON FLAG)". */
void prog20a_after_r04flag(void)
{
    tc_to(PROG20A + 016u, DOWNFLAG); /* ADRES NORRMON */
}

/* "RESET LOS BEING COMPUTED FLAG". */
void prog20a_after_norrmon(void)
{
    tc_to(PROG20A + 020u, DOWNFLAG); /* ADRES LOSCMFLG */
}

/* PROG20A +022 -- CLRADMOD (part 4) clears RADMODES' bits 10 and 15, and
 * comes back to P20LEM1. */
void prog20a_after_loscmflg(void)
{
    tc_to(PROG20A + 022u, CLRADMOD);
}

/* P20LEM1 -- p. 496. Restart group 2 at P20LEM1. */
void p20lem1(void)
{
    tc_to(P20LEM1, PHASCHNG); /* OCT 04022 */
}

/* P20LEM1 +2 -- "ZERO MARK COUNTER", then the "LOS DETERMINATION
 * ROUTINE". (CAF ZERO leaves no overflow for the TS to skip on.) */
void p20lem1_after_phaschng(void)
{
    set_A(ZERO);
    if (ts(ADDRESS_OF(MARKCTR))) not_modelled();
    tc_at(P20LEM1 + 4u);
    intpret();
}

/* P20LEM1 +5 -- LPS20.1 (part 7) works out the line of sight to the CSM
 * at the present time (TDEC1), and LPS20.2, the "TEST RANGE R/UTINE",
 * leaves MPAC 0 within 400 nautical miles, else 1. */
static const word p20lem1_words[] = {
    ORDER(RTB),       ADDRESS(LOADTIME), STCALL(TDEC1),
    ADDRESS(LPS20_1), ORDER(CALL),       ADDRESS(LPS20_2), /* "TEST RANGE R/UTINE" */
    ORDER(EXIT),
};

/* P20LEM1 +014 -- INDEX MPAC; TC +1: MPAC 0 (or -0, which indexes
 * nothing) runs the TC at P20LEM1 +016, "NORMAL RETURN WITHIN 400 N M"; 1
 * the word after it, 526ALARM. Not modelled: any other MPAC, which would
 * run a word of 526ALARM as an instruction. */
void p20lem1_after_exit(void)
{
    word tc_plus1 = (word)(P20LEM1 + 016u); /* TC +1: the instruction, TC P20LEM1 +016 */
    word to = indexed_instruction(tc_plus1, fetch(ADDRESS_OF(MPAC)));

    if (to != P20LEM1 + 016u && to != FIVE_TWENTY_SIX_ALARM) not_modelled();
    tc_to(P20LEM1 + 015u, to);
}

/* P20LEM1 +016 -- "NORMAL RETURN WITHIN 400 N M". */
void p20lem1_in_range(void)
{
    tc_at(P20LEM1 + 016u);
    p20lema();
}

/* 526ALARM -- p. 496. "ERROR EXIT -- RANGE > 400 N. MI.": priority alarm
 * 526, by PRIOLARM. */
void five_twenty_six_alarm(void)
{
    set_A(ALRM526);
    tc_to(FIVE_TWENTY_SIX_ALARM + 1u, BANKCALL); /* CADR PRIOLARM */
}

/* 526ALARM +3 -- "TERMINATE EXITS P20 VIA V56 CODING". */
void five_twenty_six_alarm_terminate(void)
{
    tc_to(FIVE_TWENTY_SIX_ALARM + 3u, GOTOV56);
}

/* 526ALARM +4 -- "PROC (ILLEGAL)": TC -4, the alarm again. */
void five_twenty_six_alarm_proceed(void)
{
    tc_to(FIVE_TWENTY_SIX_ALARM + 4u, FIVE_TWENTY_SIX_ALARM);
}

/* 526ALARM +5 -- "ENTER RECYCLE". */
void five_twenty_six_alarm_enter(void)
{
    tc_to(FIVE_TWENTY_SIX_ALARM + 5u, P20LEM1);
}

/* 526ALARM +6 -- PRIOLARM's immediate return: the job ends, to wait for
 * the crew. */
void five_twenty_six_alarm_immediate_return(void)
{
    tc_to(FIVE_TWENTY_SIX_ALARM + 6u, ENDOFJOB);
}

/* P20LEMA -- p. 496. Restart group 2 at P20LEM1 still. R22 and R21 come
 * here too. */
void p20lema(void)
{
    tc_to(P20LEMA, PHASCHNG); /* OCT 04022 */
}

/* P20LEMA +2 -- "CHECK LUNAR SURFACE FLAG (P22 FLAG)": LUNSFCHK comes
 * back to P20LEMA +3 if it is set, P20LEMA +4 if not. */
void p20lema_after_phaschng(void)
{
    tc_at(P20LEMA + 2u);
    lunsfchk();
}

/* P20LEMA +3 -- on the surface: no attitude to hold. */
void p20lema_on_surface(void)
{
    tc_to(P20LEMA + 3u, P20LEMB);
}

/* P20LEMA +4 -- "PREFERRED TRACKING ATTITUDE ROUTINE", R61LEM. */
void p20lema_in_orbit(void)
{
    tc_to(P20LEMA + 4u, BANKCALL); /* CADR R61LEM */
}

/* P20LEMB -- p. 496. "RESTART AT PRIORITY 10 TO ALLOW V37 REQUESTED
 * PROGRAM TO RUN FIRST": OCT 05022, OCT 10000. */
void p20lemb(void)
{
    tc_to(P20LEMB, PHASCHNG);
}

/* P20LEMB +3 -- "RESTORE PRIORITY 26". */
void p20lemb_after_phaschng(void)
{
    set_A(PRIO26);
    tc_to(P20LEMB + 4u, PRIOCHNG);
}

/* P20LEMB +5 -- "IS THE TRACK FLAG SET": "NO -- WAIT FOR IT TO BE SET"
 * (P20LEMWT). P20LEMB7: "IS RR AUTO MODE DISCRETE PRESENT" (channel 33's
 * bit 2, set when it is not): "YES -- DO AUTOMATIC ACQUISITION (R21)". */
void p20lemb_after_priochng(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    if (A_is_zero()) { /* EXTEND; BZF P20LEMWT */
        continue_at(P20LEMWT);
        return;
    }
    set_A(BIT2); /* P20LEMB7 */
    rand_channel(CHAN33);
    if (A_is_zero()) { /* EXTEND; BZF P20LEMB3 */
        continue_at(P20LEMB3);
        return;
    }
    p20lemb5();
}

/* P20LEMB5 -- p. 497. "RADAR NOT IN AUTO CHECK IF MAJOR MODE IS 20" (or
 * 22): "OKAY TO DO PLEASE PERFORM", P20LEMB6. Otherwise "TRACK FLAG SET --
 * FLASH PRIORITY ALARM 514 -- RADAR GOES OUT OF AUTO MODE WHILE IN USE".
 * R22 comes here too. */
void p20lemb5(void)
{
    set_A(negate(OCT24)); /* CS OCT24 */
    ad_at(ADDRESS_OF(MODREG));
    if (A_is_zero()) { /* EXTEND; BZF P20LEMB6 */
        p20lemb6();
        return;
    }
    ad(NEG2);          /* "ALSO CHECK FOR P22" */
    if (A_is_zero()) { /* EXTEND; BZF P20LEMB6 */
        p20lemb6();
        return;
    }
    set_A(ALRM514);
    tc_to(P20LEMB5 + 010u, BANKCALL); /* CADR PRIOLARM */
}

/* P20LEMB5 +012 -- "TERMINATE EXITS VIA V56". */
void p20lemb5_terminate(void)
{
    tc_to(P20LEMB5 + 012u, GOTOV56);
}

/* P20LEMB5 +013, +014 -- "PROCEED AND ENTER BOTH GO BACK TO CHECK AUTO
 * MODE AGAIN". */
void p20lemb5_proceed(void)
{
    tc_to(P20LEMB5 + 013u, P20LEMB);
}

void p20lemb5_enter(void)
{
    tc_to(P20LEMB5 + 014u, P20LEMB);
}

/* P20LEMB5 +015 -- PRIOLARM's immediate return. */
void p20lemb5_immediate_return(void)
{
    tc_to(P20LEMB5 + 015u, ENDOFJOB);
}

/* P20LEMB6 -- p. 497. "REQUEST RR AUTO MODE SELECTION": GOPERF1 asks for
 * OCT201's checklist code (verb 50 noun 25, 00201). */
static void p20lemb6(void)
{
    set_A(OCT201);
    tc_to(P20LEMB6 + 1u, BANKCALL); /* CADR GOPERF1 */
}

/* P20LEMB6 +3 -- "TERMINATE EXITS P20 VIA V56 CODING". */
void p20lemb6_terminate(void)
{
    tc_to(P20LEMB6 + 3u, GOTOV56);
}

/* P20LEMB6 +4 -- "PROCEED CHECKS AUTO MODE DISCRETE AGAIN". */
void p20lemb6_proceed(void)
{
    tc_to(P20LEMB6 + 4u, P20LEMB);
}

/* P20LEMB6 +5 -- "ENTER INDICATES MANUAL ACQUISITION (R23)", unless
 * LUNSFCHK finds the LM on the surface. */
void p20lemb6_enter(void)
{
    tc_at(P20LEMB6 + 5u);
    lunsfchk();
}

/* P20LEMB6 +6 -- "YES -- R23 NOT ALLOWED -- TURN ON OPR ERROR". */
void p20lemb6_on_surface(void)
{
    tc_to(P20LEMB6 + 6u, P20LEMB2);
}

/* P20LEMB6 +7 -- "NO -- DO MANUAL ACQUISITION". */
void p20lemb6_in_orbit(void)
{
    tc_at(P20LEMB6 + 7u);
    r23lem();
}

/* P20LEMB1 -- p. 497. "RETURN FROM R23 -- LOCKON ACHIEVED": "SET MANUAL
 * FLAG AND GO BACK TO CHECK RR AUTO MODE". */
void p20lemb1(void)
{
    tc_to(P20LEMB1, UPFLAG); /* ADRES ACMODFLG */
}

void p20lemb1_after_upflag(void)
{
    tc_to(P20LEMB1 + 2u, P20LEMB);
}

/* P20LEMB2 -- "TURNS ON OPERATOR ERROR LIGHT ON DSKY AND GOES BACK TO
 * CHECK AUTO MODE". */
void p20lemb2(void)
{
    tc_to(P20LEMB2, FALTON);
}

void p20lemb2_after_falton(void)
{
    tc_to(P20LEMB2 + 1u, P20LEMB);
}

/* P20LEMB3 -- p. 497. "ARE RR CDUS BEING ZEROED" (RADMODES' RCDU0BIT):
 * "YES -- WAIT". "IS SEARCH OR MANUAL ACQUISITION FLAG SET" (FLAGWRD2's
 * bits 13 and 14): "ZERO MEANS AUTOMATIC RR ACQUISITION", P20LEMC3.
 * Otherwise "RESET TO AUTO MODE". */
void p20lemb3(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(RCDU0BIT);
    if (A_is_zero()) { /* EXTEND; BZF P20LEMB4 */
        continue_at(P20LEMB4);
        return;
    }
    set_A(BIT13_14);
    mask_at(ADDRESS_OF(FLAGWRD2));
    if (A_is_zero()) { /* EXTEND; BZF P20LEMC3 */
        continue_at(P20LEMC3);
        return;
    }
    tc_to(P20LEMB3 + 010u, DOWNFLAG); /* ADRES SRCHOPTN */
}

void p20lemb3_after_srchoptn(void)
{
    tc_to(P20LEMB3 + 012u, DOWNFLAG); /* ADRES ACMODFLG */
}

/* P20LEMB3 +014 -- "WAIT 2.5 SECONDS THEN GO TO RR DATA READ". */
void p20lemb3_after_acmodflg(void)
{
    tc_to(P20LEMB3 + 014u, P20LEMWT);
}

/* P20LEMB4 -- p. 498. "WAIT 2.5 SECONDS WHILE RR CDUS ARE BEING ZEROED --
 * THEN GO BACK AND CHECK AGAIN": DELAYJOB, 250DEC centiseconds. */
void p20lemb4(void)
{
    set_A(DEC_250);
    tc_to(P20LEMB4 + 1u, BANKCALL); /* CADR DELAYJOB */
}

void p20lemb4_after_delayjob(void)
{
    tc_to(P20LEMB4 + 3u, P20LEMB3);
}

/* P20LEMC3 -- p. 498. Automatic acquisition: first the state vectors to
 * now (UPPSV). FINDVAC starts it here too, as a job, from P20LEMD2. */
void p20lemc3(void)
{
    tc_at(P20LEMC3);
    intpret();
}

/* P20LEMC3 +1 */
static const word p20lemc3_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STCALL(TDEC1),
    ADDRESS(UPPSV),
    /* P20LEMC4 */ ORDER(EXIT),
};

/* P20LEMC -- p. 498. Restart group 2 at P20LEM1. */
void p20lemc(void)
{
    tc_to(P20LEMC, PHASCHNG); /* OCT 04022 */
}

/* P20LEMC +2 -- "IS THE RENDEZVOUS FLAG SET": "NO -- EXIT P20". "IS TRACK
 * FLAG SET (BIT 5 FLAGWORD 1)": "TRACK FLAG NOT ON -- WAIT 15 SECONDS"
 * (P20LEMD). P20LEMF: R21, the designate. */
void p20lemc_after_phaschng(void)
{
    ca(ADDRESS_OF(FLAGWRD0));
    mask(RNDVZBIT);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    if (A_is_zero()) { /* EXTEND; BZF P20LEMD */
        continue_at(P20LEMD);
        return;
    }
    tc_to(P20LEMF, R21LEM);
}

/* P20LEMWT -- p. 498. "WAIT 2.5 SECONDS": TWIDDLE ("USE INSTEAD OF
 * WAITLIST SINCE SAME BANK") starts the task P20LEMC1 then. */
void p20lemwt(void)
{
    set_A(DEC_250);
    p20lemwt_plus1();
}

/* P20LEMWT +1 -- R22WAIT comes here with its own delay in A. */
void p20lemwt_plus1(void)
{
    tc_to(P20LEMWT + 1u, TWIDDLE); /* ADRES P20LEMC1 */
}

/* P20LEMWT +3 -- "IS TRACK FLAG SET": "NO -- EXIT WITHOUT DOING 2.7 PHASE
 * CHANGE". */
void p20lemwt_after_twiddle(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    p20lmwt1();
}

/* P20LMWT1 -- group 2's restart (OCT 40072) is now the P20LEMC1 task. */
static void p20lmwt1(void)
{
    tc_to(P20LMWT1, PHASCHNG);
}

void p20lmwt1_after_phaschng(void)
{
    tc_to(P20LMWT1 + 2u, ENDOFJOB);
}

/* P20LEMC1 -- p. 498, a task. "IS RENDEZVOUS FLAG SET": "NO -- EXIT
 * P20/R22". "IS TRACK FLAG SET": "NO -- DON'T SCHEDULE R22 JOB" (P20LEMC2).
 * "YES -- SCHEDULE R22 JOB (RR DATA READ)": FINDVAC, R22LEM42 at priority
 * 26. */
void p20lemc1(void)
{
    ca(ADDRESS_OF(FLAGWRD0));
    mask(RNDVZBIT);
    if (A_is_zero()) { /* EXTEND; BZF TASKOVER */
        continue_at(TASKOVER);
        return;
    }
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    if (A_is_zero()) { /* EXTEND; BZF P20LEMC2 */
        p20lemc2();
        return;
    }
    set_A(PRIO26);
    tc_to(P20LEMC1 + 011u, FINDVAC); /* 2CADR R22LEM42 */
}

void p20lemc1_after_findvac(void)
{
    tc_to(P20LEMC1 + 014u, TASKOVER);
}

/* P20LEMC2 -- p. 499. "TRACK FLAG NOT SET, WAIT 15 SECONDS AND CHECK
 * AGAIN": FIXDELAY, DEC 1500 centiseconds. */
static void p20lemc2(void)
{
    tc_to(P20LEMC2, FIXDELAY); /* DEC 1500 */
}

void p20lemc2_after_fixdelay(void)
{
    tc_to(P20LEMC2 + 2u, P20LEMC1);
}

/* P20LEMD -- p. 499. "WAITLIST FOR 15 SECONDS": the task P20LEMD1. */
void p20lemd(void)
{
    set_A(DEC_1500);
    tc_to(P20LEMD + 1u, TWIDDLE); /* ADRES P20LEMD1 */
}

void p20lemd_after_twiddle(void)
{
    tc_to(P20LEMD + 3u, ENDOFJOB);
}

/* P20LEMD1 -- p. 499, a task. "IS TRACK FLAG SET": "YES -- SCHEDULE
 * DESIGNATE JOB" (P20LEMD2), "NO -- WAIT 15 SECONDS". (A masked flag word
 * is never negative: not modelled.) */
void p20lemd1(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(TRACKBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: continue_at(P20LEMD2); return;              /* TCF P20LEMD2 */
    case IS_PLUS_ZERO: tc_to(P20LEMD1 + 4u, FIXDELAY); return; /* DEC 1500 */
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

void p20lemd1_after_fixdelay(void)
{
    tc_to(P20LEMD1 + 6u, P20LEMD1);
}

/* P20LEMD2 -- "SCHEDULE JOB TO DO R21": FINDVAC, P20LEMC3 at priority 26,
 * "START AT PERM. MEMORY INTEGRATION". */
void p20lemd2(void)
{
    set_A(PRIO26);
    tc_to(P20LEMD2 + 1u, FINDVAC); /* 2CADR P20LEMC3 */
}

void p20lemd2_after_findvac(void)
{
    tc_to(P20LEMD2 + 4u, TASKOVER);
}

/* UPPSV -- pp. 499-501. "UPDATES PERMANENT STATE VECTORS TO PRESENT
 * TIME", TDEC1: the vehicle P20 updates (VEHUPFLG: the CSM's if set, else
 * the LM's) with its W matrix, "IF W-MATRIX INVALID, DON'T INTEGRATE IT",
 * then the other one "WITHOUT W-MATRIX" (UPPSV4). "IF ON LUNAR SURFACE W
 * IS 6X6 ... OTHERWISE 9X9". On the surface the LM's state is its landing
 * site's: P20LEMC4. Otherwise LMPOS and LMVEL get the LM's new state,
 * scaled down by 4 (VSR2). LS21X has UPPSV's return. */
static const word uppsv_words[] = {
    /* UPPSV */ ORDERS(STQ, CALL),
    ADDRESS(LS21X),
    ADDRESS(INTSTALL),
    ORDER(CALL),
    ADDRESS(SETIFLGS),
    ORDERS(BOF, SET), /* "IF W-MATRIX INVALID, DON'T INTEGRATE IT" */
    SWITCH(BOF, RENDWFLG),
    ADDRESS(UPPSV1),
    SWITCH(SET, DIM0FLAG), /* "SET DIM0FLAG TO INTEGRATE W-MATRIX" */
    ORDERS(BON, SET),
    SWITCH(BON, SURFFLAG), /* "IF ON LUNAR SURFACE W IS 6X6" */
    ADDRESS(UPPSV5),
    SWITCH(SET, D6OR9FLG), /* "OTHERWISE 9X9" */
    /* UPPSV5 */ ORDER(BOF),
    SWITCH(BOF, VEHUPFLG),
    ADDRESS(UPPSV3),
    /* UPPSV1 */ ORDER(SET),
    SWITCH(SET, VINTFLAG),
    ORDER(CALL),
    ADDRESS(INTEGRV),
    ORDER(CALL),     /* "GROUP 2 PHASE CHANGE */
    ADDRESS(GRP2PC), /* TO PROTECT INTEGRATION" */
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(DLOAD, CLEAR), /* "GET TETCSM TO STORE IN TDEC FOR LM INT." */
    ADDRESS_PLUS_ONE(TETCSM),
    SWITCH(CLEAR, VINTFLAG),
    /* UPPSV4 */ ORDER(CALL), /* "INTEGRATE OTHER VEHICLE */
    ADDRESS(SETIFLGS),        /* WITHOUT W-MATRIX" */
    STCALL(TDEC1),
    ADDRESS(INTEGRV),
    ORDERS(BOFF, VLOAD),
    SWITCH(BOF, SURFFLAG),
    ADDRESS(P20LEMC4),
    ADDRESS_PLUS_ONE(RCVLEM),
    ORDER(VSR2),
    STOVL(LMPOS),
    ADDRESS_PLUS_ONE(VCVLEM),
    ORDER(VSR2),
    STORE(LMVEL),
    ORDER(GOTO),
    ADDRESS(LS21X),
    /* UPPSV3 */ ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, VINTFLAG),
    ADDRESS(INTEGRV),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(SET, DLOAD),
    SWITCH(SET, VINTFLAG),
    ADDRESS_PLUS_ONE(TETLEM), /* "GET TETLEM TO STORE IN TDEC FOR CSM INT." */
    ORDER(GOTO),
    ADDRESS(UPPSV4),
};

/* ---- P25 ---- */

/* PROG25 -- p. 502. "ASTRONAUT REQUEST THROUGH DSKY V37E25E". 2PHSCHNG:
 * "MAKE GROUP 4 INACTIVE (VERB 37)" and group 2 at "PRIORITY 26". */
void prog25(void)
{
    tc_to(PROG25, TWO_PHSCHNG);
}

/* PROG25 +4 -- "IMU STATUS CHECK". */
void prog25_after_2phschng(void)
{
    tc_to(PROG25 + 4u, BANKCALL); /* CADR R02BOTH */
}

void prog25_after_r02both(void)
{
    tc_to(PROG25 + 6u, UPFLAG); /* ADRES TRACKFLG: "SET TRACK FLAG" */
}

void prog25_after_trackflg(void)
{
    tc_to(PROG25 + 010u, UPFLAG); /* ADRES P25FLAG: "SET P25FLAG" */
}

/* P25LEM1 -- p. 502. Restart group 2 here. FINDVAC starts it here too, as
 * a job, from P25LEM2. */
void p25lem1(void)
{
    tc_to(P25LEM1, PHASCHNG); /* OCT 04022 */
}

/* P25LEM1 +2 -- "IS P25FLAG SET": no, the program has ended. "IS
 * TRACKFLAG SET?": "NO -- SKIP PHASE CHANGE AND WAIT 1 MINUTE". "CALL R65
 * -- FINE PREFERRED TRACKING ATTITUDE ROUTINE", with R65CNTR 7. (CAF SEVEN
 * leaves no overflow for the TS to skip on.) */
void p25lem1_after_phaschng(void)
{
    set_A(P25FLBIT);
    mask_at(ADDRESS_OF(STATE));
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (A_is_zero()) { /* EXTEND; BZF P25LMWT1 */
        continue_at(P25LMWT1);
        return;
    }
    set_A(SEVEN);
    if (ts(ADDRESS_OF(R65CNTR))) not_modelled();
    tc_to(P25LEM1 + 014u, BANKCALL); /* CADR R65LEM */
}

/* P25LEM1 +016 -- "THEN GO CHECK FLAGS". */
void p25lem1_after_r65lem(void)
{
    tc_to(P25LEM1 + 016u, P25LEM1);
}

/* P25LEMWT -- p. 503. Group 2's restart (OCT 00112) is now the wait. */
void p25lemwt(void)
{
    tc_to(P25LEMWT, PHASCHNG);
}

/* P25LMWT1 -- "WAIT ONE MINUTE THEN CHECK AGAIN": the task P25LEM2, in
 * 60SCNDS. */
void p25lmwt1(void)
{
    set_A(SIXTY_SCNDS);
    tc_to(P25LMWT1 + 1u, TWIDDLE); /* ADRES P25LEM2 */
}

void p25lmwt1_after_twiddle(void)
{
    tc_to(P25LMWT1 + 3u, ENDOFJOB);
}

/* P25LEM2 -- p. 503, a task: FINDVAC, P25LEM1 at priority 14. */
void p25lem2(void)
{
    set_A(PRIO14);
    tc_to(P25LEM2 + 1u, FINDVAC); /* 2CADR P25LEM1 */
}

void p25lem2_after_findvac(void)
{
    tc_to(P25LEM2 + 4u, TASKOVER);
}

/* ---- The interpretive code ---- */

const struct interpretive_code ORBCHG2_CODE = {024u, ORBCHG2 + 1u, COUNT(orbchg2_words),
                                               orbchg2_words};
const struct interpretive_code P20LEM1_CODE = {024u, P20LEM1 + 5u, COUNT(p20lem1_words),
                                               p20lem1_words};
const struct interpretive_code P20LEMC3_CODE = {024u, P20LEMC3 + 1u, COUNT(p20lemc3_words),
                                                p20lemc3_words};
const struct interpretive_code UPPSV_CODE = {024u, UPPSV, COUNT(uppsv_words), uppsv_words};
const struct interpretive_code ORBCHG3_CODE = {032u, ORBCHG3, COUNT(orbchg3_words), orbchg3_words};

#endif
