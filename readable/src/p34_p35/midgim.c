/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 5 (Luminary099 pp. 692-693, Comanche055 pp.
 * 493-494): the middle gimbal angle and the local vertical delta V
 * (MIDGIM's GET+MGA and GET.LVC), and SELECTMU, the primary body's
 * constants. All interpretive code, in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/p34_routines.c). The faithful C is
 * c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * Where they are: the LM's GET+MGA and GET.LVC in bank 6, its SELECTMU in
 * bank 10; the CM's all in bank 4, after a MIDGIM of its own, which picks
 * one of the two by the vehicle the computer is in and the vehicle that
 * is active. */
#include "p34_p35.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A label's bank, and its address in the bank (02000-03777). */
#define BANK_OF(label) ((word)(I_##label / 02000u))
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* The CM's MIDGIM: with the LM active (AVFLAG set) on to MIDGIM1, the
 * middle gimbal angle when the computer is the LGC (COMPUTER clear),
 * else the delta V in local vertical coordinates (GET.LVC); with the CSM
 * active the angle when the computer is the CMC (COMPUTER set), else the
 * delta V. So the active vehicle's own computer finds its gimbal angle,
 * and the other the delta V.
 *
 * GET+MGA: the delta V on the push-down list (0D), unit, dotted with the
 * stable member's Y axis (REFSMMAT's second row, +6), rescaled, and its
 * arc sine; a negative angle made positive "BY ADDING ONE REVOLUTION"
 * (two HALFREVs, half a revolution each). SETMGA: +MGA, and MGLVFLAG
 * clear ("TO INDICATE +MGA CALC"). The LM's takes the unit vector twice.
 *
 * GET.LVC: the local vertical axes from RINIT and VINIT (18D U(-R), down;
 * 12D U(V*R), against the orbit's angular momentum; 6D the third, ahead),
 * the delta V (0D) by them "FROM INER COOR TO LV COOR (+8)" and shifted to
 * DELVLVC's scale (+7), and MGLVFLAG set ("TO INDICATE LVC CALC"). */
static const word midgim_words[] = {
#if !LUMINARY099
    /* MIDGIM */
    ORDERS(BON, BOFF),
    SWITCH(BON, AVFLAG),
    ADDRESS(MIDGIM1),
    SWITCH(BOF, COMPUTER),
    ADDRESS(GET_LVC),
#endif
    /* GET+MGA */
    ORDERS(VLOAD, UNIT), /* "(PL 0D) V (+7) TO MPAC UNITIZE UV (+1)" */
#if LUMINARY099
    ORDER(UNIT),
#endif
    ORDERS(DOT, SL1),                 /* "DOT UV WITH Y(STABLE MEMBER) AND RESCALE" */
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 6), /* "FROM +2 TO +1 FOR ASIN ROUTINE" */
    ORDERS(ARCSIN, BPL),
    ADDRESS(SETMGA),
    ORDERS(DAD, DAD),          /* "CONVERT -MGA TO +MGA BY" */
    ADDRESS_PLUS_ONE(HALFREV), /* "ADDING ONE REVOLUTION" */
    ADDRESS_PLUS_ONE(HALFREV),
    /* SETMGA */
    STORE(PMGA),
    ORDERS(CLEAR, RVQ), /* "CLEAR MGLVFLAG TO INDICATE +MGA CALC" */
    SWITCH(CLEAR, MGLVFLAG),
#if !LUMINARY099
    /* MIDGIM1 */
    ORDER(BOFF),
    SWITCH(BOF, COMPUTER),
    ADDRESS(GETPMGA),
#endif
    /* GET.LVC */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RINIT),
    ORDER(VCOMP), /* "U(-R)" */
    STORE_D(18),  /* "U(-R) TO 18D" */
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(VINIT),
    STORE_D(12), /* "U(V*R) TO 12D" */
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE_D(18),
    STOVL_D(6),            /* "TRANSFORMATION MATRIX IS IN 6D (+1)" */
    ADDRESS_PLUS_ONE_D(0), /* "DELTA V (+7) IN 0D" */
    ORDERS(MXV, VSL1),     /* "CONVERT FROM INER COOR TO LV COOR (+8)" */
    ADDRESS_PLUS_ONE_D(6),
    STORE(DELVLVC),
    ORDERS(SET, RVQ), /* "SET MGLVFLAG TO INDICATE LVC CALC" */
    SWITCH(SET, MGLVFLAG),
};

/* SELECTMU: the primary body by CMOONFLG (the CSM's state in the Moon's
 * sphere): X1 -2 and X2 0 for the Earth, X1 -10D and X2 2 for the Moon.
 * MUTABLE holds the Earth's MU, 1/MU, SQRT(MU) and 1/SQRT(MU), then the
 * Moon's, two words each, and an index register is subtracted from an
 * address: SETMUER loads MUTABLE +4,1, the body's 1/SQRT(MU), keeps X1 in
 * RTX1, stores the root in RTSR1/MU (the square root's store) and loads
 * MUTABLE -2,1, its MU; the Moon's shifted right six places to the
 * Earth's scale. RTRNMU: MU to RTMU, X2 to RTX2, FINALFLG clear, and on
 * to VN1645. */
static const word selectmu_words[] = {
    ORDERS(AXC_1, AXT_2),
    NUMBER(2),
    NUMBER(0),
    ORDER(BOFF),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(SETMUER),
    ORDERS(AXC_1, AXT_2),
    NUMBER(10),
    NUMBER(2),
    /* SETMUER */
    ORDERS(DLOAD_STAR, SXA_1),
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, 4),
    ADDRESS(RTX1),
    STODL_STAR_AT(RTSR1_MU, 0),
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, -2),
    ORDERS(BOFF, SR),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(RTRNMU),
    SR_COUNT(6),
    /* RTRNMU */
    STORE(RTMU),
    ORDERS(SXA_2, CLEAR),
    ADDRESS(RTX2),
    SWITCH(CLEAR, FINALFLG),
    ORDER(GOTO),
    ADDRESS(VN1645),
};

/* ---- The interpretive code ---- */

#if LUMINARY099
#define MIDGIM_FIRST IN_BANK(GETPMGA)
#else
#define MIDGIM_FIRST IN_BANK(MIDGIM)
#endif

const struct interpretive_code MIDGIM_CODE = {BANK_OF(GETPMGA), MIDGIM_FIRST, COUNT(midgim_words),
                                              midgim_words};
const struct interpretive_code SELECTMU_CODE = {BANK_OF(SELECTMU), IN_BANK(SELECTMU),
                                                COUNT(selectmu_words), selectmu_words};
