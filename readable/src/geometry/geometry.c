/* SPDX-License-Identifier: MIT */
/* LEM_GEOMETRY.agc (Luminary099 pages 320-325): its interpretive code in
 * the listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs (src/binding/geometry_routines.c), and RRANGLES's basic
 * code. The faithful C is c64-port/src/lm_geometry.c. */
#include "geometry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* SVDWN2, SVDWN1 -- LM pp. 320-321.
 * "THESE TWO ROUTINES COMPUTE THE ACTUAL STATE VECTOR FOR LM,CSM BY ADDING
 * THE CONIC R,V AND THE DEVIATIONS R,V. THE STATE VECTORS ARE CONVERTED TO
 * METERS B-29 AND METERS/CSEC B-7 AND STORED APPROPRIATELY IN RN,VN OR
 * R-OTHER , V-OTHER FOR DOWNLINK. THE ROUTINES NAMES ARE SWITCHED IN THE
 * OTHER VEHICLES COMPUTER."
 *
 * The state vector is the integration's, in its temporary storage: the
 * deviations TDELTAV, TNUV and the conic RCV, VCV. X2 says how they are
 * scaled: "IF STATE VECTOR IS SCALED POS B27 AND VEL B5 SET X2 TO +2", "IF
 * ... POS B29 AND VEL B7 SET X2 TO 0". The shifts (VSL*, indexed by X2)
 * take both scalings to B-29 and B-7.
 *
 * SVDWN2 is this vehicle's: "R(T) IN RN, V(T) IN VN, T IN PIPTIME". It
 * does nothing while AVEMIDSW is set ("SW=1=AVETOMID DOING W-MATRIX
 * INTEG."). SVDWN1 is the other vehicle's: "R(T) IN R-OTHER, V(T) IN
 * V-OTHER (T IS DEFINED BY T-OTHER)". */
static const word svdwn[] = {
    /* SVDWN2 */ ORDERS(BOF, RVQ), /* "SW=1=AVETOMID DOING W-MATRIX INTEG." */
    SWITCH(BOF, AVEMIDSW),
    ADDRESS_AT(SVDWN2, 3), /* +1 */
    ORDERS(VLOAD, VSL_STAR),
    ADDRESS_PLUS_ONE(TDELTAV),
    VSL_X2(-7), /* 0 -7,2 */
    ORDERS(VAD, VSL_STAR),
    ADDRESS_PLUS_ONE(RCV),
    VSL_X2(0), /* 0,2 */
    STOVL(RN),
    ADDRESS_PLUS_ONE(TNUV),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-4), /* 0 -4,2 */
    ADDRESS_PLUS_ONE(VCV),
    ORDER(VSL_STAR),
    VSL_X2(0), /* 0,2 */
    STODL(VN),
    ADDRESS_PLUS_ONE(TET),
    STORE(PIPTIME),
    ORDER(RVQ),
    /* SVDWN1 */ ORDERS(VLOAD, VSL_STAR),
    ADDRESS_PLUS_ONE(TDELTAV),
    VSL_X2(-7), /* 0 -7,2 */
    ORDERS(VAD, VSL_STAR),
    ADDRESS_PLUS_ONE(RCV),
    VSL_X2(0), /* 0,2 */
    STOVL(R_OTHER),
    ADDRESS_PLUS_ONE(TNUV),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-4), /* 0 -4,2 */
    ADDRESS_PLUS_ONE(VCV),
    ORDER(VSL_STAR),
    VSL_X2(0), /* 0,2 */
    STORE(V_OTHER),
    ORDER(RVQ),
};

