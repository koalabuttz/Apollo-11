/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 7: the tracking and state vector verbs' basic
 * code -- LM pp. 262, 287-288, 296-300, CM pp. 236, 259-265. The faithful
 * C is c64-port/src/lanec_ev_track.c.
 *
 *   - V69, "FORCE A HARDWARE RESTART" (VERB69): a TC to itself, which the
 *     hardware's TC trap ends with a restart.
 *   - V56, terminate tracking: the LM's TRMTRAK1 (bank 42), the CM's
 *     TRACKTRM and CLEANOUT. The flags down, rates stopped, integration let
 *     finish (INTSTALL), the tracking phases killed, and a restart.
 *   - V66, "VEHICLES ARE ATTACHED" (ATTACHIT, a job PART 2's ATTACHED
 *     starts): this vehicle's state vector copied into the other's
 *     (GENTRAN), then both brought up to date and the job back to PINBALL
 *     (TACHEXIT in the CM, which the CM's V47, LMTOCMSV and LMTOCM, the
 *     other way round, shares).
 *   - The LM's V85 (DSPRRLOS, bank 40): the rendezvous radar's line of
 *     sight, in body axes, shown once a second (RRLOSDSP).
 *   - The CM's V87 and V88 (SETVHFLG, RESETVHF): the VHF range flag.
 *
 * Much of the work is interpretive. A TC INTPRET hands the words after it
 * to the interpreter: they are tables at the end of this file (the LM's
 * part 8, the CM's part 9), in the listing's notation, which the readable
 * interpreter runs. Their EXIT comes back to basic code at the next word,
 * an entry here.
 * Each call out of the file ends a C function with Q and Z set (tc()
 * below, or continue_at()), and the word it returns to is an entry of its
 * own; the binding goes on from one entry of this part to the next while Z
 * is one. A routine that falls into the next label calls it. */
#include "extended_verbs.h"
#include "../agc/names.h"

/* TC at fixed address at to routine: Q gets the return, at +1. */
static void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* VERB69 -- LM p. 262, CM p. 236.
 * Verb 69, "FORCE A HARDWARE RESTART": TC VERB69, to itself. The
 * computer's TC trap restarts it; the binding runs the word once. */
void verb69(void)
{
    tc(VERB69, VERB69);
}

/* After ATTACHIT's (and LMTOCM's) EXIT: GENTRAN copies the 42 words
 * (OCT51 is the count less one) from the first ADRES after the call to the
 * second: "OUR STATE VECTOR INTO OTHER". */
static void gentran_from(word at)
{
    set_A(OCT51);
    tc(at + 1u, GENTRAN); /* ADRES RRECTHIS; ADRES RRECTOTH */
}

/* TCPIN's address into the job's QPRET (INDEX FIXLOC: QPRET of its VAC
 * area), then POSTJUMP to INTWAKE: "FREE INTEGRATION AND EXIT", out
 * through TCPIN (RTB PINBRNCH) back to PINBALL. FIXLOC is a VAC area's
 * address: the TS is a plain store into erasable memory, with none of a
 * register's side effects even where E-bank 0 makes the address an alias
 * of one. A FIXLOC that takes it out of erasable memory the routine does
 * not model. */
static void back_to_pinball(word at)
{
    unsigned qpret = ADDRESS_OF(QPRET) + fetch(ADDRESS_OF(FIXLOC));

    if (qpret >= 02000u) not_modelled();
    set_A(TCPINAD);
    ERASABLE(qpret) = TCPINAD; /* INDEX FIXLOC; TS QPRET */
    tc(at + 3u, POSTJUMP);     /* CADR INTWAKE */
}

#if LUMINARY099
/* ---- V56 (bank 42) ---- */

/* TRMTRAK1 -- LM p. 287, where TRMTRACK's POSTJUMP goes: "UPDATE FLAG
 * DOWN" ... */
void trmtrak1(void)
{
    tc(TRMTRAK1, DOWNFLAG); /* ADRES UPDATFLG */
}

/* TRMTRAK1 +2 -- LM p. 287: "TRACK FLAG DOWN" ... */
void trmtrak1_after_downflag(void)
{
    tc(TRMTRAK1 + 2u, DOWNFLAG); /* ADRES TRACKFLG */
}

/* TRMTRAK1 +4 -- LM p. 287: ... and IMUSE. */
void trmtrak1_after_second_downflag(void)
{
    tc(TRMTRAK1 + 4u, DOWNFLAG); /* ADRES IMUSE */
}

/* TRMTRAK1 +6 -- LM pp. 287-288: "DON'T INTERRUPT INTEGRATION", in
 * TRMTRAK1_CODE (below). */
void trmtrak1_after_third_downflag(void)
{
    tc(TRMTRAK1 + 6u, INTPRET);
}

/* TRMTRAK1 +10 -- LM p. 288: "KILL GROUP 2 TO HALT P20 ACTIVITY". */
void trmtrak1_after_exit(void)
{
    tc(TRMTRAK1 + 10u, PHASCHNG); /* OCT 2 */
}

/* TRMTRAK1 +12 -- LM p. 288: with interrupts held, "ZERO THE COMMANDED
 * RATES TO STOP MANEUVER". */
void trmtrak1_after_phaschng(void)
{
    inhint();
    tc(TRMTRAK1 + 13u, IBNKCALL); /* CADR STOPRATE */
}

/* TRMTRAK1 +15 -- LM p. 288: the DAP's deadband back. */
void trmtrak1_after_ibnkcall(void)
{
    tc(TRMTRAK1 + 15u, IBNKCALL); /* CADR RESTORDB */
}

/* TRMTRAK1 +17 -- LM p. 288: "CLEAR BITS 10 + 15 OF RADMODES". */
void trmtrak1_after_second_ibnkcall(void)
{
    tc(TRMTRAK1 + 17u, CLRADMOD);
}

/* TRMTRAK1 +18 -- LM p. 288: "DISABLE LOCKON" (channel 12's bit 14), and
 * GOPROG2: "CAUSE RESTART". */
void trmtrak1_after_clradmod(void)
{
    set_A(negate(BIT14)); /* CS BIT14 */
    wand_channel(012);
    tc(TRMTRAK1 + 21u, POSTJUMP); /* CADR GOPROG2 */
}
#else
/* ---- V56 (bank 43) ---- */

/* TRACKTRM -- CM p. 259.
 * Verb 56, terminate tracking (P20): with the rendezvous flag up ("IS
 * REND FLAG ON"), down with it; else back to PINBALL. */
void tracktrm(void)
{
    set_A(RNDVZBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (A_is_zero()) { /* EXTEND; BZF GOPIN: "NO" */
        continue_at(GOPIN);
        return;
    }
    tc(TRACKTRM + 4u, DOWNFLAG); /* ADRES RNDVZFLG */
}

/* TRACKTRM +6 -- CM p. 259: "IS TRACK FLAG ON": down with it. */
void tracktrm_after_downflag(void)
{
    set_A(TRACKBIT);
    mask_at(ADDRESS_OF(FLAGWRD1));
    if (A_is_zero()) { /* EXTEND; BZF GOPIN: "NO" */
        continue_at(GOPIN);
        return;
    }
    tc(TRACKTRM + 10u, DOWNFLAG); /* ADRES TRACKFLG */
}

/* TRACKTRM +12 and +14 -- CM p. 259: UPDATFLG and IMUSE down. */
void tracktrm_after_second_downflag(void)
{
    tc(TRACKTRM + 12u, DOWNFLAG); /* ADRES UPDATFLG */
}

void tracktrm_after_third_downflag(void)
{
    tc(TRACKTRM + 14u, DOWNFLAG); /* ADRES IMUSE */
}

/* TRACKTRM +16 -- CM p. 259: E-bank 6, and with interrupts held the
 * commanded rates stopped. */
void tracktrm_after_fourth_downflag(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    inhint();
    tc(TRACKTRM + 19u, STOPRATE);
}

/* TRACKTRM +20 -- CM p. 259: OPTIND -1, and "DONT INTERRUPT INTEGRATION"
 * (TRACKTRM_CODE, below). */
void tracktrm_after_stoprate(void)
{
    set_A(NEGONE);
    ts(ADDRESS_OF(OPTIND));
    tc(TRACKTRM + 22u, INTPRET);
}

/* TRACKTRM +26 -- CM pp. 259-260: "KILL GROUP 2 TO HALT P20 ACTIVITY",
 * "ALSO KILL GROUP 1"; 2PHSCHNG returns to CLEANOUT. */
void tracktrm_after_exit(void)
{
    tc(TRACKTRM + 26u, TWO_PHSCHNG); /* OCT 2; OCT 1 */
}

/* CLEANOUT -- CM p. 260: ENEMA, "CAUSE RESTART". */
void cleanout(void)
{
    inhint();
    tc(CLEANOUT + 1u, POSTJUMP); /* CADR ENEMA */
}

/* ---- V87 and V88 (bank 43) ---- */

/* SETVHFLG -- CM p. 264.
 * Verb 87, "SET VHF RANGE FLAG - ALLOWS R22 TO ACCEPT RANGE DATA"
 * (SETVHFLG_CODE, below). */
void setvhflg(void)
{
    tc(SETVHFLG, INTPRET);
}

/* SETVHFLG +3 -- CM p. 264. */
void setvhflg_after_exit(void)
{
    tc(SETVHFLG + 3u, GOPIN);
}

/* RESETVHF -- CM p. 264.
 * Verb 88, "RESET VHF RANGE FLAG - STOPS ACCEPTANCE OF RANGE DATA"
 * (RESETVHF_CODE, below). */
void resetvhf(void)
{
    tc(RESETVHF, INTPRET);
}

/* RESETVHF +3 -- CM p. 264: the "TRACKER FAIL LIGHT" off. */
void resetvhf_after_exit(void)
{
    tc(RESETVHF + 3u, TRFAILOF);
}

/* RESETVHF +4 -- CM p. 264. */
void resetvhf_after_trfailof(void)
{
    tc(RESETVHF + 4u, GOPIN);
}
#endif

/* ---- V66 (bank 43) ---- */

/* ATTACHIT -- LM p. 297, CM p. 264.
 * Verb 66's job (ATTACHED starts it): integration let finish, and the
 * other vehicle's sphere of influence made this one's (ATTACHIT_CODE,
 * below). */
void attachit(void)
{
    tc(ATTACHIT, INTPRET);
}

/* ATTACHIT +10 -- LM p. 297, CM p. 264: "OUR STATE VECTOR INTO OTHER VIA
 * GENTRAN". */
void attachit_after_exit(void)
{
    gentran_from(ATTACHIT + 10u);
}

#if LUMINARY099
/* ATTACHIT +14 -- LM p. 297: "UPDATE R-OTHER, V-OTHER", in
 * ATTACHIT_CODE_2 (below). */
void attachit_after_gentran(void)
{
    relint();
    tc(ATTACHIT + 15u, INTPRET);
}

/* ATTACHIT +22 -- LM p. 297. */
void attachit_after_second_exit(void)
{
    back_to_pinball(ATTACHIT + 22u);
}
#else
/* TACHEXIT -- CM pp. 264-265, where ATTACHIT's GENTRAN returns and
 * LMTOCM ends: "UPDATE RN, VN, R-OTHER, V-OTHER" (TACHEXIT_CODE,
 * below). */
void tachexit(void)
{
    relint();
    tc(TACHEXIT + 1u, INTPRET);
}

/* TACHEXIT +10 -- CM p. 265. */
void tachexit_after_exit(void)
{
    back_to_pinball(TACHEXIT + 10u);
}

/* ---- V47 (bank 43) ---- */

/* LMTOCMSV -- CM p. 265.
 * Verb 47, "MOVE LM STATE VECTOR INTO CSM STATE VECTOR": the job LMTOCM at
 * priority 10. */
void lmtocmsv(void)
{
    set_A(PRIO10);
    tc(LMTOCMSV + 1u, FINDVAC); /* 2CADR LMTOCM */
}

/* LMTOCMSV +4 -- CM p. 265. */
void lmtocmsv_after_findvac(void)
{
    tc(LMTOCMSV + 4u, ENDOFJOB);
}

/* LMTOCM -- CM p. 265: as ATTACHIT, the other way round (LMTOCM_CODE,
 * below). */
void lmtocm(void)
{
    tc(LMTOCM, INTPRET);
}

/* LMTOCM +10 -- CM p. 265: "LM STATE VECTOR INTO CM VIA GENTRAN". */
void lmtocm_after_exit(void)
{
    gentran_from(LMTOCM + 10u);
}

/* LMTOCM +14 -- CM p. 265. */
void lmtocm_after_gentran(void)
{
    continue_at(TACHEXIT); /* TCF TACHEXIT */
}
#endif

#if LUMINARY099
/* ---- V85 (bank 40) ---- */

/* DSPRRLOS -- LM p. 299, where VERB85's POSTJUMP goes.
 * Verb 85, the rendezvous radar's line of sight: the job RRLOSDSP at
 * priority 5. */
void dsprrlos(void)
{
    set_A(PRIO5);
    tc(DSPRRLOS + 1u, FINDVAC); /* 2CADR RRLOSDSP */
}

/* DSPRRLOS +4 -- LM p. 299: at priority 4, V16 N56 (flashing, GOMARKFR):
 * RR-AZ and RR-ELEV. */
void dsprrlos_after_findvac(void)
{
    set_A(PRIO4);
    tc(DSPRRLOS + 5u, PRIOCHNG);
}

/* DSPRRLOS +6 -- LM p. 299. */
void dsprrlos_after_priochng(void)
{
    set_A(V16N56);
    tc(DSPRRLOS + 7u, BANKCALL); /* CADR GOMARKFR */
}

/* DSPRRLOS +9, +10 and +11 -- LM p. 299: terminate, proceed, and enter
 * or recycle each end it: B5OFF, extended verb bit 5 off, which RRLOSDSP
 * watches. */
void dsprrlos_after_terminate(void)
{
    tc(DSPRRLOS + 9u, B5OFF);
}

void dsprrlos_after_proceed(void)
{
    tc(DSPRRLOS + 10u, B5OFF);
}

void dsprrlos_after_enter(void)
{
    tc(DSPRRLOS + 11u, B5OFF);
}

/* DSPRRLOS +12 -- LM p. 299: GOMARKFR's immediate return, with the
 * display up: blank R3 (BIT3) and end the job. */
void dsprrlos_immediate_return(void)
{
    set_A(BIT3);
    tc(DSPRRLOS + 13u, BLANKET);
}

/* DSPRRLOS +14 -- LM p. 299. */
void dsprrlos_after_blanket(void)
{
    tc(DSPRRLOS + 14u, ENDOFJOB);
}

/* RRLOSDSP -- LM pp. 299-300.
 * The radar's trunnion and shaft angles (CDUT, CDUS) into MPAC, then
 * RRLOSDSP_CODE (below): "GET RR LOS IN BODY AXIS" and the angles shown,
 * 0-360 degrees. */
void rrlosdsp(void)
{
    dca(ADDRESS_OF(CDUT)); /* EXTEND; DCA CDUT */
    dxch(ADDRESS_OF(MPAC));
    tc(RRLOSDSP + 3u, INTPRET);
}

/* RRLOSDSP +39 -- LM p. 300: a second's wait. */
void rrlosdsp_after_exit(void)
{
    set_A(ONE_SEC);               /* CA 1SEC */
    tc(RRLOSDSP + 40u, BANKCALL); /* CADR DELAYJOB */
}

/* RRLOSDSP +42 -- LM p. 300: again while the display is up (extended verb
 * bit 5, BIT5 of EXTVBACT); else the display is released. */
void rrlosdsp_after_delayjob(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(EXTVBACT));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc(RRLOSDSP + 45u, RRLOSDSP);
        return;
    }
    tc(RRLOSDSP + 46u, ENDEXT);
}
#endif

