/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 18 (Comanche055 pp. 627-630): HORIZ, DIVIDE and
 * RECT.1 (bank 31), all interpretive code.
 *
 * HORIZ, which POINTAXS calls when no landmark is given: the point on the
 * earth's or the moon's horizon the star is marked on. UBAR0, UBAR1 and
 * UBAR2 are a frame from the star's direction (US) and the CSM's position
 * (RZC): UBAR2 across both, UBAR0 across UBAR2 and US, UBAR1 across
 * UBAR0 and UBAR2. The planet is an ellipse in it, its axes AH and BH:
 * for the earth the Fischer ellipsoid's radius under the CSM (GETERAD)
 * and AEARTH, each plus HORIZALT; for the moon (LUNAFLAG) RADMOON
 * twice. RH and USH are the CSM's position and the star's direction in
 * the frame. Of the two points where lines from RH touch the ellipse
 * (T-0 and T1), the near or far horizon (NORFHOR) is chosen by which side
 * of the star each lies (A-SUB-ZERO and A-SUB-ONE), and taken back out of
 * the frame (VXM UBAR0); HORIZ returns through SRRETURN.
 *
 * DIVIDE: 34D divided by MPAC, each normalized first (NORM: the shift in
 * X1, then S1) for a quotient with the bits to spare; X1 less S1 is the
 * shift the quotient needs (SR*, back in HORIZ).
 *
 * RECT.1 picks up the CSM's state vector from the permanent one: the
 * conic position and velocity (RCVCSM, VCVCSM) plus their deviations
 * (DELTACSM, NUVCSM), to RZC "NOW SCALED B29" and VZC "NOW SCALED B7". X2
 * is the scale shift: 2 more in the lunar sphere (CMOONFLG). ZMEASURE is
 * set when the planet measured is not the one the CSM's state vector is
 * about (LUNAFLAG and CMOONFLG differ). */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* HORIZ: the return in SRRETURN, the push-down pointer 0. */
