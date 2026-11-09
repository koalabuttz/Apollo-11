/* SPDX-License-Identifier: MIT */
/* INFLIGHT_ALIGNMENT_ROUTINES.agc, both missions' (Luminary099 pp.
 * 1249-1257, Comanche055 pp. 1355-1363), in bank 23. The interpretive
 * words are in the listing's notation (src/interpreter/notation.h); the
 * basic code after GIMLOCK1's EXIT is C.
 *
 * The routines that turn the IMU's stable member to a new orientation
 * (the alignment programs, P51 and P52, and the IMU's coarse and fine
 * alignment):
 *
 * "CALCGTA COMPUTES THE GYRO TORQUE ANGLES REQUIRED TO BRING THE STABLE
 * MEMBER INTO THE DESIRED ORIENTATION." From the desired axes in the
 * present stable member's frame (XDC, YDC and ZDC, half-unit vectors), the
 * angles for the Y, Z and X gyros, into IGC, MGC and OGC.
 *
 * "ARCTRIG COMPUTES AN ANGLE GIVEN THE SINE AND COSINE OF THIS ANGLE."
 * From SINTH and COSTH (each over 4), the angle in revolutions into
 * THETA: by the arcsine where the sine is the smaller (within 45 degrees
 * of 0 or of 180), otherwise by the arccosine with the sine's sign.
 *
 * "CALCGA COMPUTES THE CDU DRIVING ANGLES REQUIRED TO BRING THE STABLE
 * MEMBER INTO THE DESIRED ORIENTATION." From the navigation base's axes
 * (XNB, YNB, ZNB) and the desired stable member's (XSM, YSM, ZSM), in the
 * same frame, the outer, middle and inner gimbal angles, into THETAD (as
 * the CDUs count, by V1STO2S). A middle gimbal angle over 60 degrees is
 * gimbal lock: alarm 401 and GLOKFAIL set (GIMLOCK1), and on.
 *
 * "AXISGEN COMPUTES THE COORDINATES OF ONE COORDINATE SYSTEM REFERRED TO
 * ANOTHER COORDINATE SYSTEM." From two stars in each frame (STARAD and
 * STARAD +6 in frame A; the push-down list's 6D and 12D in frame B), a
 * triad in each (the first star, the unit of the two's cross product, and
 * the cross product of those), and from the two triads frame A's axes in
 * frame B, into XDC, YDC and ZDC and STARAD, STARAD +6 and STARAD +12D.
 * The binding is src/binding/inflight_align_routines.c. */
#include "inflight_align.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