/* ---- The interpretive code ----
 * The words the TC INTPRETs above hand to the interpreter, as the listing
 * writes them (src/interpreter/notation.h): the LM's are part 8, the CM's
 * part 9. Each EXIT comes back to the basic word after it. */

/* ATTACHIT +1 -- LM p. 297, CM p. 264. Integration held off (INTSTALL:
 * it waits for an integration under way to finish and holds off the
 * next); then the other vehicle's sphere of influence becomes this one's:
 * MOONOTH set, and cleared again unless MOONTHIS is set (BON MOONTHIS +3
 * goes on to the EXIT). */
static const word attachit_words[] = {
    ORDER(CALL), /* ATTACHIT +1 */
    ADDRESS(INTSTALL),     ORDERS(SET, BON),         SWITCH(SET, MOONOTH),
    SWITCH(BON, MOONTHIS), ADDRESS_AT(ATTACHIT, 9u), /* +3 */
    ORDER(CLEAR),          SWITCH(CLEAR, MOONOTH),   ORDER(EXIT),
};

/* TCPIN -- LM p. 298, CM p. 265, where INTWAKE goes on, through QPRET:
 * RTB PINBRNCH, back to PINBALL's job. */
static const word tcpin_words[] = {
    ORDER(RTB), /* TCPIN */
    ADDRESS(PINBRNCH),
};

