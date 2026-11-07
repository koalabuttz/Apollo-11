/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc, pp. 844-847 (Comanche055, bank 25): the "ENTRY
 * INITIALIZATION ROUTINE", STARTENT, its target vector (STARTEN1), and the
 * pass SERVICER's CM/POSE makes every two seconds (SCALEPOP, REFAZE10).
 * The faithful C is c64-port/src/cm_reentry.c. */
#include "reentry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- STARTENT -- pp. 844-846 ---- */

/* STARTENT ("MM = 63", "COME HERE FROM CM/POSE.  RESTARTED IN CM/POSE."):
 * out of the interpreter, ... */
static const word startent_words[] = {
    /* STARTENT */ ORDER(EXIT),
};

const struct interpretive_code STARTENT_CODE = {025u, STARTENT, COUNT(startent_words),
                                                startent_words};

/* ... to "INITIALIZE ALL SWITCHES TO ZERO EXCEPT LATSW, ENTRYDSP, AND
 * GONEPAST." (CM/FLAGS without ENTMASK's bits) and "SET ENTRYDSP, LATSW,
 * GONEPAST." (ENTRYSW's), with interrupts inhibited; ... */
void startent_after_exit(void)
{
    set_A(negate(ENTMASK));
    inhint();
    mask_at(ADDRESS_OF(CM_FLAGS));
    ad(ENTRYSW);
    ts(ADDRESS_OF(CM_FLAGS));
    relint();
    tc_at(STARTENT + 7u);
    intpret();
}

/* ... and back in the interpreter: LOD and LAD from their pad loads,
 * "L/DCMINR = LAD COS(15)", "KLAT = LAD/24" (LATSLOPE), "Q7 = Q7F",
 * FACTOR 1.0 ("1.0 -1BIT"), and "L/D = - LAD SGN(HEADSUP)", HEADSUP "MAY
 * BE NOISE FOR DISPLAY P61"; then the target vector (STARTEN1, "RETURN
 * VIA GOTOADDR"). LATANG "= UNI.RT /4", the target's side of the orbit
 * plane, and "K2ROLL = -SGN(LATANG)"; "Q2 = -1152 + 500 LAD". GOTOADDR
 * INITROLL ("SET SELECTOR FOR INITIAL PASS") and POSEXIT SCALEPOP ("SET
 * CM/POSE TO CONTINUE AT SCALEPOP"); SERVNOUT ends the pass: "OMIT
 * INITIAL DISPLAY, SINCE 1ST GUESSBAD".
 *
 * STARTEN1: "CALCULATE THE INITIAL TARGET VECTOR: RTINIT", the splash
 * point LAT(SPL) at altitude 0 ("SET ALT=0."), from LALOTORV ("DO CALL
 * USING PAD RADIUS.  WILL UNIT IT."), at PIPTIME ("SAVE TIME BASE OF
 * RTINIT."); the earth's rotation (EARROT1) over 500SEC, "NOMINAL ENTRY
 * TIME FOR P63", gives RT; and THETAH, the "RANGE ANGLE /360", from it
 * and UNITR. Back through GOTOADDR: "RETURN TO CALLER". */