/* RRANGLES -- LM pp. 322-323.
 * "THE FOLLOWING ROUTINE TAKES A HALF UNIT TARGET VECTOR REFERRED TO NAV
 * BASE COORDINATES AND FINDS BOTH GIMBAL ORIENTATIONS AT WHICH THE RR MIGHT
 * SIGHT THE TARGET. THE GIMBAL ANGLES CORRESPONDING TO THE PRESENT MODE ARE
 * LEFT IN MODEA AND THOSE WHICH WOULD BE USED AFTER A REMODE IN MODEB."
 * Mode 1 has the trunnion angle under 90 degrees in size, the shaft angle
 * anything; "MODE SELECTION AND LIMIT CHECKING ARE DONE ELSEWHERE."
 *
 * "THE MODE 1 CONFIGURATION IS CALCULATED FROM THE VECTOR AND THEN MODE 2
 * IS FOUND USING THE RELATIONS S(2) = 180 + S(1), T(2) = 180 - T(1)." The
 * trunnion is the arcsine of the vector's Y component; the shaft comes from
 * the vector's projection into the X-Z plane, through ARCTRIG ("SINCE
 * SHAFT COULD BE ARB."). A vector along Y has no such projection (UNIT
 * overflows): LUNDESCH then asks for a maneuver "UNLESS ON LUNAR SURF".
 * RTB 2V1STO2S turns each pair into CDU angles. "THE VECTOR ARRIVES IN
 * MPAC WHERE TRG*SMNB OR *SMNB* WILL HAVE LEFT IT." */
static const word rrangles[] = {
    /* RRANGLES */ STORE_D(32),
    ORDERS(DLOAD, DCOMP),       /* "SINCE WE WILL FIND THE MODE 1 SHAFT" */
    ADDRESS_PLUS_ONE_D(34),     /* "ANGLE LATER, WE CAN FIND THE MODE 1" */
    ORDERS(SETPD, ASIN),        /* "TRUNNION BY SIMPLY TAKING THE ARCSIN OF" */
    ADDRESS_PLUS_ONE_D(0),      /* "THE Y COMPONENT, THE ASIN GIVIN AN" */
    ORDERS(PUSH, BDSU),         /* "ANSWER WHOSE ABS VAL IS LESS THAN 90 DEG" */
    ADDRESS_PLUS_ONE(LODPHALF), /* (180 degrees) */
    STODL_D(4),                 /* "MODE 2 TRUNNION TO 4." */
    ADDRESS_PLUS_ONE(LO6ZEROS),
    STOVL_D(34),            /* "UNIT THE PROJECTION OF THE VECTOR" */
    ADDRESS_PLUS_ONE_D(32), /* "IN THE X-Z PLANE" */
    ORDERS(UNIT, BOVB),     /* "IF OVERFLOW,TARGET VECTOR IS ALONG Y" */
    ADDRESS(LUNDESCH),      /* "CALL FOR MANEUVER UNLESS ON LUNAR SURF" */
    STODL_D(32),            /* "PROJECTION VECTOR." */
    ADDRESS_PLUS_ONE_D(32),
    ORDERS(SR1, STQ), /* (the return, which the CALL below replaces, to S2) */
    ADDRESS(S2),
    STODL(SINTH), /* "USE ARCTRIG SINCE SHAFT COULD BE ARB." */
    ADDRESS_PLUS_ONE_D(36),
    ORDER(SR1),
    STCALL(COSTH),
    ADDRESS(ARCTRIG),
    ORDERS(PUSH, DAD), /* "MODE 1 SHAFT TO 2." */
    ADDRESS_PLUS_ONE(LODPHALF),
    STOVL_D(6),
    ADDRESS_PLUS_ONE_D(4),
    ORDER(RTB), /* "FIND MODE 2 CDU ANGLES." */
    ADDRESS(2V1STO2S),
    STOVL(MODEB),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(RTB), /* "MODE 1 ANGLES TO MODE A." */
    ADDRESS(2V1STO2S),
    STORE(MODEA),
    ORDER(EXIT),
};

/* RRANGLES +042 -- LM p. 323. After the EXIT: "SWAP MODEA AND MODEB IF
 * RR IN MODE 2" (ANTENBIT of RADMODES set). CS and MASK leave +0 then, so
 * the CCS goes to the three DXCHes, which swap the two pairs; otherwise TCF
 * +4 passes them. Then back to the interpreter (TC INTPRET), for the GOTO
 * after it. */