#if LUMINARY099
/* TRMTRAK1 +7 -- LM pp. 287-288. "DON'T INTERRUPT INTEGRATION". */
static const word trmtrak1_words[] = {
    ORDER(CALL), /* TRMTRAK1 +7 */
    ADDRESS(INTSTALL),
    ORDER(EXIT),
};

/* ATTACHIT +16 -- LM p. 297. "UPDATE R-OTHER, V-OTHER": PTOALEM moves the
 * permanent state vector (now the copy GENTRAN made) to the LM's, and
 * SVDWN1, with X2 the body (PBODY), brings it to the present. */
static const word attachit_words_2[] = {
    ORDER(CALL),                           /* ATTACHIT +16 */
    ADDRESS(PTOALEM), ORDERS(LXA_2, CALL), /* LXA,2  CALL */
    ADDRESS(PBODY),   ADDRESS(SVDWN1),     ORDER(EXIT),
};

/* RRLOSDSP +4 -- LM pp. 299-300.
 * "GET RR LOS IN BODY AXIS" (RRNBMPAC: the radar's angles in MPAC to a
 * unit vector, the "UNIT LOS", at 0D). Its projection into the X-Z plane
 * (the Y component zeroed from HI6ZEROS), made a unit vector at 6D, gives
 * the elevation: its dot products with UNITZ and UNITX are the cosine and
 * sine (COSTH, 16D, and SINTH, 18D) ARCTRIG takes; the azimuth, the angle
 * between the line of sight and that projection: its Y component and its
 * dot product with the projection. Each angle, if negative, has a whole
 * revolution added (DPPOSMAX): "INSURE DISPLAY OF 0-360 DEG." */