static const word horiz_words[] = {
    ORDERS(STQ, SETPD),
    ADDRESS(SRRETURN),
    ADDRESS_PLUS_ONE_D(0),
    /* "PUSH 0-1 = -AYO SCALED B0" */
    ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE(MAYO),
    ADDRESS_PLUS_ONE(AXO),
    /* "PUSH 2-3 = +AX SCALED B0" */
    ORDERS(PDDL, PDVL),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    ADDRESS_PLUS_ONE(US),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RZC),
    STOVL(UBAR2),
    /* "PUSH UP" */
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UBAR2),
    STOVL(UBAR0),
    ADDRESS_PLUS_ONE(UBAR2),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UBAR0),
    STORE(UBAR1),
    ORDERS(BON, DOT),
    SWITCH(BON, LUNAFLAG),
    ADDRESS(HORIZ_6),
    /* "UBAR1 DOT UZ" */
    ADDRESS_PLUS_ONE_D(0),
    STCALL_AT(ALPHAV, 4),
    ADDRESS(GETERAD),
    /* "MPAC HAS RADIUS OF FISCHER ELLIPSOID" */
    ORDERS(DAD, PDDL),
    /* "PUSH 0-1 = BH SCALED B29" */
    ADDRESS_PLUS_ONE(HORIZALT),
    ADDRESS_PLUS_ONE(AEARTH),
    /* "PUSH 2-3 = AH B29" */
    ORDERS(DAD, PUSH),
    ADDRESS_PLUS_ONE(HORIZALT),
    /* HORIZ.1 */
    ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(RZC),   /* "B29" */
    ADDRESS_PLUS_ONE(UBAR0), /* "B1" */
    /* "PUSH 4-9 = RH(XH,YH,ZH) B29" */
    ORDERS(VSL1, PDVL),
    ADDRESS_PLUS_ONE(US),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(UBAR0),
    /* "PUSH 10-15 = USH B1" */
    ORDER(PDDL),
    ADDRESS_PLUS_ONE_D(2), /* "AH" */
    STODL_D(34),
    ADDRESS_PLUS_ONE_D(4), /* "XH" */
    ORDER(CALL),
    ADDRESS(DIVIDE),
    ORDERS(SR_STAR, DMP),
    SR_X1(8), /* "NOW SCALED B9" */
    ADDRESS_PLUS_ONE(MPAC),
    STODL_D(30),
    ADDRESS_PLUS_ONE_D(0),
    STODL_D(34),
    ADDRESS_PLUS_ONE_D(6), /* "YH" */
    ORDER(CALL),
    ADDRESS(DIVIDE),
    ORDERS(SR_STAR, DMP),
    SR_X1(8),               /* "B9" */
    ADDRESS_PLUS_ONE(MPAC), /* "B18" */
    /* "PUSH 16-17 =A SCALED B18" */
    ORDERS(DAD, PUSH),
    ADDRESS_PLUS_ONE_D(30),
    ORDERS(DSU, SQRT),
    ADDRESS_PLUS_ONE(1_0B18),
    /* "PUSH 18-19 SQRT(A-1) B9" */
    ORDER(PDDL),
    ADDRESS_PLUS_ONE_D(16),
    STODL_D(34),
    ADDRESS_PLUS_ONE_D(4), /* "XH" */
    ORDER(CALL),
    ADDRESS(DIVIDE),
    ORDERS(SR_STAR, PDDL),
    SR_X1(17),             /* "PUSH 20-21 = XH/A B29" */
    ADDRESS_PLUS_ONE_D(6), /* "YH" */
    ORDER(CALL),
    ADDRESS(DIVIDE),
    ORDERS(SR_STAR, PDDL),
    SR_X1(17),              /* "PUSH 22-23 = YH/A B29" */
    ADDRESS_PLUS_ONE_D(16), /* "A" */
    STODL_D(34),
    ADDRESS_PLUS_ONE_D(18), /* "SQRT(A-1)" */
    ORDER(CALL),
    ADDRESS(DIVIDE),
    ORDER(SR_STAR),
    SR_X1(8),
    STODL_D(28),
    ADDRESS_PLUS_ONE_D(0), /* "BH" */
    STODL_D(34),
    ADDRESS_PLUS_ONE_D(2), /* "AH" */
    ORDER(CALL),
    ADDRESS(DIVIDE),
    /* "AH/BH SCALED B1" */
    ORDERS(SR_STAR, DMP),
    SR_X1(0),
    ADDRESS_PLUS_ONE_D(28), /* "SQRT(A-1)/A" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(6), /* "YH" */
    ORDER(PDDL),
    ADDRESS_PLUS_ONE_D(2), /* "AH" */
    STODL_D(34),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(CALL),
    ADDRESS(DIVIDE),
    /* "BH/AH SCALED B1" */
    ORDERS(SR_STAR, DMP),
    SR_X1(0),
    ADDRESS_PLUS_ONE_D(28), /* "SQRT (A-1)/A" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(4), /* "XH" */
    ORDERS(PDDL, DAD),
    ADDRESS_PLUS_ONE_D(20), /* "XH/A" */
    ADDRESS_PLUS_ONE_D(24), /* "ALPHA" */
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE_D(22), /* "YH/A" */
    ADDRESS_PLUS_ONE_D(26), /* "BETA" */
    ORDERS(PUSH, SETPD),
    ADDRESS_PLUS_ONE_D(16),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(20), /* "XH/A" */
    ADDRESS_PLUS_ONE_D(24), /* "ALPHA" */
    ORDERS(PDDL, DAD),
    ADDRESS_PLUS_ONE_D(22), /* "YH/A" */
    ADDRESS_PLUS_ONE_D(26), /* "BETA" */
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STOVL_D(32), /* "ZERO THIRD COMP. OF T-0 VECTOR" */
    ADDRESS_PLUS_ONE_D(28),
    ORDERS(VSU, UNIT),
    ADDRESS_PLUS_ONE_D(4), /* "RH VECTOR" */
    /* "PUSH 22-23 A-SUB-ZERO" */
    ORDERS(DOT, PDVL),
    ADDRESS_PLUS_ONE_D(10), /* "USH VECTOR" */
    ADDRESS_PLUS_ONE_D(16), /* "T1 VECTOR" */
    ORDERS(VSU, UNIT),
    ADDRESS_PLUS_ONE_D(4), /* "RH VECTOR" */
    /* "PUSH 24-25 A-SUB-ONE" */
    ORDERS(DOT, PUSH),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE_D(22), /* "A-SUB-ZERO" */
    ADDRESS(HORIZ_3),
    ORDER(BON),
    SWITCH(BON, NORFHOR),
    ADDRESS(HORIZ_4),
    /* HORIZ.2 */
    ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE_D(28), /* "T-0 VECTOR" */
    ADDRESS(HORIZ_5),
    /* HORIZ.3 */
    ORDERS(BON, GOTO),
    SWITCH(BON, NORFHOR),
    ADDRESS(HORIZ_2),
    ADDRESS(HORIZ_4),
    /* HORIZ.4 */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_D(16), /* "T1 VECTOR" */
    /* HORIZ.5 */
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(UBAR0),
    ORDER(GOTO),
    ADDRESS(SRRETURN),
    /* HORIZ.6: the moon's radius, both axes. */
    ORDERS(DLOAD, PUSH),
    ADDRESS_PLUS_ONE(RADMOON),
    ORDERS(PUSH, GOTO),
    ADDRESS(HORIZ_1),
};

