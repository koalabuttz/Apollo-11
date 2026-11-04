/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 4 (Luminary099 pp. 685-690, Comanche055 pp.
 * 486-490): INITVEL, the velocity a vehicle needs at RINIT to reach RTARG
 * in DELLT4, in bank 11 of both missions. All interpretive code, in the
 * listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs (src/binding/p34_routines.c). The faithful C is
 * c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * "THIS SUBROUTINE COMPUTES THE REQUIRED INITIAL VELOCITY VECTOR FOR A
 * TRAJECTORY OF SPECIFIC TRANSFER TIME BETWEEN SPECIFIED INITIAL AND
 * TARGET POSITIONS." LAMBERT finds the conic; with iterations asked for
 * (0D, the number of them), INTEGRVS flies the precision trajectory from
 * that velocity, and the conic's target moves by the miss until the
 * count runs out: the result then also gives an offset target, the end
 * of a conic with the same start, for cross-product steering. Near a
 * 180-degree transfer (inside the cone 2D gives), the target is first
 * turned into the plane of RINIT and VINIT ("ROTATE RC INTO YC PLANE").
 *
 * The missions differ in three places: the LM keeps the target's length
 * in RTMAG, calls LAMBERT through SETITCTR ("GO TO END OF BANK TO SET
 * ITERCTR BEFORE CALLING LAMBER"), and at the end sets MU/A and MUASTEER
 * for steering. */
#include "p34_p35.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* INITVEL: GUESSW set ("COGA GUESS NOT AVAILABLE"); HAVEGUES: the return
 * to NORMEX, RTARG to RTARG1 (the LM keeps its length in RTMAG), and in
 * the Moon's sphere (RTX2 not zero) RINIT, VINIT and RTARG1 shifted left
 * two places ("B29" to "B27", "B7" to "B5").
 *
 * INITVEL1, "INITIALIZATION": ITCTR -1, and from the cone angle E4 (2D)
 * COZY4, its cosine; VTARGTAG the iteration count (0D, MPAC's after the
 * DLOAD); R1VEC, R2VEC and TDESIRED, LAMBERT's inputs; UN, the unit
 * normal of RINIT and VINIT; COZY4 the target's direction cosine to RINIT
 * plus the cone's. INITVEL2: a positive sum, the target outside the cone
 * ("UN CALCULATED IN LAMBERT"), on at INITVEL3; otherwise NORMSW set and
 * R2VEC turned into the plane, RC less its projection on UN, at RC's
 * length; on the first pass (ITCTR negative) RTARG1 too.
 *
 * INITVEL3: the gravity constant, unit R2VEC and R1VEC, and their normal;
 * by X1 (RTX1, less 8 on the Earth's) and RTX2 the vehicle's sense of
 * travel, GEOMSGN; then LAMBERT ("OPERATE THE LAMBERT CONIC ROUTINE").
 * VIPRIME, its velocity. With no iterations (VTARGTAG zero) on to
 * INITVEL7; otherwise INTEGRVS from RINIT with it, in the Moon's sphere
 * or the Earth's, to the time of flight (INITVEL5), VTARGET its velocity
 * at the end. ITCTR one more; with iterations left the conic target moves
 * by RTARG1 less RATT1 (the miss) and round again from INITVEL2;
 * otherwise (INITVEL6) the last R2VEC is the offset target. INITVEL7:
 * DELVEET3, VIPRIME less VINIT; VTPRIME, the velocity at the target; in
 * the Moon's sphere the outputs shifted back. INITVELX: (the LM's) MU/A
 * and MUASTEER, then RTARG1 to RTARG, and back by NORMEX. */
static const word initvel_words[] = {
    ORDER(SET), /* "COGA GUESS NOT AVAILABLE" */
    SWITCH(SET, GUESSW),
    /* HAVEGUES */
    ORDERS(VLOAD, STQ),
    ADDRESS_PLUS_ONE(RTARG),
    ADDRESS(NORMEX),
    STORE(RTARG1),
#if LUMINARY099
    ORDER(ABVAL),
    STORE(RTMAG),
#endif
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(RTX2),
    ADDRESS(INITVEL1),
    ORDERS(VLOAD, VSL2),
    ADDRESS_PLUS_ONE(RINIT), /* "B29" */
    STOVL(RINIT),            /* "B27" */
    ADDRESS_PLUS_ONE(VINIT), /* "B7" */
    ORDER(VSL2),
    STOVL(VINIT), /* "B5" */
    ADDRESS_PLUS_ONE(RTARG1),
    ORDER(VSL2),
    STORE(RTARG1),
#if LUMINARY099
    ORDER(ABVAL),
    STORE(RTMAG),
#endif
    /* INITVEL1 */
    ORDERS(SSP, DLOAD), /* "SET ITCTR TO -1,LOAD MPAC WITH E4 (PL 2D)" */
    ADDRESS_PLUS_ONE(ITCTR),
    MINUS_NUMBER(1),
    ORDERS(COS, SR1), /* "CALCULATE COSINE (E4) (+2)" */
    STODL(COZY4),     /* "SET COZY4 TO COSINE (E4)" */
    ORDERS(LXA_2, SXA_2),
    ADDRESS(MPAC),
    ADDRESS(VTARGTAG), /* "SET VTARGTAG TO 0D (SP)" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RINIT),
    STOVL(R1VEC), /* "R1VEC EQ RINIT" */
    ADDRESS_PLUS_ONE(RTARG1),
    STODL(R2VEC), /* "R2VEC EQ RTARG" */
    ADDRESS_PLUS_ONE(DELLT4),
    STORE(TDESIRED), /* "TDESIRED EQ DELLT4" */
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),   /* "INITIALIZE PL TO 0D" */
    ADDRESS_PLUS_ONE(RINIT), /* "MPAC EQ RINIT (+29)" */
    ORDERS(UNIT, PUSH),      /* "UNIT(RI) (+1)" */
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(VINIT), /* "MPAC EQ UNIT(RI) X VI (+8)" */
    STOVL(UN),
    ADDRESS_PLUS_ONE(RTARG1),
    ORDERS(UNIT, DOT), /* "TEMP*RT.URI (+2)" */
    ORDERS(DAD, CLEAR),
    ADDRESS_PLUS_ONE(COZY4),
    SWITCH(CLEAR, NORMSW),
    STORE(COZY4),
    /* INITVEL2 */
    ORDERS(BPL, SET),
    ADDRESS(INITVEL3), /* "UN CALCULATED IN LAMBERT" */
    SWITCH(SET, NORMSW),
    /* "ROTATE RC INTO YC PLANE -- SET UNIT NORMAL TO YC" */
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(R2VEC), /* "RC TO 6D (+29)" */
    ORDERS(ABVAL, PDVL),     /* "RC TO MPAC, ABVAL(RC) (+29) TO OD" */
    ORDERS(PUSH, VPROJ),
    ADDRESS_PLUS_ONE(UN),
    ORDERS(VSL2, BVSU),
    ORDERS(UNIT, VXSC),
    ORDER(VSL1),
    STORE(R2VEC),
    ORDERS(TLOAD, SLOAD),
    ADDRESS_PLUS_ONE(ZEROVEC),
    ADDRESS_PLUS_ONE(ITCTR),
    ORDERS(BPL, VLOAD),
    ADDRESS(INITVEL3),
    ADDRESS_PLUS_ONE(R2VEC),
    STORE(RTARG1),
    /* INITVEL3 */
    ORDERS(DLOAD, PDVL),
    ADDRESS_PLUS_ONE(MUEARTH), /* "POSITIVE VALUE" */
    ADDRESS_PLUS_ONE(R2VEC),
    ORDERS(UNIT, PDVL), /* "2D = UNIT(R2VEC)" */
    ADDRESS_PLUS_ONE(R1VEC),
    ORDERS(UNIT, PUSH), /* "8D = UNIT(R1VEC)" */
    ORDERS(VXV, VCOMP), /* "-N = UNIT(R2VEC) X UNIT(R1VEC)" */
    ADDRESS_PLUS_ONE_D(2),
    ORDER(PUSH),
    ORDERS(LXA_1, DLOAD),
    ADDRESS(RTX1),
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(BMN, INCR_1),
    ADDRESS_AT(INITVEL3, 017), /* "+2" */
    MINUS_NUMBER(8),
    ORDERS(INCR_1, SLOAD),
    NUMBER(10), /* 10D */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BHIZ, VLOAD),
    ADDRESS_AT(INITVEL3, 025), /* "+2" */
    ORDERS(VCOMP, PUSH),
    ORDER(VLOAD),
    ORDERS(VXV, DOT),
    ORDERS(BPL, DLOAD),
    ADDRESS(INITVEL4),
    ORDERS(DCOMP, PUSH),
    /* INITVEL4 */
    ORDERS(LXA_2, SXA_2),
    ADDRESS_D(0),
    ADDRESS(GEOMSGN),
    /* "SET INPUTS UP FOR LAMBERT" */
    ORDERS(LXA_1, CALL),
    ADDRESS(RTX1),