static const word rrlosdsp_words[] = {
    ORDER(CALL), /* RRLOSDSP +4 */
    ADDRESS(RRNBMPAC),
    STORE_D(0), /* "UNIT LOS" */
    STODL_D(6),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STOVL_D(8),
    ADDRESS_PLUS_ONE_D(6),
    ORDER(UNIT),
    STORE_D(6), /* "UNIT OF LOS PROJ IN X-Z PLANE" */
    ORDER(DOT),
    ADDRESS_PLUS_ONE(UNITZ),
    STOVL(COSTH),
    ADDRESS_PLUS_ONE(UNITX),
    ORDER(DOT),
    ADDRESS_PLUS_ONE_D(6),
    STCALL(SINTH),
    ADDRESS(ARCTRIG),
    ORDERS(BPL, DAD),           /* "INSURE DISPLAY OF 0-360 DEG." */
    ADDRESS_AT(RRLOSDSP, 24u),  /* +2 */
    ADDRESS_PLUS_ONE(DPPOSMAX), /* "INTRODUCES AND ERROR OF B-28 REVS." */
    STOVL(RR_ELEV),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(UNITY),
    STOVL(SINTH),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(DOT),
    ADDRESS_PLUS_ONE_D(6),
    STCALL(COSTH),
    ADDRESS(ARCTRIG),
    ORDERS(BPL, DAD),
    ADDRESS_AT(RRLOSDSP, 37u), /* +2 */
    ADDRESS_PLUS_ONE(DPPOSMAX),
    STORE(RR_AZ),
    ORDER(EXIT),
};