static const word startent_interpretive_words[] = {
    /* STARTENT +010 */ ORDER(SLOAD),
    ADDRESS_PLUS_ONE(LODPAD),
    STORE(LOD),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(LADPAD),
    STORE(LAD),
    ORDER(DMP), /* "L/DCMINR = LAD COS(15)" */
    ADDRESS_PLUS_ONE(COS15),
    STODL(L_DCMINR),
    ADDRESS_PLUS_ONE(LATSLOPE),
    ORDERS(DMP, SR1), /* "KLAT = LAD/24" */
    ADDRESS_PLUS_ONE(LAD),
    STODL(KLAT),
    ADDRESS_PLUS_ONE(Q7F),
    STODL(Q7), /* "Q7 = Q7F" */
    ADDRESS_PLUS_ONE(NEARONE),
    STODL(FACTOR),
    ADDRESS_PLUS_ONE(LAD),
    ORDERS(SIGN, DCOMP),
    ADDRESS_PLUS_ONE(HEADSUP),
    STCALL(L_D), /* "L/D = - LAD SGN(HEADSUP)" */
    ADDRESS(STARTEN1),
    ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(VN),
    ADDRESS_PLUS_ONE(UNITR),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(RT),
    STORE(LATANG), /* "LATANG = UNI.RT /4" */
    ORDERS(DCOMP, RTB),
    ADDRESS(SIGNMPAC),
    STODL(K2ROLL), /* "K2ROLL = -SGN(LATANG)" */
    ADDRESS_PLUS_ONE(LAD),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(Q21),
    ADDRESS_PLUS_ONE(Q22),
    STORE(Q2), /* "Q2 = -1152 + 500 LAD" */
    ORDERS(SSP, SSP),
    ADDRESS_PLUS_ONE(GOTOADDR),
    ADDRESS(INITROLL), /* "SET SELECTOR FOR INITIAL PASS" */
    ADDRESS_PLUS_ONE(POSEXIT),
    ADDRESS(SCALEPOP), /* "SET CM/POSE TO CONTINUE AT SCALEPOP" */
    ORDER(RTB),
    ADDRESS(SERVNOUT),
    /* STARTEN1 */ ORDERS(STQ, VLOAD),
    ADDRESS(GOTOADDR),
    ADDRESS_PLUS_ONE(LAT_SPL), /* "TARGET COORDINATES" */
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, ERADFLAG),
    SWITCH(CLEAR, LUNAFLAG),
    STODL(LAT),
    ADDRESS_PLUS_ONE(3ZEROS),
    STODL_AT(LAT, 4), /* "SET ALT=0." */
    ADDRESS_PLUS_ONE(PIPTIME),
    STCALL(TIME_RTO), /* "SAVE TIME BASE OF RTINIT." */
    ADDRESS(LALOTORV),
    ORDER(UNIT),
    STODL(RTINIT), /* ".5 UNIT TARGET" */
    ADDRESS_PLUS_ONE(500SEC),
    STCALL(DTEAROT), /* "INITIALIZE EARROT" */
    ADDRESS(EARROT1),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UNITR),
    ORDER(ACOS),
    STCALL(THETAH), /* "RANGE ANGLE /360" */
    ADDRESS(GOTOADDR),
};

const struct interpretive_code STARTENT_INTERPRETIVE_CODE = {
    025u, STARTENT + 010u, COUNT(startent_interpretive_words), startent_interpretive_words};

/* ---- SCALEPOP, REFAZE10 -- p. 847 ---- */

/* SCALEPOP, where CM/POSE goes on each pass: the state (TARGETNG), ... */
static const word scalepop_words[] = {
    /* SCALEPOP */ ORDER(CALL),
    ADDRESS(TARGETNG),
    ORDER(EXIT),
};

const struct interpretive_code SCALEPOP_CODE = {025u, SCALEPOP, COUNT(scalepop_words),
                                                scalepop_words};

/* ... REFAZE10: "SERVICER 5.3 RESTART AT REFAZE10" (PHASCHNG, OCT
 * 10035), ... */
void refaze10(void)
{
    tc_to(REFAZE10, PHASCHNG);
}

void refaze10_after_phaschng(void)
{
    tc_at(REFAZE10 + 2u);
    intpret();
}

/* ... and "JUMP TO PARTICULAR RE-ENTRY PHASE": GOTOADDR "CONTAINS THE
 * ADDRESS OF THE ROLL COMMAND EQUATIONS TO THE CURRENT PHASE OF
 * RE-ENTRY." */
static const word refaze10_words[] = {
    /* REFAZE10 +3 */ ORDER(GOTO),
    ADDRESS(GOTOADDR),
};

const struct interpretive_code REFAZE10_CODE = {025u, REFAZE10 + 3u, COUNT(refaze10_words),
                                                refaze10_words};

#endif
