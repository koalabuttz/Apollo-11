/* SPDX-License-Identifier: MIT */
/* LEM_GEOMETRY.agc (Luminary099 pages 320-325) and CSM_GEOMETRY.agc
 * (Comanche055 pages 285-296): their interpretive code in the listing's
 * notation (src/interpreter/notation.h), which the readable interpreter
 * runs (src/binding/geometry_routines.c), and the basic code among it. The
 * faithful C is c64-port/src/lm_geometry.c and cm_geometry.c. */
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

#else

/* SXTNB -- CM p. 285.
 * "THIS ROUTINE TAKES THE SHAFT AND TRUNNION ANGLES AS READ BY THE CM
 * OPTICAL SYSTEM AND CONVERTS THEM INTO A UNIT VECTOR REFERENCED TO THE
 * NAVIGATION BASE COORDINATE SYSTEM AND COINCIDENT WITH THE SEXTANT LINE
 * OF SIGHT." The angles are a mark's: "THE SEXTANT SHAFT AND TRUNNION
 * ANGLES ARE STORED SP IN LOCATIONS 3 AND 5 RESPECTIVELY OF THE MARK VAC
 * AREA", and X1 holds "THE COMPLEMENT OF THE BASE ADDRESS OF THE MARK VAC
 * AREA", so "3,1" and "5,1" read them. Each CDU angle goes through RTB
 * CDULOGIC to double precision; the trunnion's also through SXTLOGIC
 * below, for the sextant's offset. The line of sight, (COS(SA)SIN(TA),
 * SIN(SA)SIN(TA), COS(TA)) in the sextant's axes (STARM), goes to the
 * navigation base through the fixed matrix NB1NB2. "THE OUTPUT IS A
 * HALF-UNIT VECTOR IN NAVIGATION BASE COORDINATES AND STORED AT LOCATION
 * 32D OF THE VAC AREA. THE OUTPUT IS ALSO AVAILABLE AT MPAC." */
static const word sxtnb[] = {
    /* SXTNB */ ORDERS(SLOAD_STAR, RTB), /* "PUSHDOWN 00,02,04,(17D-19D),32D-36D" */
    ADDRESS_PLUS_ONE_X1(5),              /* "TRUNNION = TA" */
    ADDRESS(CDULOGIC),
    ORDERS(RTB, PUSH),
    ADDRESS(SXTLOGIC),
    ORDERS(SIN, SL1),
    ORDERS(PUSH, SLOAD_STAR), /* "PD2 = SIN(TA)" */
    ADDRESS_PLUS_ONE_X1(3),   /* "SHAFT = SA" */
    ORDERS(RTB, PUSH),        /* "PD4 = SA" */
    ADDRESS(CDULOGIC),
    ORDERS(COS, DMP),
    ADDRESS_PLUS_ONE_D(2),
    STODL(STARM), /* "COS(SA)SIN(TA)" */
    ORDERS(SIN, DMP),
    ORDER(STADR),
    AFTER_STADR(STODL_AT(STARM, 2)), /* "SIN(SA)SIN(TA)" */
    ORDER(COS),
    STOVL_AT(STARM, 4),
    ADDRESS_PLUS_ONE(STARM), /* "STARM = 32D" */
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(NB1NB2),
    STORE_D(32),
    ORDER(RVQ),
};

/* SXTLOGIC -- CM p. 285. The RTB routine SXTNB runs on the trunnion
 * angle, which CDULOGIC has made double precision, in revolutions: "CORRECT
 * FOR 19.775 DEGREE OFFSET" (10DEGS- added in), then times QUARTER
 * (SHORTMP), and back to DANZIG. */
void sxtlogic(void)
{
    set_A(TEN_DEGS_MINUS); /* CAF 10DEGS- */
    ads(ADDRESS_OF(MPAC));
    set_A(QUARTER); /* CAF QUARTER */
    tc_at(SXTLOGIC + 3u);
    shortmp();
    tc_at(SXTLOGIC + 4u);
    continue_at(DANZIG); /* TC DANZIG */
}

