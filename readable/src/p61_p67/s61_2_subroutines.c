/* SPDX-License-Identifier: MIT */
/* P61-P67.agc of the CM, part 3 (Comanche055 pp. 812-816, banks 26 and
 * 27): the subroutines S61.2 calls. FISHCALC (the Fischer radius at the
 * end of a transfer angle), VGAMCALC (the velocity and flight path angle
 * at a terminal radius, by vis viva), TFF/TRIG (the sine and cosine of the
 * transfer angle), DISPTARG and VRCALC (the range to the target).
 *
 * They read what the TFF routines (TIME_OF_FREE_FALL.agc) leave on the
 * push-down list, under those routines' names: "THE FOLLOWING PUSH LIST
 * LOCATIONS HAVE BEEN RESERVED FOR TFF ROUTINES": NRTERM (16D), the
 * terminal radius normalized, and NRMAG (32D) the present one; TFFX (34D),
 * TFFTEM (36D), TFFNP (28D), TFF/RTMU (30D) and TFFVSQ (20D); and
 * CDELF/2 (14D), the cosine TFF/TRIG leaves. X1 holds -NR, the
 * normalization's shift. All interpretive. */
#include "p61_p67.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* FISHCALC: "GIVEN THE PRESENT POSITION, UNITR, CALCULATE A NEW UNITR THAT
 * IS ROTATED THROUGH TRANSFER ANGLE, THETA, ALONG THE TRAJECTORY.": "URPR
 * = UR CDELF + UHOR SDELF", UHOR being URONE cross UNI, the sine coming in
 * MPAC. The new unit vector is URH, "FOR USE IN RTGO FROM EMS DISPLAY";
 * its dot with UNITW, the "UNIT NORTH POLE", is ".5 SIN(LAT)" in ALPHAV
 * +4, for GETERAD (DUMPFISH), which "SAVES FISCHER RAD (-29) M IN ERADM
 * AND IN MPAC." and returns to the caller.
 *
 * PREVGAM: "ENTER WITH NEW RTERM IN MPAC", normalized by X1 to NRTERM;
 * then VGAMCALC: "EARTH CENTERED VIS VIVA CALCULATION OF TERMINAL VELOCITY
 * AND GAMMA": "VGAM = SQRT(VN VN/MU + 2(RN-RTERM)/(RN RTERM) ) RTMU", left
 * in word 0, and "COSGAM = H / RTERM VGAM = SQRT (LCP) / (RTERM
 * VGAM/RTMU)", GAMMA / 360 in MPAC, positive: "CALLER MUST SUPPLY OWN SIGN
 * ...". The DDV's overflow is the limiter: "USE DDV OVFL AS LIMITER (|COS|
 * <1.0)". DUMPVGAM: RVQ. */
static const word fishcalc_words[] = {
    /* FISHCALC */ ORDERS(PDVL, VXV), /* "URPR = UR CDELF + UHOR SDELF" */
    ADDRESS_PLUS_ONE(URONE),
    ADDRESS_PLUS_ONE(UNI),
    ORDERS(VXSC, VSL1), /* "SIN(THETA) / 2 FROM PDL+0" */
    ORDERS(PDVL, VXSC),
    ADDRESS_PLUS_ONE(URONE),
    ADDRESS_PLUS_ONE(CDELF_2), /* "COS(THETA) / 2" */
    ORDERS(VAD, STADR),
    AFTER_STADR(STORE(URH)), /* "FOR USE IN RTGO FROM EMS DISPLAY" */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UNITW),
    STORE_AT(ALPHAV, 4), /* "= .5 SIN(LAT)" */
    /* DUMPFISH */ ORDER(GOTO),
    ADDRESS(GETERAD),
    /* PREVGAM */ ORDER(SL_STAR), /* "ENTER WITH NEW RTERM IN MPAC" */
    SL_X1(0),                     /* "X1 = -NR" */
    STORE(NRTERM),
    /* VGAMCALC */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(NRMAG),
    ADDRESS_PLUS_ONE(NRTERM),
    ORDERS(PDDL, DSU), /* "RMAG RTERM M" */
    ADDRESS_PLUS_ONE(NRMAG),
    ADDRESS_PLUS_ONE(NRTERM),
    ORDERS(SL_STAR, DDV), /* "2(RN-RTERM)" */
    SL_X1(-8),            /* "PUSH UP PRODUCT." */
    ORDER(DSU),
    ADDRESS_PLUS_ONE(TFFVSQ), /* "-(V SQ/MU)" */
    ORDERS(SQRT, PUSH),       /* "SAVE VGAM/RT(MU) FOR NOW." */
    ORDERS(DDV, PDDL),        /* "XCH PDL+0, LEAVING VGAM FOR OUTPUT." */
    ADDRESS_PLUS_ONE(TFF_RTMU),
    ORDERS(DMP, PDDL), /* "RTERM VGAM/RTMU" */
    ADDRESS_PLUS_ONE(NRTERM),
    ADDRESS_PLUS_ONE(TFFNP), /* "LC P =H.H/MU M" */
    ORDERS(SQRT, DDV),       /* "PUSH UP DEN" */
    ORDERS(SR1, ACOS),
    /* DUMPVGAM */ ORDER(RVQ),
};

