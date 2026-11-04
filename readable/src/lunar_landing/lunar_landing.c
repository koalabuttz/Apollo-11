/* SPDX-License-Identifier: MIT */
/* THE_LUNAR_LANDING.agc, pp. 785-792 (Luminary099 only): P63LM, the
 * ignition algorithm (IGNALG, DDUMCALC), the countdown's start and the
 * burn attitude (ASTNCLOK, ASTNRET, P63SPOT2 .. P63SPOT4), and P68's
 * LANDJUNK. The faithful C is c64-port/src/lm_lunar_landing.c. */
#include "lunar_landing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define P63_BANK     032u /* P63LM .. P63SPOT4 */
#define P68_BANK     031u /* LANDJUNK */

/* TC at, to a routine out of the file or to INTPRET: Q gets the return
 * and the routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- P63: THE LUNAR LANDING, BRAKING PHASE -- p. 785 ---- */

/* P63LM: group 4 restarts here (TC PHASCHNG, OCT 04024), */
void p63lm(void)
{
    tc_to(P63LM, PHASCHNG);
}

/* and the IMU status check, R02. */
void p63lm_after_phaschng(void)
{
    tc_to(P63LM + 2u, BANKCALL); /* CADR R02BOTH */
}

/* BURNBABY's table for P63 (WHICH), the delta V monitor's threshold and
 * count (DVTHRUSH, DVCNTR), WCHPHASE -1 and FLPASS0 +0 for the guidance;
 * the track-enable discrete (channel 12 bit 14) off; and to the
 * interpreter, for IGNALG_CODE. */
void p63lm_after_r02both(void)
{
    set_A(P63ADRES); /* "INITIALIZE WHICH FOR BURNBABY" */
    ts(ADDRESS_OF(WHICH));
    set_A(DPSTHRSH); /* "INITIALIZE DVMON" */
    ts(ADDRESS_OF(DVTHRUSH));
    set_A(FOUR);
    ts(ADDRESS_OF(DVCNTR));
    set_A(negate(ONE)); /* "INITIALIZE WCHPHASE AND FLPASS0" */
    ts(ADDRESS_OF(WCHPHASE));
    set_A(ZERO);
    ts(ADDRESS_OF(FLPASS0));
    set_A(negate(BIT14));
    wand_channel(CHAN12); /* "REMOVE TRACK-ENABLE DISCRETE." */
    tc_to(FLAGORGY, INTPRET);
}

/* FLAGORGY, the "DIONYSIAN FLAG WAVING": NOTHROTL, REDFLAG and LRBYPASS
 * clear, MUNFLAG set, and P25FLAG and RNDVZFLG clear, which stops P25 and
 * P20. Then IGNALG: RP-TO-R turns the landing site (RLS, moon-fixed) at
 * the landing time TLAND into the reference frame, and GUIDINIT starts the
 * guidance from it; the first guess at ignition is TLAND less GUIDDURN,
 * where LEMPREC integrates the LM's state; then NIGNLOOP, CG, DELTAH,
 * UNFC/2 and TTF/8 get their first values. IGNALOOP takes LEMPREC's state
 * (TAT, RATT1) into R and its gravity (MUNGRAV), and ?GUIDSUB runs the
 * guidance there, which returns to DDUMCALC. */
static const word ignalg_words[] = {
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, NOTHROTL),
    SWITCH(CLEAR, REDFLAG),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, LRBYPASS),
    SWITCH(SET, MUNFLAG),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, P25FLAG),  /* "TERMINATE P25 IF IT IS RUNNING." */
    SWITCH(CLEAR, RNDVZFLG), /* "TERMINATE P20 IF IT IS RUNNING" */
    /* IGNALG */ ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RLS), /* "AT 0D LANDING SITE IN MOON FIXED FRAME" */
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(TLAND), /* "AT 6D ESTIMATED TIME OF LANDING" */
    STCALL(TPIP),            /* "ALSO SET TPIP FOR FIRST GUIDANCE PASS" */
    ADDRESS(RP_TO_R),
    ORDERS(VSL4, MXV),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STCALL(LAND),
    ADDRESS(GUIDINIT), /* "GUIDINIT INITIALIZES WM AND /LAND/" */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TLAND),
    ADDRESS_PLUS_ONE(GUIDDURN),
    STCALL(TDEC1), /* "INTEGRATE STATE FORWARD TO THAT TIME" */
    ADDRESS(LEMPREC),
    ORDERS(SSP, VLOAD),
    ADDRESS_PLUS_ONE(NIGNLOOP),
    NUMBER(40),
    ADDRESS_PLUS_ONE(UNITX),
    STOVL(CG),
    ADDRESS_PLUS_ONE(UNITY),
    STOVL_AT(CG, 6u),
    ADDRESS_PLUS_ONE(UNITZ),
    STODL_AT(CG, 014u),
    ADDRESS_PLUS_ONE(99999CON),
    STOVL(DELTAH), /* "INITIALIZE DELTAH FOR V16N68 DISPLAY" */
    ADDRESS_PLUS_ONE(ZEROVECS),
    STODL(UNFC_2), /* "INITIALIZE TRIM VELOCITY CORRECTION TERM" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STORE(TTF_8),
    /* IGNALOOP */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TAT),
    STOVL(PIPTIME1),
    ADDRESS_PLUS_ONE(RATT1),
    ORDERS(VSL4, MXV),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STCALL(R),
    ADDRESS(MUNGRAV),
    STCALL(GDT_2),
    ADDRESS(_QGUIDSUB), /* ?GUIDSUB: "WHICH DELIVERS N PASSES OF GUIDANCE" */
};