static const word calcgta_words[] = {
    /* CALCGTA -- LM p. 1249 (CM p. 1355): the return into S2; ZPRIME the
     * unit of (-XD3, 0, XD1). */
    /* CALCGTA */ ORDERS(ITA, DLOAD),
    ADDRESS(S2),
    ADDRESS_PLUS_ONE(XDC),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ADDRESS_PLUS_ONE_AT(XDC, 4),
    ORDERS(DCOMP, VDEF),
    ORDER(UNIT),
    STODL(ZPRIME), /* "ZP = UNIT(-XD3 0 XD1) = (ZP1 ZP2 ZP3)" */
    ADDRESS_PLUS_ONE(ZPRIME),
    /* The Y gyro's angle, IGC, from ZP1 and ZP3. */
    ORDER(SR1),
    STODL(SINTH), /* "SIN(IGC) = ZP1" */
    ADDRESS_PLUS_ONE_AT(ZPRIME, 4),
    ORDER(SR1),
    STCALL(COSTH), /* "COS(IGC) = ZP3" */
    ADDRESS(ARCTRIG),
    STODL(IGC), /* "Y GYRO TORQUING ANGLE   FRACTION OF REV." */
    ADDRESS_PLUS_ONE_AT(XDC, 2),
    /* The Z gyro's angle, MGC: the sine XD2, the cosine
     * (ZP3)(XD1) - (ZP1)(XD3). */
    ORDER(SR1),
    STODL(SINTH), /* "SIN(MGC) = XD2" */
    ADDRESS_PLUS_ONE(ZPRIME),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE_AT(XDC, 4), /* "PD00 = (ZP1)(XD3)" */
    ADDRESS_PLUS_ONE_AT(ZPRIME, 4),
    ORDERS(DMP, DSU),
    ADDRESS_PLUS_ONE(XDC), /* "MPAC = (ZP3)(XD1)" */
    ORDER(STADR),
    AFTER_STADR(STCALL(COSTH)), /* "COS(MGC) = MPAC - PD00" */
    ADDRESS(ARCTRIG),
    /* LM p. 1250 (CM p. 1356): the X gyro's angle, OGC, from ZP . ZDC
     * and ZP . YDC; and back by S2. */
    STOVL(MGC), /* "Z GYRO TORQUING ANGLE   FRACTION OF REV." */
    ADDRESS_PLUS_ONE(ZPRIME),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(ZDC),
    STOVL(COSTH), /* "COS(OGC) = ZP . ZDC" */
    ADDRESS_PLUS_ONE(ZPRIME),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(YDC),
    STCALL(SINTH), /* "SIN(OGC) = ZP . YDC" */
    ADDRESS(ARCTRIG),
    STCALL(OGC), /* "X GYRO TORQUING ANGLE   FRACTION OF REV." */
    ADDRESS(S2),
    /* ARCTRIG -- LM p. 1251 (CM p. 1357): within 45 degrees of 0 or 180,
     * TRIG1; otherwise the arccosine with the sine's sign. */
    /* ARCTRIG */ ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(SINTH),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(QTSN45), /* "ABS(SIN/4) - SIN(45)/4" */
    ADDRESS(TRIG1),           /* "IF (-45,45) OR (135,-135)" */
    ORDERS(DLOAD, SL1),       /* "(45,135) OR (-135,-45)" */
    ADDRESS_PLUS_ONE(COSTH),
    ORDERS(ACOS, SIGN),
    ADDRESS_PLUS_ONE(SINTH),
    STORE(THETA), /* "X = ARCCOS(COS) WITH SIGN(SIN)" */
    ORDER(RVQ),
    /* TRIG1: the arcsine; with the cosine negative (TRIG2), half a
     * revolution with the sine's sign, less the arcsine. */
    /* TRIG1 */ ORDERS(DLOAD, SL1), /* "(-45,45) OR (135,-135)" */
    ADDRESS_PLUS_ONE(SINTH),
    ORDER(ASIN),
    STODL(THETA), /* "X = ARCSIN(SIN) WITH SIGN(SIN)" */
    ADDRESS_PLUS_ONE(COSTH),
    ORDER(BMN),
    ADDRESS(TRIG2), /* "IF (135,-135)" */
    ORDERS(DLOAD, RVQ),
    ADDRESS_PLUS_ONE(THETA),         /* "X = ARCSIN(SIN)   (-45,45)" */
    /* TRIG2 */ ORDERS(DLOAD, SIGN), /* "(135,-135)" */
    ADDRESS_PLUS_ONE(HIDPHALF),
    ADDRESS_PLUS_ONE(SINTH),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(THETA),
    STORE(THETA), /* "X = .5 WITH SIGN(SIN) - ARCSIN(SIN)" */
    ORDER(RVQ),
    /* CALCGA -- LM p. 1253 (CM p. 1359): the middle gimbal axis, PD0, the
     * unit of the outer's (XNB) cross the inner's (YSM); the return into
     * S2; the outer gimbal's angle, OGC. */
    /* CALCGA */ ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(XNB), /* "XNB = OGA (OUTER GIMBAL AXIS)" */
    ADDRESS_PLUS_ONE(YSM), /* "YSM = IGA (INNER GIMBAL AXIS)" */
    ORDERS(UNIT, PUSH),    /* "PD0 = UNIT(OGA X IGA) = MGA" */
    ORDERS(DOT, ITA),
    ADDRESS_PLUS_ONE(ZNB),
    ADDRESS(S2),
    STOVL(COSTH), /* "COS(OG) = MGA . ZNB" */
    ADDRESS_PLUS_ONE_D(0),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(YNB),
    STCALL(SINTH), /* "SIN(OG) = MGA . YNB" */
    ADDRESS(ARCTRIG),
    STOVL(OGC),
    ADDRESS_PLUS_ONE_D(0),
    /* The middle gimbal's angle, MGC; over 60 degrees, GIMLOCK1. */
    ORDERS(VXV, DOT), /* "PROVISION FOR MG ANGLE OF 90 DEGREES" */
    ADDRESS_PLUS_ONE(XNB),
    ADDRESS_PLUS_ONE(YSM),
    ORDER(SL1),
    STOVL(COSTH), /* "COS(MG) = IGA . (MGA X OGA)" */
    ADDRESS_PLUS_ONE(YSM),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(XNB),
    STCALL(SINTH), /* "SIN(MG) = IGA . OGA" */
    ADDRESS(ARCTRIG),
    STORE(MGC),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(_166___),
    ORDER(BPL),
    ADDRESS(GIMLOCK1), /* "IF ANGLE GREATER THAN 60 DEGREES" */

    /* CALCGA1: the inner gimbal's angle, IGC; the three angles as the
     * CDUs count them (V1STO2S) into THETAD; back by S2. */
    /* CALCGA1 */ ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(ZSM),
    ADDRESS_PLUS_ONE_D(0),
    STOVL(COSTH), /* "COS(IG) = ZSM . MGA" */
    ADDRESS_PLUS_ONE(XSM),
    /* LM p. 1254 (CM p. 1360) */
    ORDERS(DOT, STADR),
    AFTER_STADR(STCALL(SINTH)), /* "SIN(IG) = XSM . MGA" */
    ADDRESS(ARCTRIG),
    STOVL(IGC),
    ADDRESS_PLUS_ONE(OGC),
#if LUMINARY099
    ORDER(RTB),
    ADDRESS(V1STO2S),
#else
    /* The CM's: with CPHIFLAG set, it is cleared, and back by S2 at once
     * (THETAD left as it was). */
    ORDERS(RTB, BONCLR),
    ADDRESS(V1STO2S),
    SWITCH(BONCLR, CPHIFLAG),
    ADDRESS(S2),
#endif
    STCALL(THETAD),
    ADDRESS(S2),
    /* GIMLOCK1: out of the interpreter for the alarm. */
    /* GIMLOCK1 */ ORDER(EXIT),
};

