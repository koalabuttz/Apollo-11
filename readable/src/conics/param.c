/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, PARAM and GEOM (LM pp. 1186-1188, CM pp.
 * 1289-1291), in bank 4, in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs.
 *
 * GEOM: the geometry of two vectors, the first at 0D of the push-down list
 * and the second in MPAC: their unit vectors (UR1 and U2) and lengths (R1
 * and MAGVEC2), the cosine of the angle between them (pushed) and its sine
 * (in MPAC), with GEOMSGN's sign. Unless NORMSW is set (the normal UN
 * given), the unit normal to the two, UN.
 *
 * PARAM: for TIME-THETA (and LAMBERT and TIME-RADIUS), the conic's shape
 * from the position at 0D and the velocity in MPAC. GEOM gives the sine
 * and cosine of GAMMA, the angle between them; COGA is its cotangent, R1A
 * is 2 - R1 V1 squared/MU (R1 over the semi-major axis) and P is R1 V1
 * squared sine squared GAMMA/MU (the semi-latus rectum over R1). On the
 * way the central body's constants go to the work area (1/MU, ROOTMU and
 * 1/ROOTMU, from MUTABLE by X1, as KEPLER's start does). */
#include "conics.h"
#include "../agc/names.h"

static const word param_words[] = {
    /* PARAM -- LM p. 1186 (CM p. 1289): the return to RTNPRM, NORMSW and
     * COGAFLAG cleared, GEOMSGN positive ("GAMMA ALWAYS LESS THAN
     * 180DEG", the operand 37777, which yaYUL assembles as 27777), and
     * GEOM: "MPAC=SNGA (+1), 0D=CSGA (+1)". */
    /* PARAM */ ORDERS(STQ, CLEAR), /* "MPAC=V1VEC, 0D=R1VEC PL AT 6" */
    ADDRESS(RTNPRM),
    SWITCH(CLEAR, NORMSW),
    ORDER(CLEAR),
    SWITCH(CLEAR, COGAFLAG),
    ORDERS(SSP, CALL),
    ADDRESS_PLUS_ONE(GEOMSGN),
    NUMBER(027777),
    ADDRESS(GEOM),
    /* The sine of GAMMA to 36D; its cosine over it, shifted, is COGA,
     * and the body's three constants are loaded by X1 and stored at 14D
     * (1/MU). V1 squared (MAGVEC2, normalized), times 1/MU and R1 and
     * shifted back, is R1 V1SQ/MU, pushed; R1A from it; and then P, R1
     * V1SQ/MU times the sine of GAMMA twice, back through RTNPRM. */
    STODL_D(36), /* "36D=SIN GAMMA (+1) PL AT 0" */
    ORDERS(SR, DDV),
    SR_COUNT(5),
    ADDRESS_PLUS_ONE_D(36),
    STOVL_STAR(COGA),
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, 0),
    STODL(1_MU),
    ADDRESS_PLUS_ONE(MAGVEC2),
    ORDERS(DSQ, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMPR, DMP),
    ADDRESS_PLUS_ONE(1_MU),
    ADDRESS_PLUS_ONE(R1),
    ORDER(SRR_STAR),
    SRR_X1(-3),
    ORDERS(PUSH, BDSU), /* "0D=R1 V1SQ/MU (+6) PL AT 2" */
    ADDRESS_PLUS_ONE(D1_32),
    STODL(R1A), /* "R1A (+6) PL AT 0" */
    ORDERS(DMP, NORM),
    ADDRESS_PLUS_ONE_D(36),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMP, SR_STAR),
    ADDRESS_PLUS_ONE_D(36),
    SR_X1(-4),
#if LUMINARY099
    STORE(P), /* "P (+4)" */
    ORDER(GOTO),
    ADDRESS(RTNPRM),
#else
    STCALL(P), /* "P (+4)" */
    ADDRESS(RTNPRM),
#endif
    /* GEOM -- LM p. 1188 (CM p. 1291): U2 and MAGVEC2 from the second
     * vector, UR1 and R1 from the first, the cosine pushed. Then the cross
     * product of the unit vectors, the sine times the normal: with NORMSW
     * set, HAVENORM, its length; otherwise its unit vector is UN (if
     * UNIT overflows, COLINEAR shifts MPAC right one for UN instead). The
     * sine takes GEOMSGN's sign, and RVQ returns. */
    /* GEOM */ ORDER(UNIT), /* "MPAC=V2VEC, 0D=R1VEC PL AT 6" */
    STODL(U2),              /* "U2 (+1)" */
    ADDRESS_PLUS_ONE_D(36),
    STOVL(MAGVEC2), /* "PL AT 0" */
    ORDER(UNIT),
    STORE(UR1), /* "UR1 (+1)" */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(U2),
    ORDER(PDDL), /* the cosine, CSTH, at 0D: "PL AT 2" */
    ADDRESS_PLUS_ONE_D(36),
    STOVL(R1), /* "R1 (+29 OR +27)" */
    ADDRESS_PLUS_ONE(UR1),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(U2),
    ORDERS(BON, SIGN),
    SWITCH(BON, NORMSW),
    ADDRESS(HAVENORM),
    ADDRESS_PLUS_ONE(GEOMSGN),
    ORDERS(UNIT, BOV),
    ADDRESS(COLINEAR),
    /* UNITNORM */ STODL(UN), /* "UN (+1)" */
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(SIGN, RVQ), /* "MPAC=SNTH (+1), 34D=SNTH.SNTH (+2)" */
    ADDRESS_PLUS_ONE(GEOMSGN),
    /* COLINEAR */ ORDERS(VSR1, GOTO),
    ADDRESS(UNITNORM),
    /* HAVENORM */ ORDERS(ABVAL, SIGN),
    ADDRESS_PLUS_ONE(GEOMSGN),
    ORDER(RVQ), /* "MPAC=SNTH (+1), 34D=SNTH.SNTH (+2)" */
};

const struct interpretive_code PARAM_CODE = {I_PARAM >> 10, K_PARAM, COUNT(param_words),
                                             param_words};