#if LUMINARY099
    ADDRESS(SETITCTR), /* "GO TO END OF BANK TO SET ITERCTR BEFORE" */
#else
    ADDRESS(LAMBERT),
#endif
    ORDERS(CLEAR, VLOAD),
    SWITCH(CLEAR, GUESSW),
    ADDRESS_PLUS_ONE(VVEC),
    STODL(VIPRIME), /* "INITIAL VELOCITY REQUIRED (+7)" */
    ADDRESS_PLUS_ONE(VTARGTAG),
    ORDERS(BHIZ, CALL),
    ADDRESS(INITVEL7),
    ADDRESS(INTSTALL),
    ORDERS(SLOAD, CLEAR),
    ADDRESS_PLUS_ONE(RTX2),
    SWITCH(CLEAR, MOONFLAG),
    ORDERS(BHIZ, SET),
    ADDRESS(INITVEL5),
    SWITCH(SET, MOONFLAG),
    /* INITVEL5 */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RINIT),
    STORE(R1VEC),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(VIPRIME),
    STODL(VCV),
    ADDRESS_PLUS_ONE(INTIME),
    STORE(TET),
    ORDERS(DAD, CLEAR),
    ADDRESS_PLUS_ONE(DELLT4),
    SWITCH(CLEAR, INTYPFLG),
    STCALL(TDEC1),
    ADDRESS(INTEGRVS),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VATT1),
    STORE(VTARGET),
    ORDERS(LXA_2, INCR_2),
    ADDRESS(ITCTR),
    NUMBER(1), /* "INCREMENT ITCTR" */
    ORDERS(SXA_2, XSU_2),
    ADDRESS(ITCTR),
    ADDRESS(VTARGTAG),
    ORDERS(SLOAD, BHIZ), /* "IF SP(MPAC) EQ 0, CONTINUE AT INITVELC" */
    ADDRESS_PLUS_ONE(X2),
    ADDRESS(INITVEL6),
    /* "OFFSET CONIC TARGET VECTOR" */
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(RTARG1),
    ADDRESS_PLUS_ONE(RATT1),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(R2VEC),
    STODL(R2VEC),
    ADDRESS_PLUS_ONE(COZY4),
    ORDER(GOTO),
    ADDRESS(INITVEL2), /* "CONTINUE ITERATING AT INITVEL2" */
    /* INITVEL6, "COMPUTE THE DELTA VELOCITY" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(R2VEC),
    STORE(RTARG1),
    /* INITVEL7 */
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(VIPRIME),
    ADDRESS_PLUS_ONE(VINIT),
    STOVL(DELVEET3), /* "DELVEET3 = VIPRIME-VINIT (+7)" */
    ADDRESS_PLUS_ONE(VTARGET),
    STORE(VTPRIME),
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(RTX2),
    ADDRESS(INITVELX),
    ORDERS(VLOAD, VSR2),
    ADDRESS_PLUS_ONE(VTPRIME),
    STOVL(VTPRIME),
    ADDRESS_PLUS_ONE(VIPRIME),
    ORDER(VSR2),
    STOVL(VIPRIME),
    ADDRESS_PLUS_ONE(RTARG1),
    ORDER(VSR2),
    STOVL(RTARG1),
    ADDRESS_PLUS_ONE(DELVEET3),
    ORDER(VSR2),
    STORE(DELVEET3),
/* INITVELX */
#if LUMINARY099
    ORDERS(LXA_1, DLOAD_STAR),
    ADDRESS(RTX1),
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, -2),
    ORDERS(PUSH, DMP),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(SR1, DDV),
    ADDRESS_PLUS_ONE(R1),
    STODL(MU_A),
    ORDER(SR),
    SR_COUNT(6),
    STORE(MUASTEER),
#endif
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RTARG1),
    STCALL(RTARG),
    ADDRESS(NORMEX),
};

const struct interpretive_code INITVEL_CODE = {011u, IN_BANK(INITVEL), COUNT(initvel_words),
                                               initvel_words};