#else
/* TRACKTRM +23 -- CM p. 259. "DONT INTERRUPT INTEGRATION". */
static const word tracktrm_words[] = {
    ORDER(CALL), /* TRACKTRM +23 */
    ADDRESS(INTSTALL),
    ORDER(EXIT),
};

/* SETVHFLG +1 -- CM p. 264. "SET VHF RANGE FLAG". */
static const word setvhflg_words[] = {
    ORDERS(SET, EXIT), /* SETVHFLG +1 */
    SWITCH(SET, VHFRFLAG),
};

/* RESETVHF +1 -- CM p. 264. "RESET VHF RANGE FLAG". */
static const word resetvhf_words[] = {
    ORDERS(CLEAR, EXIT), /* RESETVHF +1 */
    SWITCH(CLEAR, VHFRFLAG),
};

/* TACHEXIT +2 -- CM pp. 264-265. "UPDATE RN, VN, R-OTHER, V-OTHER":
 * PTOACSM moves the permanent state vector to the CSM's, and SVDWN1, with
 * X2 the body (PBODY), and SVDWN2 bring this vehicle's and the other's to
 * the present. */
static const word tachexit_words[] = {
    ORDER(CALL),                           /* TACHEXIT +2 */
    ADDRESS(PTOACSM), ORDERS(LXA_2, CALL), /* LXA,2  CALL */
    ADDRESS(PBODY),   ADDRESS(SVDWN1),     ORDER(CALL), ADDRESS(SVDWN2), ORDER(EXIT),
};