void rrangles_after_exit(void)
{
    cs(ADDRESS_OF(RADMODES)); /* "SWAP MODEA AND MODEB IF RR IN MODE 2." */
    mask(ANTENBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: break;                       /* TCF +4 */
    case IS_PLUS_ZERO: dxch(ADDRESS_OF(MODEA)); /* fall through */
    case NEGATIVE: dxch(ADDRESS_OF(MODEB));     /* fall through */
    case IS_MINUS_ZERO: dxch(ADDRESS_OF(MODEA)); break;
    }
    tc_at(RRANGLES + 051u);
    intpret();
}

/* RRANGLES +052 -- LM p. 323: back to RRANGLES's caller, whose return the
 * STQ kept in S2. */
static const word rrangles_goto[] = {
    ORDER(GOTO),
    ADDRESS(S2),
};

/* RRNB, RRNB1, RRNBMPAC -- LM p. 324.
 * "GIVEN RR TRUNNION AND SHAFT (T,S) IN TANGNB,+1,FIND THE ASSOCIATED LINE
 * OF SIGHT IN NAV BASE AXES. THE HALF UNIT VECTOR, .5(SIN(S)COS(T),
 * -SIN(T),COS(S)COS(T)) IS LEFT IN MPAC AND 32D." Each CDU angle goes
 * through RTB CDULOGIC to double precision first. RRNBMPAC is "THIS
 * ENTRY TO RRNB", which "REQUIRES THE TRUNNION AND SHAFT ANGLES IN MPAC AND
 * MPAC +1 RESPECTIVELY." */
static const word rrnb[] = {
    /* RRNB */ ORDERS(SLOAD, RTB),
    ADDRESS_PLUS_ONE(TANGNB),
    ADDRESS(CDULOGIC),
    ORDERS(SETPD, PUSH), /* "TRUNNION ANGLE TO 0" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SIN, DCOMP),
    STODL_D(34),       /* "Y COMPONENT" */
    ORDERS(COS, PUSH), /* ".5 COS(T) TO 0" */
    ORDERS(SLOAD, RTB),
    ADDRESS_PLUS_ONE_AT(TANGNB, 1),
    ADDRESS(CDULOGIC),
    /* RRNB1 */ ORDERS(PUSH, COS), /* "SHAFT ANGLE TO 2" */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(0),
    STODL_D(36), /* "Z COMPONENT" */
    ORDERS(SIN, DMP),
    ORDER(SL1),
    STOVL_D(32),
    ADDRESS_PLUS_ONE_D(32),
    ORDER(RVQ),
    /* RRNBMPAC */ STODL_D(20), /* "SAVE SHAFT CDU IN 21." */
    ADDRESS_PLUS_ONE(MPAC),     /* "SET MODE TO DP." */
    ORDERS(RTB, SETPD),
    ADDRESS(CDULOGIC),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PUSH, SIN), /* "TRUNNION ANGLE TO 0" */
    ORDER(DCOMP),
    STODL_D(34),        /* "Y COMPONENT" */
    ORDERS(COS, PUSH),  /* ".5COS(T) TO 0" */
    ORDERS(SLOAD, RTB), /* "PICK UP CDU'S." */
    ADDRESS_PLUS_ONE_D(21),
    ADDRESS(CDULOGIC),
    ORDER(GOTO),
    ADDRESS(RRNB1),
};

const struct interpretive_code SVDWN_CODE = {013u, 02070u, COUNT(svdwn), svdwn};
const struct interpretive_code RRANGLES_CODE = {013u, 02133u, COUNT(rrangles), rrangles};
const struct interpretive_code RRANGLES_GOTO_CODE = {013u, 02205u, COUNT(rrangles_goto),
                                                     rrangles_goto};
const struct interpretive_code RRNB_CODE = {023u, 02041u, COUNT(rrnb), rrnb};

#endif