/* DIVIDE: called with the divisor in MPAC and the dividend in 34D. */
static const word divide_words[] = {
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X1), /* the divisor's shift */
    STODL_D(36),
    ADDRESS_PLUS_ONE_D(34),
    ORDERS(NORM, BDDV),
    ADDRESS_PLUS_ONE(S1), /* the dividend's shift */
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(XSU_1, RVQ),
    ADDRESS(S1), /* X1 less S1 */
};

/* RECT.1: "SR TO SET ZMEASURE = 0 IF MEASUREMENT" "PLANET AND PRIMARY
 * PLANET ARE THE SAME." ("OTHERWISE = 1"). */
static const word rect_1_words[] = {
    ORDERS(BOFF, AXT_2),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(RECT_3),
    MINUS_NUMBER(2), /* DEC -2 */
    /* "VEC. AND SCALE B29 AND B7" */
    ORDER(BOFF),
    SWITCH(BOF, LUNAFLAG),
    ADDRESS(RECT_4),
    /* RECT.2 */
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, ZMEASURE),
    ADDRESS(RECT_5),
    /* RECT.3 */
    ORDERS(AXT_2, BOFF),
    NUMBER(0),
    SWITCH(BOF, LUNAFLAG),
    ADDRESS(RECT_2),
    /* RECT.4 */
    ORDER(SET),
    SWITCH(SET, ZMEASURE),
    /* RECT.5 */
    ORDERS(VLOAD, VSR7),
    ADDRESS_PLUS_ONE(DELTACSM), /* "SCALED B22 OR B18" */
    ORDERS(VSR_STAR, VAD),
    SR_X2(0),
    ADDRESS_PLUS_ONE(RCVCSM), /* "SCALED B29 OR B27" */
    ORDER(VSR_STAR),
    SR_X2(0),
    STOVL(RZC),               /* "NOW SCALED B29" */
    ADDRESS_PLUS_ONE(NUVCSM), /* "SCALED B3 OR B-1" */
    ORDERS(VSR4, VSR_STAR),
    SR_X2(0),
    ORDERS(VAD, VSR_STAR),
    ADDRESS_PLUS_ONE(VCVCSM), /* "SCALED B7 OR B5" */
    SR_X2(0),
    STORE(VZC), /* "NOW SCALED B7" */
    ORDER(RVQ),
};

/* ---- The interpretive code ---- */

const struct interpretive_code HORIZ_CODE = {031u, HORIZ, COUNT(horiz_words), horiz_words};
const struct interpretive_code DIVIDE_CODE = {031u, DIVIDE, COUNT(divide_words), divide_words};
const struct interpretive_code RECT_1_CODE = {031u, RECT_1, COUNT(rect_1_words), rect_1_words};

#endif