/* ---- DDUMCALC -- pp. 786-788 ---- */

/* DDUMCALC: ?GUIDSUB's pass count (NIGNLOOP), and to the interpreter for
 * DDUMCALC_CODE. Not modelled: overflow in A, where TS NIGNLOOP would skip
 * into the order words (no caller leaves it). */
void ddumcalc(void)
{
    if (ts(ADDRESS_OF(NIGNLOOP))) not_modelled();
    tc_to(DDUMCALC + 1u, INTPRET);
}

/* DDUM, how far to move the guess at ignition, from the guidance's
 * results (RGU and VGU, the position and velocity in guidance
 * coordinates) against the wanted ignition point (RIGNX, RIGNZ, VIGN) and
 * its gains (KIGNX/B4, KIGNY/B8, KIGNV/B4). The quotient is in units of
 * 2(18) centiseconds and SRR 10D rescales it; the new guess is DDUM past
 * PIPTIME1 (TDEC1). While |DDUM| is DDUMCRIT or more, INTSTALL and
 * INTEGRVS integrate the LM's state to the new guess, and GOTO IGNALOOP.
 * Then DDUMGOOD: TIG is TDEC1 less the zoom time (ZOOMTIME, at 2(14) to
 * 2(28)); OUTOFPLN the landing site's distance out of the orbit's plane
 * ("SIGN IS + IF LANDING SITE IS TO THE" right); and R60INIT keeps UNFC/2
 * in R60VSAVE for the burn attitude. */
static const word ddumcalc_words[] = {
    ORDERS(DLOAD, DMPR), /* "FORM DENOMINATOR FIRST" */
    ADDRESS_PLUS_ONE(VGU),
    ADDRESS_PLUS_ONE(KIGNX_B4),
    ORDERS(SL4R, BDSU),
    ADDRESS_PLUS_ONE_AT(VGU, 4u),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE(RIGNZ),
    ADDRESS_PLUS_ONE_AT(RGU, 4u),
    ORDERS(SR4R, PDDL),
    ADDRESS_PLUS_ONE_AT(RGU, 2u),
    ORDERS(DSQ, DMPR),
    ADDRESS_PLUS_ONE(KIGNY_B8),
    ORDERS(SL4R, PDDL),
    ADDRESS_PLUS_ONE(RGU),
    ORDERS(DSU, DMPR),
    ADDRESS_PLUS_ONE(RIGNX),
    ADDRESS_PLUS_ONE(KIGNX_B4),
    ORDERS(PDVL, ABVAL),
    ADDRESS_PLUS_ONE(VGU),
    ORDERS(DSU, DMPR),
    ADDRESS_PLUS_ONE(VIGN),
    ADDRESS_PLUS_ONE(KIGNV_B4),
    ORDERS(DAD, DAD), /* (the three terms off the push-down list) */
    ORDERS(DAD, DDV), /* (and the denominator) */
    ORDER(SRR),
    SRR_COUNT(10),
    ORDERS(PUSH, DAD),
    ADDRESS_PLUS_ONE(PIPTIME1),
    STODL(TDEC1), /* "STORE NEW GUESS FOR NEXT INTEGRATION" */
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(DDUMCRIT),
    ORDERS(BMN, CALL),
    ADDRESS(DDUMGOOD),
    ADDRESS(INTSTALL),
    ORDERS(SET, SET),
    SWITCH(SET, INTYPFLG),
    SWITCH(SET, MOONFLAG),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(PIPTIME1),
    STOVL(TET),              /* "HOPEFULLY ?GUIDSUB DID NOT" */
    ADDRESS_PLUS_ONE(RATT1), /* "CLOBBER RATT1 AND VATT1" */
    STOVL(RCV),
    ADDRESS_PLUS_ONE(VATT1),
    STCALL(VCV),
    ADDRESS(INTEGRVS),
    ORDER(GOTO),
    ADDRESS(IGNALOOP),
    /* DDUMGOOD */ ORDERS(SLOAD, SR),
    ADDRESS_PLUS_ONE(ZOOMTIME),
    SR_COUNT(14),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(TDEC1),
    STOVL(TIG), /* "COMPUTE DISTANCE LANDING SITE WILL BE" */
    ADDRESS_PLUS_ONE(V),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(R),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(LAND),
    /* R60INIT */ STOVL(OUTOFPLN), /* "INITIALIZATION FOR CALCMANU" */
    ADDRESS_PLUS_ONE(UNFC_2),
    STORE(R60VSAVE), /* "STORE UNFC/2 TEMPORARILY IN R60SAVE" */
    ORDER(EXIT),
};

