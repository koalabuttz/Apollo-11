/* SPDX-License-Identifier: MIT */
/* P61-P67.agc of the CM, part 2 (Comanche055 pp. 806-811, bank 26):
 * S61.2, which P61 calls for its displays, N60 and N63.
 *
 * S61.2 predicts the entry from the state vector P61 left in RONE and VONE,
 * by conics (the TFF routines, TIME_OF_FREE_FALL.agc): the time, range and
 * velocity at EMSALT above the Fischer ellipsoid (TTE, RTGO, VIO), and the
 * velocity and flight path angle at 400K FT (VPRED, GAMMAEI). Between the
 * two it estimates GMAX, the "MAXIMUM ACCELERATION EXPECTED", by a fit of
 * VGAM and GAMMA at 300K FT. It ends with RTB P61.1, the N60 display.
 *
 * All of it is interpretive. The push-down list carries its numbers: RTERM
 * (18D) is the TFF routines' terminal radius, and word 0 VGAM, which
 * VGAMCALC leaves there. */
#include "p61_p67.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* S61.2: the MU for the conics: an EMSALT under 290K FT is an orbital
 * entry ("ESTABLISH MU FOR ORBITAL ENTRIES", 1/RTMU); above it, LUNENT, a
 * lunar one (1/RTMUE). CALLCON: "FILL VAC AREA WITH CONIC PARAMETERS"
 * (TFFCONIC). Then the time to EMSALT: RTRIAL, a "1ST GUESS AT TERMINAL
 * RADIUS", its transfer angle (CALCTFF, TFF/TRIG), the Fischer radius
 * there (FISHCALC) plus EMSALT, the time again (CALCTFF); negated, "AS IN
 * COUNTDOWN", it is TTE1 and TTE.
 *
 * From TTE on, the range and velocity at EMSALT: the angle (TFF/TRIG),
 * FISHCALC, VRCALC, DISPTARG twice; RTGO, then VGAMCALC's GAMMA and VGAM
 * (to word 0). VIO is VGAM plus VEMSCON / GAMMA ("PREDICTED VELOCITY AT
 * EMSALT."); then VGAM and GAMMA again at 300K FT above the Fischer radius
 * (ERADM, RTERM, PREVGAM, "VGAMCALC WITH NEW RTERM").
 *
 * GMAXCALC: "VBAR = (V(FPS) - 36KF/S) / 20 F/S", and "GMAX = (4/(1 + 4.8
 * VBARSQ))(GAM - 6.05 - 2.4 VBARSQ) - 10(L/D - .3) + 10", with the
 * constants KR1 to KR4 below; a negative GMAX is 0 (HI6ZEROS). Then "2 ND
 * ITERATION FOR FISCHER RADIUS" at 400K FT: CALCTFF, TFF/TRIG, FISHCALC,
 * PREVGAM again. GAMMAEI's low word is RTGO's high one, by X2 ("FOR TM,
 * DP(GAMMAEI) = (GAMMA, RTGO) / 360"); VPRED is VGAM from word 0 (STADR);
 * then RTB P61.1. */
static const word s61_2_words[] = {
    /* S61.2 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(EMSALT),
    ADDRESS_PLUS_ONE(290KFT),
    ORDERS(BPL, DLOAD),
    ADDRESS(LUNENT),
    ADDRESS_PLUS_ONE(1_RTMU), /* "ESTABLISH MU FOR ORBITAL ENTRIES" */
    /* CALLCON */ ORDER(CALL),
    ADDRESS(TFFCONIC), /* "FILL VAC AREA WITH CONIC PARAMETERS" */
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(RTRIAL), /* "1ST GUESS AT TERMINAL RADIUS" */
    ADDRESS(CALCTFF),         /* "SAVES MPAC IN RTERM" */
    ORDER(CALL),
    ADDRESS(TFF_TRIG), /* "RETURN WITH S(THETA) IN MPAC" */
    ORDER(CALL),
    ADDRESS(FISHCALC), /* "ANS IN MPAC AND IN ERADM" */
    ORDERS(DAD, CALL),
    ADDRESS_PLUS_ONE(EMSALT),
    ADDRESS(CALCTFF),
    ORDER(DCOMP), /* "NEGATIVE AS IN COUNTDOWN" */
    STORE(TTE1),
    STCALL(TTE), /* "LET MISS CONTRL DECR BY ELAPSED TIME" */
    ADDRESS(TFF_TRIG),
    ORDER(CALL),
    ADDRESS(FISHCALC),
    ORDER(CALL),
    ADDRESS(VRCALC),
    ORDER(CALL),
    ADDRESS(DISPTARG),
    ORDER(CALL),
    ADDRESS(DISPTARG),
    STCALL(RTGO),
    ADDRESS(VGAMCALC),
    ORDER(DMP), /* "MPAC = GAMMA" */
    ORDERS(BDDV, DAD),
    ADDRESS_PLUS_ONE(VEMSCON),
    ADDRESS_PLUS_ONE_D(0), /* "VGAM FROM PDL0" */
    STODL(VIO),            /* "PREDICTED VELOCITY AT EMSALT." */
    ADDRESS_PLUS_ONE(ERADM),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(300KFT),
    STCALL(RTERM),                    /* "TERMINAL RADIUS M (-29)" */
    ADDRESS(PREVGAM),                 /* "VGAMCALC WITH NEW RTERM" */
    /* GMAXCALC */ ORDERS(PDDL, DSU), /* "GAM TO PDL2" */
    ADDRESS_PLUS_ONE_D(0),            /* "VGAM IS IN PDL0" */
    ADDRESS_PLUS_ONE(36KFT_S),
    ORDERS(DDV, DSQ),
    ADDRESS_PLUS_ONE(20KFT_S),
    STORE_D(0), /* "VBARSQ (-2) TO PDL0" */
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(KR1), /* (the DAD's: "GAM, POS DOWN, FROM PDL2") */
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(M6_05DEG),
    ADDRESS_PLUS_ONE(KR2),
    ORDER(PDDL), /* "XCH PDL+0 FOR VBARSQ" */
    ORDERS(DDV, DAD),
    ADDRESS_PLUS_ONE(KR4),
    ADDRESS_PLUS_ONE(DP2_M4),
    ORDER(BDDV), /* "NUM FROM PDL+0" */
    ORDERS(DAD, BPL),
    ADDRESS_PLUS_ONE(KR3),
    ADDRESS_AT(S61_2, 077), /* "+3": past the DLOAD */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    STODL(GMAX),
    ADDRESS_PLUS_ONE(ERADM),
    ORDERS(DAD, CALL), /* "2 ND ITERATION FOR FISCHER RADIUS" */
    ADDRESS_PLUS_ONE(400KFT),
    ADDRESS(CALCTFF), /* "ESTABLISH TRANSFER ANGLE DATA." */
    ORDER(CALL),
    ADDRESS(TFF_TRIG), /* "GET SIN, COS DELF" */
    ORDER(CALL),
    ADDRESS(FISHCALC),  /* "GET CORRESPONDING FISCHER RADIUS." */
    ORDERS(DAD, LXA_2), /* "SAVE HI-WORD FOR DOWNLIST." */
    ADDRESS_PLUS_ONE(400KFT),
    ADDRESS(RTGO),
    STCALL(RTERM),
    ADDRESS(PREVGAM),
    ORDERS(DCOMP, SXA_2), /* "HI-WORD OF EACH ON DOWNLIST." */
    ADDRESS_AT(MPAC, 1),
    STODL(GAMMAEI), /* "CONIC GAMMA/360 AT 400K FT." */
    ORDER(STADR),
    AFTER_STADR(STORE(VPRED)), /* "CONIC VELOCITY AT 400K FT" */
    ORDER(RTB),
    ADDRESS(P61_1),
    /* LUNENT */ ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(1_RTMUE), /* "ESTABLISH MU FOR LUNAR TYPE ENTRIES" */
    ADDRESS(CALLCON),
};

const struct interpretive_code S61_2_CODE = {026u, S61_2, COUNT(s61_2_words), s61_2_words};

#endif
