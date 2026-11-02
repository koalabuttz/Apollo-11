/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 14 (Luminary099 pp. 585-586): GETULC, RADARANG and
 * LGCUPDTE, the subroutines LSR22.3 and LSR22.4 call for each mark.
 *
 * GETULC "COMPUTES THE RELATIVE POSITION VECTOR BETWEEN THE CSM AND THE
 * LM, LEAVING THE UNIT VECTOR IN THE PUSHLIST AND MPAC AND THE MAGNITUDE
 * IN 36D": each vehicle's position is its conic position (RCVLEM, RCVCSM)
 * plus its deviation from it (DELTALEM, DELTACSM), the deviation shifted
 * for the sphere of influence (X2, SCALSHFT). RADARANG adds the trunnion
 * angle's sine, "SINTHETA = -ULC DOT MY", and RXZ, "(SQRT
 * (1-SINTHETA**2))RLC", normalized (the shift count in X1).
 *
 * LGCUPDTE gives one measurement to INCORP1, which finds the state
 * vector's deviation (DELTAX). Its position and velocity parts' lengths
 * go to R22DISP for noun 49. Over RMAX or VMAX, R22 shows them to the crew
 * (R22LEM96); else, or when the crew accepts them (ASTOK), INCORP2
 * corrects the state vector.
 *
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. GETULC's RTB NORMUNX1 ends one: the
 * pass that resumes at DANZIG with the PUSH in EDOP is its own key. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* GETULC -- p. 585: the push-down list from 0. The LM's position: DELTALEM
 * "SHIFT FOR EARTH/MOON SPHERE" (X2, "LOAD X2 WITH SCALE SHIFT"), plus
 * RCVLEM; the CSM's likewise from DELTACSM and RCVCSM, less the LM's. "USE
 * NORMUNIT TO PRESERVE ACCURACY" (NORMUNX1, its shift count in X1), ULC;
 * the length (36D) "ADJUST MAGNITUDE FROM NORMUNIT". "ULC IN PD0 AND
 * MPAC,RLC IN 36D".
 *
 * RADARANG -- p. 586: its return in RDRET; GETULC. "SIN THETA TO PD6"
 * (-ULC DOT MY, doubled) and to SINTHETA; "1-(SIN THETA)**2", its square
 * root times RLC (36D), normalized: "SET SHIFT COUNTER IN X1", RXZ.
 * "EXIT" through RDRET.
 *
 * LGCUPDTE -- its return in LGRET; INCORP1. The velocity deviation's
 * length (DELTAX +6) by SCALSHFT ("0 -- MOON. 2 -- EARTH"), "SET VEL
 * DISPLAY TO B-7", to R22DISP +2; the position's, "SET POS DISPLAY TO
 * B-29", to R22DISP. RMAX less the position's, negative: "GO DISPLAY"
 * (R22LEM96); "VMAX MINUS VEL. DEVIATION", negative: the same. ASTOK:
 * INCORP2, and back through LGRET. */
static const word getulc_words[] = {
    /* GETULC */ ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(DELTALEM),
    ORDER(LXA_2),
    ADDRESS(SCALSHFT), /* "LOAD X2 WITH SCALE SHIFT" */
    ORDERS(VSR_STAR, VAD),
    SR_X2(9), /* "SHIFT FOR EARTH/MOON SPHERE" */
    ADDRESS_PLUS_ONE(RCVLEM),
    ORDERS(PDVL, VSR_STAR),
    ADDRESS_PLUS_ONE(DELTACSM),
    SR_X2(9), /* "SHIFT FOR EARTH/MOON SPHERE" */
    ORDERS(VAD, VSU),
    ADDRESS_PLUS_ONE(RCVCSM),
    ORDERS(RTB, PUSH), /* "USE NORMUNIT TO PRESERVE ACCURACY" */
    ADDRESS(NORMUNX1),
    STODL(ULC),
    ADDRESS_PLUS_ONE_D(36),
    ORDER(SL_STAR), /* "ADJUST MAGNITUDE FROM NORMUNIT" */
    SL_X1(0),
    STOVL_D(36), /* "ULC IN PD0 AND MPAC,RLC IN 36D" */
    ADDRESS_PLUS_ONE(ULC),
    ORDER(RVQ),
    /* RADARANG */ ORDERS(STQ, CALL),
    ADDRESS(RDRET),
    ADDRESS(GETULC),
    ORDERS(VCOMP, DOT),
    ADDRESS_PLUS_ONE(MY),
    ORDERS(SL1R, PUSH), /* "SIN THETA TO PD6" */
    STORE(SINTHETA),
    ORDERS(DSQ, BDSU),
    ADDRESS_PLUS_ONE(DP1_4TH), /* "1-(SIN THETA)**2" */
    ORDERS(SQRT, DMP),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(SL1, NORM),
    ADDRESS_PLUS_ONE(X1), /* "SET SHIFT COUNTER IN X1" */
    STORE(RXZ),
    ORDER(GOTO), /* "EXIT" */
    ADDRESS(RDRET),
    /* LGCUPDTE */ ORDERS(STQ, CALL),
    ADDRESS(LGRET),
    ADDRESS(INCORP1),
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE_AT(DELTAX, 6),
    ORDERS(LXA_2, SRR_STAR),
    ADDRESS(SCALSHFT), /* "0 -- MOON. 2 -- EARTH" */
    SRR_X2(2),         /* "SET VEL DISPLAY TO B-7" */
    STOVL_AT(R22DISP, 2),
    ADDRESS_PLUS_ONE(DELTAX),
    ORDERS(ABVAL, SRR_STAR),
    SRR_X2(2), /* "SET POS DISPLAY TO B-29" */
    STORE(R22DISP),
    ORDERS(SLOAD, SR),
    ADDRESS_PLUS_ONE(RMAX),
    SR_COUNT(10),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(R22DISP),
    ADDRESS(R22LEM96), /* "GO DISPLAY" */
    ORDERS(SLOAD, DSU),
    ADDRESS_PLUS_ONE(VMAX),
    ADDRESS_PLUS_ONE_AT(R22DISP, 2), /* "VMAX MINUS VEL. DEVIATION" */
    ORDER(BMN),
    ADDRESS(R22LEM96), /* "GO DISPLAY" */
    /* ASTOK */ ORDER(CALL),
    ADDRESS(INCORP2),
    ORDER(GOTO),
    ADDRESS(LGRET),
};

const struct interpretive_code GETULC_CODE = {026u, GETULC, COUNT(getulc_words), getulc_words};

#endif