/* IGNALGRT: group 4's restart point moves on (TC PHASCHNG, OCT 04024),
 * "PREVENT REPEATING IGNALG", */
void ignalgrt(void)
{
    tc_to(IGNALGRT, PHASCHNG);
}

/* ASTNCLOK: and the countdown clock starts (STCLOK2 with -ASTNDEX); */
void astnclok(void)
{
    set_A(negate(ASTNDEX));
    tc_to(ASTNCLOK + 1u, BANKCALL); /* CADR STCLOK2 */
}

/* this job ends: the clock goes on in a job of its own, at ASTNRET
 * ("RETURN IN NEW JOB AND IN EBANK FIVE"). */
void astnclok_after_stclok2(void)
{
    continue_at(ENDOFJOB);
}

/* ---- ASTNRET and the burn attitude -- pp. 788-789 ---- */

/* ASTNRET: to the interpreter, for P63SPOT2_CODE. */
void astnret(void)
{
    tc_to(ASTNRET, INTPRET);
}

/* R51's fine alignment, with P63SPOT2 as its return (QMAJ): PROCEED does
 * the alignment, ENTER goes on at P63SPOT2. P63SPOT2: "INITIALIZE KALCMANU
 * FOR BURN ATTITUDE", the X axis (SCAXIS) along the unit UNFC/2 kept in
 * R60VSAVE (POINTVSM). */
static const word p63spot2_words[] = {
    ORDERS(SSP, RTB),
    ADDRESS_PLUS_ONE(QMAJ),
    ADDRESS(P63SPOT2), /* FCADR P63SPOT2 */
    ADDRESS(R51P63),
    /* P63SPOT2 */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R60VSAVE),
    STOVL(POINTVSM),
    ADDRESS_PLUS_ONE(UNITX),
    STORE(SCAXIS),
    ORDER(EXIT),
};

/* After the EXIT: EBANK 7, and PFLITEDB with interrupts off, */
void p63spot2_after_exit(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    inhint();
    tc_to(P63SPOT2 + 011u, IBNKCALL); /* CADR PFLITEDB */
}

/* the maneuver to the burn attitude (R60LEM), */
void p63spot2_after_pflitedb(void)
{
    relint();
    tc_to(P63SPOT2 + 014u, BANKCALL); /* CADR R60LEM */
}

/* and group 4's restart point moves on: "PREVENT RECALLING R60". */
void p63spot2_after_r60lem(void)
{
    tc_to(P63SPOT2 + 016u, PHASCHNG);
}

/* P63SPOT3: "IS THE LR ANTENNA IN POSITION 1 YET" (channel 33 bit 6
 * clear). If not, the crew is asked to turn it (GOPERF1 with CODE500). */
void p63spot3(void)
{
    set_A(BIT6);
    rand_channel(CHAN33);
    if (A_is_zero()) { /* "BRANCH IF ANTENNA ALREADY IN POSITION 1" */
        p63spot4();
        return;
    }
    set_A(CODE500);
    tc_to(P63SPOT3 + 6u, BANKCALL); /* CADR GOPERF1 */
}

void p63spot3_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* PROCEED: "SEE IF HE'S LYING". */
void p63spot3_proceed(void)
{
    p63spot3();
}