/* CALCSXA, SXTANG, SXTANG1, SXTALARM, ZNB=S1 -- CM pp. 286-288.
 * "CALCSXA COMPUTES THE SEXTANT SHAFT AND TRUNNION ANGLES REQUIRED TO
 * POSITION THE OPTICS SUCH THAT A STAR LINE-OF-SIGHT LIES ALONG THE STAR
 * VECTOR. THE ROUTINE TAKES THE GIVEN STAR VECTOR AND EXPRESSES IT AS A
 * VECTOR REFERENCED TO THE OPTICS COORDINATE SYSTEM. IN ADDITION IT SETS UP
 * THREE UNIT VECTORS DEFINING THE X,Y, AND Z AXES REFERENCED TO THE OPTICS
 * COORDINATE SYSTEM." The star vector STAR is in stable member axes, and
 * *SMNB* turns it to navigation base axes (from "SINES AND COSINES OF THE
 * CDU ANGLES ... A CALL TO CDUTRIG WILL PROVIDE THIS INPUT").
 *
 * SXTANG does the same from "THE STAR VECTOR REFERRED TO ANY COORDINATE
 * SYSTEM" and "THE NAVIGATION BASE COORDINATES REFERRED TO THE SAME
 * COORDINATE SYSTEM ... STORED AT XNB, YNB, AND ZNB", which TRANSP1
 * transposes into the matrix from that system to navigation base axes
 * ("EREF WRT NB2"), and back after.
 *
 * Both then go on at SXTANG1, which finds the angles. The shaft angle is
 * the direction of the star's projection across the sextant's Z axis
 * (UNIT(ZNB X S)), from its sine and cosine through ARCTRIG; the trunnion
 * from the angle between the star and that axis, through ACOS, less
 * 20DEG- ("TRUNNION ANGLE NEGATIVE" and "TRUNNION ANGLE GREATER THAN 90
 * DEGREES" go to SXTALARM). RTB 1STO2S makes each a CDU angle.
 * "THE OUTPUTS ARE THE SEXTANT SHAFT AND TRUNNION ANGLES STORED DP AT SAC
 * AND PAC RESPECTIVELY. (LOW ORDER PART EQUAL TO ZERO)." A star along
 * the Z axis has no projection (UNIT overflows): ZNB=S1 then gives the
 * shaft 270 degrees and the trunnion 20DEGS-. Each way back clears or
 * sets CULTFLAG and returns through 28D, where ITA kept the return:
 * "ALARM HAS BEEN REMOVED FROM THIS SUBROUTINE,ALARM WILL BE SET BY MPI"
 * (CULTFLAG set). */
static const word calcsxa[] = {
    /* CALCSXA */ ORDERS(ITA, VLOAD), /* "PUSHDOWN 00-26D,28D,30D,32D-36D" */
    ADDRESS_D(28),
    ADDRESS_PLUS_ONE(STAR),
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR), /* *SMNB* */
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(NB2NB1),
    STOVL(STAR),
    ADDRESS_PLUS_ONE(HIUNITX),
    STOVL(XNB1),
    ADDRESS_PLUS_ONE(HIUNITY),
    STOVL(YNB1),
    ADDRESS_PLUS_ONE(HIUNITZ),
    STCALL(ZNB1),
    ADDRESS(SXTANG1),
    /* SXTANG */ ORDERS(ITA, RTB), /* "PUSHDOWN 16D,18D,22D-26D,28D" */
    ADDRESS_D(28),
    ADDRESS(TRANSP1), /* "EREF WRT NB2" */
    ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(XNB),
    ADDRESS_PLUS_ONE(NB2NB1),
    ORDER(VSL1),
    STOVL(XNB1),
    ADDRESS_PLUS_ONE(YNB),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(NB2NB1),
    STOVL(YNB1),
    ADDRESS_PLUS_ONE(ZNB),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(NB2NB1),
    STORE(ZNB1),
    ORDERS(RTB, RTB),
    ADDRESS(TRANSP1),
    ADDRESS(TRANSP2),
    /* SXTANG1 */ ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(ZNB1),
    ADDRESS_PLUS_ONE(STAR),
    ORDER(BOV),
    ADDRESS_AT(SXTANG1, 5), /* +1 */
    ORDERS(UNIT, BOV),
    ADDRESS(ZNB_EQ_S1), /* ZNB=S1 */
    STORE(PDA),         /* "PDA = UNIT(ZNB X S)" */
    ORDERS(DOT, DCOMP),
    ADDRESS_PLUS_ONE(XNB1),
    STOVL(SINTH), /* "SIN(SA) = PDA . -XNB" */
    ADDRESS_PLUS_ONE(PDA),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(YNB1),
    STCALL(COSTH), /* "COS(SA) = PDA . YNB" */
    ADDRESS(ARCTRIG),
    ORDER(RTB),
    ADDRESS(1STO2S),
    STOVL(SAC),
    ADDRESS_PLUS_ONE(STAR),
    ORDER(BOV),
    ADDRESS_AT(SXTANG1, 026), /* +1 */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(ZNB1),
    ORDER(ACOS),
    ORDERS(BMN, SL2),
    ADDRESS(SXTALARM), /* "TRUNNION ANGLE NEGATIVE" */
    ORDERS(BOV, DSU),
    ADDRESS(SXTALARM),        /* "TRUNNION ANGLE GREATER THAN 90 DEGREES" */
    ADDRESS_PLUS_ONE(20DEGM), /* 20DEG- */
    ORDER(RTB),
    ADDRESS(1STO2S),
    STORE(PAC),   /* "FOR FLIGHT USE, CULTFLAG IS ON IF" */
    ORDER(CLRGO), /* "TRUNION IS GREATER THAN 90 DEG" */
    SWITCH(CLRGO, CULTFLAG),
    ADDRESS_D(28),
    /* SXTALARM */ ORDER(SETGO), /* "ALARM HAS BEEN REMOVED FROM THIS" */
    SWITCH(SETGO, CULTFLAG),
    ADDRESS_D(28), /* "SUBROUTINE,ALARM WILL BE SET BY MPI" */
    /* ZNB=S1 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(270DEG),
    STODL(SAC),
    ADDRESS_PLUS_ONE(20DEGSM), /* 20DEGS- */
    STORE(PAC),
    ORDER(CLRGO),
    SWITCH(CLRGO, CULTFLAG),
    ADDRESS_D(28),
};