const struct interpretive_code FISHCALC_CODE = {026u, FISHCALC, COUNT(fishcalc_words),
                                                fishcalc_words};

/* TFF/TRIG: "COS(THETA) = 1-2 ABS(ARG) / (RN RTERM (1+X) )", from TFFX
 * and TFFTEM, the ARG, to CDELF/2; then the sine, the square root of 1
 * less the cosine squared, "KEEP HONEST FOR SQRT.", with TFFTEM's sign
 * (DUMPTRIG): "RETURN WITH .5 SIN(THETA) IN MPAC".
 *
 * DISPTARG, with its return in 60GENRET (STQ): the time to the target,
 * the range angle (in MPAC, "TRGO ESTIMATE") times KTETA1 less TTE1, to
 * DTEAROT, the earth's turn in that time (EARROT2); then VRCALC, and back
 * through 60GENRET. VRCALC: the angle between URH and RT, the target
 * (ACOS of their dot); RVQ. */
static const word tff_trig_words[] = {
    /* TFF/TRIG */ ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(TFFX),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(HIDPHALF),
    ADDRESS_PLUS_ONE(NRMAG), /* "RMAG M" */
    ORDERS(DMP, BDDV),
    ADDRESS_PLUS_ONE(NRTERM), /* "RTERM M" */
    ADDRESS_PLUS_ONE(TFFTEM), /* "P ZSQ OR P/ALFA" */
    ORDERS(ABS, BDSU),        /* "THE SIGN IS FOR SDELF." */
    ADDRESS_PLUS_ONE(HIDPHALF),
    STORE(CDELF_2),     /* ".5 COS(THETA)" */
    ORDERS(DSQ, DCOMP), /* "KEEP HONEST FOR SQRT." */
    ORDERS(DAD, SQRT),
    ADDRESS_PLUS_ONE(HIDP1_4),
    /* DUMPTRIG */ ORDERS(SIGN, RVQ),
    ADDRESS_PLUS_ONE(TFFTEM), /* "AFFIX SIGN(DELE/2)" */
    /* DISPTARG */ ORDER(STQ),
    ADDRESS(60GENRET),
    ORDERS(DMP, DSU),
    ADDRESS_PLUS_ONE(KTETA1),
    ADDRESS_PLUS_ONE(TTE1),
    STCALL(DTEAROT),
    ADDRESS(EARROT2),
    ORDER(CALL),
    ADDRESS(VRCALC),
    ORDER(GOTO),
    ADDRESS(60GENRET),
    /* VRCALC */ ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(URH),
    ADDRESS_PLUS_ONE(RT),
    ORDERS(SL2, ACOS),
    ORDER(RVQ),
};

const struct interpretive_code TFF_TRIG_CODE = {027u, TFF_TRIG, COUNT(tff_trig_words),
                                                tff_trig_words};

#endif