/* P63SPOT4 (and GOPERF1's ENTER): the landing radar's position 1
 * (SETPOS1), */
void p63spot4(void)
{
    tc_to(P63SPOT4, BANKCALL); /* CADR SETPOS1 */
}

/* and BURNBABY: "OFF TO SEE THE WIZARD..." */
void p63spot4_after_setpos1(void)
{
    tc_to(P63SPOT4 + 2u, POSTJUMP); /* CADR BURNBABY */
}

/* ---- P68: LANDING CONFIRMATION -- pp. 791-792 ---- */

/* LANDJUNK: group 4 restarts here (TC PHASCHNG, OCT 04024), */
void landjunk(void)
{
    tc_to(LANDJUNK, PHASCHNG);
}

/* the attitude error zeroed (ZATTEROR, with interrupts off), */
void landjunk_after_phaschng(void)
{
    inhint();
    tc_to(LANDJUNK + 3u, BANKCALL); /* CADR ZATTEROR */
}

/* the 5 degree deadband (SETMAXDB), */
void landjunk_after_zatteror(void)
{
    tc_to(LANDJUNK + 5u, BANKCALL); /* CADR SETMAXDB */
}

/* and to the interpreter for LANDJUNK_CODE, "AS TIME IS NOT CRITICAL". */
void landjunk_after_setmaxdb(void)
{
    tc_to(LANDJUNK + 7u, INTPRET);
}

/* On the surface: SURFFLAG and APSFLAG set, LETABORT clear; RN (in
 * ALPHAV) and PIPTIME for LAT-LONG, with LUNAFLAG set; and "COMPUTE RLS
 * AND STORE IT AWAY": RN at PIPTIME in moon-fixed coordinates
 * (R-TO-RP). */
static const word landjunk_words[] = {
    ORDERS(SET, CLEAR),
    SWITCH(SET, SURFFLAG),
    SWITCH(CLEAR, LETABORT),
    ORDERS(SET, VLOAD),
    SWITCH(SET, APSFLAG),
    ADDRESS_PLUS_ONE(RN),
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE(PIPTIME),
    ORDERS(SET, CALL),
    SWITCH(SET, LUNAFLAG),
    ADDRESS(LAT_LONG),
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RN),
    ORDERS(VSL2, PDDL),
    ADDRESS_PLUS_ONE(PIPTIME),
    ORDERS(PUSH, CALL),
    ADDRESS(R_TO_RP),
    STORE(RLS),
    ORDER(EXIT),
};

/* After the EXIT: V06N43*, "NOW LOOK WHERE YOU ENDED UP" (GOFLASH). */
void landjunk_after_exit(void)
{
    set_A(V06N43_STAR);
    tc_to(LANDJUNK + 035u, BANKCALL); /* CADR GOFLASH */
}

void landjunk_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* PROCEED (TCF +2): to the interpreter for GSAV_CODE. */
void landjunk_proceed(void)
{
    tc_to(LANDJUNK + 042u, INTPRET);
}

/* RECYCLE (TCF -5): the display again. */
void landjunk_recycle(void)
{
    landjunk_after_exit();
}

/* "INITIALIZE GSAV AND (USING REFMF)" the rest of P57's start. */
static const word gsav_words[] = {
    ORDER(VLOAD), ADDRESS_PLUS_ONE(UNITX), STCALL(GSAV), ADDRESS(REFMF), ORDER(EXIT),
};

/* After the EXIT: "ASTRONAUT:  PLEASE SELECT P57". */
void landjunk_after_refmf_exit(void)
{
    continue_at(GOTOPOOH);
}

const struct interpretive_code IGNALG_CODE = {P63_BANK, FLAGORGY + 1u, COUNT(ignalg_words),
                                              ignalg_words};
const struct interpretive_code DDUMCALC_CODE = {P63_BANK, DDUMCALC + 2u, COUNT(ddumcalc_words),
                                                ddumcalc_words};
const struct interpretive_code P63SPOT2_CODE = {P63_BANK, ASTNRET + 1u, COUNT(p63spot2_words),
                                                p63spot2_words};
const struct interpretive_code LANDJUNK_CODE = {P68_BANK, LANDJUNK + 010u, COUNT(landjunk_words),
                                                landjunk_words};
const struct interpretive_code GSAV_CODE = {P68_BANK, LANDJUNK + 043u, COUNT(gsav_words),
                                            gsav_words};

#endif