/* LOG -- CM p. 291.
 * "SUBROUTINE TO COMPUTE THE NATURAL LOG OF C(MPAC, MPAC +1)", "RETURNS
 * WITH -LOG IN DP MPAC". NORM "GENERATES LOG BY SHIFTING ARG UNTIL IT
 * LIES BETWEEN .5 AND 1." (the shift count to MPAC +3); "THE LOG OF THIS
 * PART IS FOUND AND THE LOG OF THE SHIFTED PART IS COMPUTED AND ADDED
 * IN": the first by a polynomial in NEARLY1 less it, in basic code after
 * the EXIT; the second as the shift count times CLOG2/32. */
static const word log_[] = {
    /* LOG */ ORDERS(NORM, BDSU), /* "GENERATES LOG BY SHIFTING ARG" */
    ADDRESS_PLUS_ONE_AT(MPAC, 3), /* "UNTIL IT LIES BETWEEN .5 AND 1." */
    ADDRESS_PLUS_ONE(NEARLY1),    /* "THE LOG OF THIS PART IS FOUND AND THE" */
    ORDER(EXIT),                  /* "LOG OF THE SHIFTED PART IS COMPUTED" */
};

/* LOG +4 -- CM p. 291. After the EXIT, the polynomial: TC POLY, whose
 * degree and coefficients are the words after the TC ("DEC 2", "(N-1,
 * SUPPLIED BY SMERZH)"); POLY returns past them, to LOG +016. */
void log_after_exit(void)
{
    tc_at(LOG + 4u);
    continue_at(POLY); /* TC POLY "AND ADDED IN. SHIFT COUNT STORED" */
}

/* LOG +016 -- CM p. 291. The polynomial's value in MPAC (its third word
 * cleared), the shift count, kept in MPAC +3 by NORM, times CLOG2/32 (the
 * log of 2, scaled by 32) added in, and back to the interpreter, for the
 * RVQ after. COM makes the count, which NORM leaves negative, positive;
 * SHORTMP multiplies MPAC's DP number by it, so the DXCHes first swap the
 * polynomial's value out to MPAC +3 and CLOG2/32 into MPAC, and after it
 * the product to A,L and the value back, for the DAS. */
void log_after_poly(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(MPAC_AT(2u));
    set_A(CLOG2_32[0]); /* EXTEND; DCA CLOG2/32 */
    set_L(CLOG2_32[1]);
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(3u));
    com(); /* "LOAD POSITIVE SHIFT COUNT IN A." */
    tc_at(LOG + 025u);
    shortmp(); /* "MULTIPLY BY SHIFT COUNT." */
    dxch(MPAC_AT(1u));
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(3u));
    das(MPAC_AT(0u));
    tc_at(LOG + 032u);
    intpret(); /* TC INTPRET "RESULT IN MPAC, MPAC +1" */
}

/* LOG +033 -- CM p. 291: the RVQ after TC INTPRET. */
static const word log_rvq[] = {
    ORDER(RVQ),
};

