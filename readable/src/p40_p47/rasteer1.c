/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 3 (Luminary099 pp. 776-780, bank 27): RASTEER1, in
 * the listing's notation. The faithful C is c64-port/src/lm_p40_p47.c.
 *
 * RASTEER1 is S40.8's way to the velocity to be gained for a burn to an
 * aim point (XDELVFLG clear): each SERVICER pass it works out the
 * velocity required at the vehicle's position now (RN) to reach the
 * target (RTARG) at TPASS4, by the conic through the two (a Lambert
 * solution kept up to date with the semi-major axis's MU/A and MUASTEER,
 * which INITVEL left), and VG, that less the velocity now (VN1), with the
 * Earth's oblateness added near the Earth. It ends at S40.8's VGAIN*. */
#include "p40_p47.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* RASTEER1: RMAG the magnitude of RN, IC the unit chord to RTARG
 * (NORMUNX1), "C(36D) = ABVAL(C)", in 30D, R1C the product of RMAG and
 * the chord, normalized (its scale in MUSCALE). "SS = (R1 + R2 + C)/2".
 * From SS, the chord and the two radii with MU/A and MUASTEER, A ("+ OR
 * - A", in 32D, with GEOMSGN's sign) and B ("-B (NO SIGN)"), and the
 * conic's time of flight less TPASS4, plus PIPTIME, signs B, "B WITH
 * SIGN", in 30D. The velocity required: B along the unit of IC less UNIT/R/ plus A
 * along that of IC plus UNIT/R/; or with NORMSW set (180MESS), near a
 * 180-degree transfer, from the orbit's normal (UN), whichever way IC
 * and UNIT/R/ point (NEGPROD). GETVRVG2: in VIPRIME, shifted back by
 * RTX2's scale, and on at ASTREND -2: VG, "OUTPUT FROM INITVEL VG = VR -
 * VN" (VN1), in DELVEET3. FIRSTTME: "NO OBLATENESS COMP IF IN MOON
 * SPHERE" (RTX2 nonzero); GETGOBL: "CALCULATE OBLATENESS TERM.", GOBL
 * from UNITGOBL, EARTHMU, RN and the time since GOBLTIME, added in;
 * NOGOBL: "VG = VR + GOBL - VN", and on at VGAIN*. */
static const word rasteer1_words[] = {
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(RN),
    ORDERS(LXC_2, SL_STAR),
    ADDRESS(RTX2),
    VSL_X2(0),
    STOVL(RMAG),
    ADDRESS_PLUS_ONE(RTARG),
    ORDERS(VSU, RTB),
    ADDRESS_PLUS_ONE(RN),
    ADDRESS(NORMUNX1),
    STODL(IC),
    ADDRESS_PLUS_ONE_D(36), /* "C(36D) = ABVAL(C)" */
    ORDERS(XAD_2, SL_STAR),
    ADDRESS(X1),
    VSL_X2(0),
    STORE_D(30),
    ORDERS(NORM, DMP),
    ADDRESS_PLUS_ONE(X2),
    ADDRESS_PLUS_ONE(RMAG),
    ORDERS(NORM, XAD_2),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS(X1),
    ORDER(SXA_2),
    ADDRESS(MUSCALE),
    STODL(R1C), /* "2(+56 -X)" */
    ADDRESS_PLUS_ONE_D(30),
    ORDERS(SR1, PDDL),
    ADDRESS_PLUS_ONE(RMAG),
    ORDERS(SR1, PDDL),
    ADDRESS_PLUS_ONE(RTMAG),
    ORDERS(SR1, DAD),
    ORDERS(DAD, STADR),
    AFTER_STADR(STORE(SS)), /* "SS = (R1 + R2 + C)/2" */
    ORDERS(DSU, DMP),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS_PLUS_ONE(MU_A),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(MUASTEER),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE(SS),
    ADDRESS_PLUS_ONE(RMAG),
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DDV, DMP),
    ADDRESS_PLUS_ONE(R1C),
    ORDERS(XSU_2, SL_STAR),
    ADDRESS(X1),
    VSL_X2(1),
    ORDER(LXA_2),
    ADDRESS(MUSCALE),
    ORDERS(SQRT, SIGN),
    ADDRESS_PLUS_ONE(GEOMSGN),
    STORE_D(32), /* "+ OR - A" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(SS),
    ADDRESS_PLUS_ONE(MU_A),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(MUASTEER),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE(SS),
    ADDRESS_PLUS_ONE(RTMAG),
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DDV, DMP),
    ADDRESS_PLUS_ONE(R1C),
    ORDERS(XSU_2, SL_STAR),
    ADDRESS(X1),
    VSL_X2(1),
    ORDERS(SQRT, PDDL), /* "-B (NO SIGN)" */
    ADDRESS_PLUS_ONE(SS),
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS_PLUS_ONE(SS),
    ORDERS(SQRT, PUSH),
    ORDERS(SR1, ASIN),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE(2PIP3),
    ORDERS(PDDL, DDV),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS_PLUS_ONE(SS),
    ORDER(BOV),
    ADDRESS_AT(RASTEER1, 0122), /* "+1" */
    ORDERS(SQRT, DMP),
    ORDERS(SR3, BDSU),
    ORDERS(SIGN, PDDL),
    ADDRESS_PLUS_ONE(GEOMSGN),
    ADDRESS_PLUS_ONE(2PIP3),
    ORDERS(SR2, DSU),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE(SS),
    ADDRESS_PLUS_ONE(SS),
    ORDERS(SR3, SQRT),
    ORDER(DMP),
    ORDERS(PDDL, SL3),
    ADDRESS_PLUS_ONE(MUASTEER),
    ORDERS(SQRT, BDDV),
    ORDERS(DSU, DAD),
    ADDRESS_PLUS_ONE(TPASS4),
    ADDRESS_PLUS_ONE(PIPTIME),
    STODL_D(30),
    ORDER(SIGN),
    ADDRESS_PLUS_ONE_D(30), /* "B WITH SIGN" */
    STORE_D(30),
    ORDERS(BON, VLOAD),
    SWITCH(BON, NORMSW),
    ADDRESS(180MESS),
    ADDRESS_PLUS_ONE(IC),
    ORDERS(VSU, UNIT),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VXSC, PDVL),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS_PLUS_ONE(IC),
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE(UNIT_R_),
    /* GETVRVG1 */
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE_D(32),
    /* GETVRVG2 */
    ORDERS(LXC_2, VSR_STAR),
    ADDRESS(RTX2),
    SR_X2(-1),
    STORE(VIPRIME),
    ORDER(GOTO),
    ADDRESS_AT(ASTREND, -2),
    /* 180MESS */
    ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(IC),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(BMN, VLOAD),
    ADDRESS(NEGPROD),
    ADDRESS_PLUS_ONE(IC),
    ORDERS(VSR1, PDVL),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VSR1, VAD),
    ORDER(UNIT),
    ORDERS(PUSH, VCOMP), /* "FOR A" */
    ORDERS(VXV, SIGN),
    ADDRESS_PLUS_ONE(UN),
    ADDRESS_PLUS_ONE(GEOMSGN),
    ORDERS(UNIT, VXSC),
    ADDRESS_PLUS_ONE_D(30),
    ORDER(PDVL), /* "UNIT(IC-IR)", "+-B" */
    ORDER(GOTO),
    ADDRESS(GETVRVG1),
    /* NEGPROD */
    ORDERS(VLOAD, VSR1),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(PDVL, VSR1),
    ADDRESS_PLUS_ONE(IC),
    ORDERS(VSU, UNIT),
    ORDER(PUSH),
    ORDERS(VXV, SIGN),
    ADDRESS_PLUS_ONE(UN), /* "FOR B" */
    ADDRESS_PLUS_ONE(GEOMSGN),
    ORDERS(UNIT, VXSC),
    ADDRESS_PLUS_ONE_D(32),
    ORDER(PDVL),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE_D(30),
    ORDER(GOTO),
    ADDRESS(GETVRVG2),
    ORDER(VSU),
    ADDRESS_PLUS_ONE(VN1),
    /* ASTREND */
    STORE(DELVEET3),
    /* FIRSTTME */
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(RTX2),
    ADDRESS(GETGOBL),
    ORDERS(VLOAD, GOTO), /* "NO OBLATENESS COMP IF IN MOON SPHERE" */
    ADDRESS_PLUS_ONE(DELVEET3),
    ADDRESS(NOGOBL),
    /* GETGOBL */
    ORDERS(VLOAD, UNIT), /* "CALCULATE OBLATENESS TERM." */
    ADDRESS_PLUS_ONE(RN),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(PIPTIME),
    ADDRESS_PLUS_ONE(GOBLTIME), /* "G    = -(MU/R )(UNITGOBL)(T-TIG)" */
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(EARTHMU),
    ADDRESS_PLUS_ONE_D(34), /* "34D = /RN/ (2) FROM UNIT OPERATION." */
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UNITGOBL),
    ADDRESS_PLUS_ONE(DELVEET3), /* "OUTPUT FROM INITVEL VG = VR - VN" */
    /* NOGOBL */
    STORE(DELVEET3), /* "VG = VR + GOBL - VN" */
    ORDER(GOTO),
    ADDRESS(VGAIN_STAR),
};

const struct interpretive_code RASTEER1_CODE = {027u, RASTEER1, COUNT(rasteer1_words),
                                                rasteer1_words};

#endif