const struct interpretive_code CALCGTA_CODE = {CODE_BANK(I_CALCGTA), CODE_Z(I_CALCGTA),
                                               COUNT(calcgta_words), calcgta_words};

/* TC at "at" to a routine outside the file (ALARM, UPFLAG), which reads
 * the word after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* After GIMLOCK1's EXIT: alarm 401, GLOKFAIL set ("GIMBAL LOCK HAS
 * OCCURRED"), and back into the interpreter. */
void gimlock1_after_exit(void)
{
    tc_to((word)(GIMLOCK1 + 1u), ALARM); /* OCT 00401 */
}

void gimlock1_after_alarm(void)
{
    tc_to((word)(GIMLOCK1 + 3u), UPFLAG); /* ADRES GLOKFAIL */
}

void gimlock1_after_upflag(void)
{
    tc_at((word)(GIMLOCK1 + 5u)); /* TC INTPRET */
    intpret();
}

static const word axisgen_words[] = {
    /* Back to CALCGA1, the angles computed as they are. */
    ORDER(GOTO),
    ADDRESS(CALCGA1),
    /* AXISGEN -- LM p. 1255 (CM p. 1361): X1 STARAD +6 and S1 STARAD -6,
     * so that AXISGEN1 runs twice: with X1 STARAD +6 on frame B's stars
     * (STARAD +12D,1 is then the push-down list's 6D), and with X1 12
     * (TIX takes S1 from it) on frame A's, at STARAD. */
    /* AXISGEN */ ORDERS(AXT_1, SSP),
    ADDRESS_AT(STARAD, 6),
    ADDRESS_PLUS_ONE(S1),
    ADDRESS_AT(STARAD, -6),
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    /* AXISGEN1: of the two stars, the triad U (the first star), V (the
     * unit of the first cross the second, in the second's place) and W
     * (U cross V, after it). */
    /* AXISGEN1 */ ORDERS(VLOAD_STAR, VXV_STAR),
    ADDRESS_PLUS_ONE_AT_X1(STARAD, 12),
    ADDRESS_PLUS_ONE_AT_X1(STARAD, 18),
    ORDER(UNIT),
    STORE_AT_X1(STARAD, 18),
    ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(STARAD, 12),
    ORDERS(VXV_STAR, VSL1),
    ADDRESS_PLUS_ONE_AT_X1(STARAD, 18),
    STORE_AT_X1(STARAD, 24),
    ORDER(TIX_1),
    ADDRESS(AXISGEN1),
    /* 30D -6, X1 18 by 6 and X2 6 by 2: AXISGEN2 makes frame A's axes,
     * XDC, YDC and ZDC, each the sum of A's triad, U, V and W, weighted
     * by one component of B's: J, K and L in the listing's comments. */
    ORDERS(AXC_1, SXA_1),
    NUMBER(6),
    ADDRESS_D(30),
    ORDERS(AXT_1, SSP),
    NUMBER(18),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDERS(AXT_2, SSP),
    NUMBER(6),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(2),
    /* AXISGEN2 */ ORDERS(XCHX_1, VLOAD_STAR),
    ADDRESS_D(30),
    ADDRESS_PLUS_ONE_X1(0),
    /* LM p. 1256 (CM p. 1362) */
    ORDERS(VXSC_STAR, PDVL_STAR),
    ADDRESS_PLUS_ONE_AT_X2(STARAD, 6),
    ADDRESS_PLUS_ONE_X1(6),
    ORDER(VXSC_STAR),
    ADDRESS_PLUS_ONE_AT_X2(STARAD, 12),
    STOVL_STAR_D(24),
    ADDRESS_PLUS_ONE_X1(12),
    ORDERS(VXSC_STAR, VAD),
    ADDRESS_PLUS_ONE_AT_X2(STARAD, 18),
    ORDERS(VAD, VSL1),
    ADDRESS_PLUS_ONE_D(24),
    ORDERS(XCHX_1, UNIT),
    ADDRESS_D(30),
    STORE_AT_X1(XDC, 18),
    ORDER(TIX_1),
    ADDRESS(AXISGEN3),
    /* AXISGEN3 */ ORDER(TIX_2),
    ADDRESS(AXISGEN2),
    /* Frame A's axes into STARAD too. */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(XDC),
    STOVL(STARAD),
    ADDRESS_PLUS_ONE(YDC),
    STOVL_AT(STARAD, 6),
    ADDRESS_PLUS_ONE(ZDC),
    STORE_AT(STARAD, 12),
    ORDER(RVQ),
};

/* The words after GIMLOCK1's TC INTPRET. */
const struct interpretive_code AXISGEN_CODE = {
    CODE_BANK(I_GIMLOCK1), (word)(CODE_Z(I_GIMLOCK1) + 6u), COUNT(axisgen_words), axisgen_words};

/* The constants -- LM p. 1257 (CM p. 1363): sin 45 degrees over 4, and
 * 60 degrees in revolutions. */
static const word constant_words[] = {
    /* QTSN45 */ NUMBER(005520), /* 2DEC .1768 */
    NUMBER(026075),
    /* .166... */ NUMBER(005252), /* 2DEC .1666666667 */
    NUMBER(025253),
};

const struct interpretive_code INFLIGHT_CONSTANTS = {CODE_BANK(I_QTSN45), CODE_Z(I_QTSN45),
                                                     COUNT(constant_words), constant_words};