/* LMTOCM +1 -- CM p. 265. As ATTACHIT's, the other way round: MOONTHIS
 * set, and cleared again unless MOONOTH is set. */
static const word lmtocm_words[] = {
    ORDER(CALL), /* LMTOCM +1 */
    ADDRESS(INTSTALL),    ORDERS(SET, BON),        SWITCH(SET, MOONTHIS),
    SWITCH(BON, MOONOTH), ADDRESS_AT(LMTOCM, 9u), /* +3 */
    ORDER(CLEAR),         SWITCH(CLEAR, MOONTHIS), ORDER(EXIT),
};
#endif

#define WORDS(array) (sizeof(array) / sizeof((array)[0]))

const struct interpretive_code ATTACHIT_CODE = {043u, ATTACHIT + 1u, WORDS(attachit_words),
                                                attachit_words};
const struct interpretive_code TCPIN_CODE = {043u, TCPIN, WORDS(tcpin_words), tcpin_words};
#if LUMINARY099
const struct interpretive_code TRMTRAK1_CODE = {042u, TRMTRAK1 + 7u, WORDS(trmtrak1_words),
                                                trmtrak1_words};
const struct interpretive_code ATTACHIT_CODE_2 = {043u, ATTACHIT + 16u, WORDS(attachit_words_2),
                                                  attachit_words_2};
const struct interpretive_code RRLOSDSP_CODE = {040u, RRLOSDSP + 4u, WORDS(rrlosdsp_words),
                                                rrlosdsp_words};
#else
const struct interpretive_code TRACKTRM_CODE = {043u, TRACKTRM + 23u, WORDS(tracktrm_words),
                                                tracktrm_words};
const struct interpretive_code SETVHFLG_CODE = {043u, SETVHFLG + 1u, WORDS(setvhflg_words),
                                                setvhflg_words};
const struct interpretive_code RESETVHF_CODE = {043u, RESETVHF + 1u, WORDS(resetvhf_words),
                                                resetvhf_words};
const struct interpretive_code TACHEXIT_CODE = {043u, TACHEXIT + 2u, WORDS(tachexit_words),
                                                tachexit_words};
const struct interpretive_code LMTOCM_CODE = {043u, LMTOCM + 1u, WORDS(lmtocm_words), lmtocm_words};
#endif