/* EARROT1, EARROT2, OVERADAY -- CM pp. 293-294.
 * The "EARTH ROTATOR": "THIS ROUTINE PROJECTS THE INITIAL EARTH TARGET
 * VECTOR RTINIT AHEAD THROUGH THE ESTIMATED TIME OF FLIGHT. INITIAL CALL
 * RESOLVES THE INITIAL TARGET VECTOR RTINIT INTO EASTERLY AND NORMAL
 * COMPONENTS RTEAST AND RTNORM. INITIAL AND SUBSEQUENT CALLS ROTATE THIS
 * VECTOR ABOUT THE (FULL) UNIT POLAR AXIS UNITW THROUGH THE ANGLE WIE
 * DTEAROT TO OBTAIN THE ROTATED TARGET VECTOR RT":
 *
 *     RT = RTINIT + RTNORM (COS(WT) - 1) + RTEAST SIN(WT),  WT = WIE DTEAROT
 *
 * with RTEAST = UNITW * RTINIT and RTNORM = RTEAST * UNITW (cross
 * products). "FOR CONTINUOUS UPDATING, ONLY ONE ENTRY TO EARROT1 IS
 * REQUIRED, WITH SUBSEQUENT ENTRIES AT EARROT2", each with the time of
 * flight DTEAROT in MPAC. 1/WIE is a sidereal day in centiseconds: a time over a day
 * (DDV overflows) goes to OVERADAY, which takes a day off DTEAROT (toward
 * zero) and tries again. The BOVB first resets OVFIND ("RESET OVFIND, IF
 * ON"), through TCDANZIG. */
static const word earrot[] = {
    /* EARROT1 */ ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(UNITW),  /* "FULL UNIT VECTOR" */
    ADDRESS_PLUS_ONE(RTINIT), /* ".5 UNIT" */
    STORE(RTEAST),            /* ".5 UNIT" */
    ORDER(VXV),
    ADDRESS_PLUS_ONE(UNITW),   /* "FULL UNIT" */
    STODL(RTNORM),             /* ".5 UNIT" */
    ADDRESS_PLUS_ONE(DTEAROT), /* "(-28) CS" */
    /* EARROT2 */ ORDERS(BOVB, DDV),
    ADDRESS(TCDANZIG),       /* "RESET OVFIND, IF ON" */
    ADDRESS_PLUS_ONE(1_WIE), /* 1/WIE */
    ORDERS(BOV, PUSH),
    ADDRESS(OVERADAY),
    ORDERS(COS, DSU),
    ADDRESS_PLUS_ONE(HIDPHALF),
    ORDERS(VXSC, PDDL),       /* "XCH W PUSH LIST" */
    ADDRESS_PLUS_ONE(RTNORM), /* ".5 UNIT" */
    ORDERS(SIN, VXSC),
    ADDRESS_PLUS_ONE(RTEAST), /* ".5 UNIT" */
    ORDERS(VAD, VSL1),
    ORDERS(VAD, UNIT),        /* "INSURE THAT RT IS 'UNIT'." */
    ADDRESS_PLUS_ONE(RTINIT), /* ".5 UNIT" */
    STORE(RT),                /* ".5 UNIT TARGET VECTOR" */
    ORDER(RVQ),
    /* OVERADAY */ ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE(1_WIE), /* 1/WIE */
    ADDRESS_PLUS_ONE(DTEAROT),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(DTEAROT),
    STORE(DTEAROT),
    ORDER(GOTO),
    ADDRESS(EARROT2),
};

/* SVDWN1, SVDWN2 -- CM pp. 289-290.
 * The LM's SVDWN2 and SVDWN1 (above), their names switched: "THE ROUTINES
 * NAMES ARE SWITCHED IN THE OTHER VEHICLES COMPUTER." SVDWN1 is this
 * vehicle's, "R(T) IN RN, V(T) IN VN, T IN PIPTIME", doing nothing while
 * AVEMIDSW is set ("SW=1=AVETOMID DOING W-MATRIX INTEG"); SVDWN2 the other
 * vehicle's, "R(T) IN R-OTHER, V(T) IN V-OTHER". */
static const word svdwn[] = {
    /* SVDWN1 */ ORDERS(BOF, RVQ), /* "SW=1=AVETOMID DOING W-MATRIX INTEG" */
    SWITCH(BOF, AVEMIDSW),
    ADDRESS_AT(SVDWN1, 3), /* +1 */
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
    /* SVDWN2 */ ORDERS(VLOAD, VSL_STAR),
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

const struct interpretive_code SXTNB_CODE = {023u, 02000u, COUNT(sxtnb), sxtnb};
const struct interpretive_code CALCSXA_CODE = {023u, 02034u, COUNT(calcsxa), calcsxa};
const struct interpretive_code LOG_CODE = {023u, 02155u, COUNT(log_), log_};
const struct interpretive_code LOG_RVQ_CODE = {023u, 02210u, COUNT(log_rvq), log_rvq};
const struct interpretive_code EARROT_CODE = {023u, 02215u, COUNT(earrot), earrot};
const struct interpretive_code SVDWN_CODE = {010u, 02237u, COUNT(svdwn), svdwn};

#endif
