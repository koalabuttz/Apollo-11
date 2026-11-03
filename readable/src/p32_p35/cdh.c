/* SPDX-License-Identifier: MIT */
/* P32-P35_P72-P75.agc (Luminary099 pp. 642-649): the subroutines of the
 * CSI and CDH computations (bank 34), from ADVANCE to CDHMVR. ADVANCE
 * brings both vehicles to TIG and into the passive vehicle's orbital
 * plane; INTINT2C and INTINT3P integrate a state vector from TCSI to TCDH
 * and from TCDH to TTPI; S32/33.1 shows the delta V in local vertical
 * coordinates and turns it into reference coordinates; CDHMVR finds the
 * CDH burn, the one that puts the active vehicle on an orbit concentric
 * with the passive vehicle's.
 *
 * All interpretive: ADVANCE_CODE holds the words, which the readable
 * interpreter runs. A comment names each stretch the way the listing's
 * comments do; "PL02D" is the push-down list's pointer after the order. */
#include "p32_p35.h"
#include "../agc/names.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word advance_words[] = {
    /* ADVANCE: the return in SUBEXIT; both vehicles to TIG (PRECSET, with
     * TDEC1 TIG); XDELVFLG set (the burn's velocity to be gained from an
     * external delta V, not by Lambert). The passive vehicle's state there, RPASS3
     * and VPASS3, to RPASS1 and RPASS2, VPASS1 and VPASS2; UP1, the unit
     * normal to its orbit, UNIT(UNIT(RPASS1) X VPASS1). */
    ORDERS(STQ, DLOAD),
    ADDRESS(SUBEXIT),
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TDEC1),
    ADDRESS(PRECSET),
    ORDERS(SET, VLOAD),
    SWITCH(SET, XDELVFLG),
    ADDRESS_PLUS_ONE(VPASS3),
    STORE(VPASS2),
    STOVL(VPASS1),
    ADDRESS_PLUS_ONE(RPASS3),
    STORE(RPASS2),
    STORE(RPASS1),
    ORDERS(UNIT, VXV),
    ADDRESS_PLUS_ONE(VPASS1),
    ORDER(UNIT),
    /* The active vehicle's state, RACT3 and VACT3, each to RTIG and VTIG
     * and through ROTATE into that plane, to RACT1 and RACT2, VACT1 and
     * VACT2; back through SUBEXIT. */
    STOVL(UP1),
    ADDRESS_PLUS_ONE(RACT3),
    STCALL(RTIG),
    ADDRESS(ROTATE),
    STORE(RACT2),
    STOVL(RACT1),
    ADDRESS_PLUS_ONE(VACT3),
    STCALL(VTIG),
    ADDRESS(ROTATE),
    STORE(VACT2),
    STCALL(VACT1),
    ADDRESS(SUBEXIT),
    /* ROTATE: the vector in MPAC less its part along UP1, (V . UP1) UP1,
     * as a unit vector, times the vector's own length: the vector turned
     * into the passive vehicle's orbital plane. RVQ returns. */
    ORDERS(PUSH, PUSH),
    ORDERS(DOT, VXSC),
    ADDRESS_PLUS_ONE(UP1),
    ADDRESS_PLUS_ONE(UP1),
    ORDERS(VSL2, BVSU),
    ORDERS(UNIT, PDVL),
    ORDERS(ABVAL, VXSC),
    ORDERS(VSL1, RVQ),
    /* INTINT2C: the state vector on the push-down list from TCSI to
     * TCDH; to INTINT, whose last word on the list, TWOPI (not zero),
     * sets INTYPFLG: a conic integration. */
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TCSI),
    ADDRESS_PLUS_ONE(TCDH),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(TWOPI),
    ORDER(GOTO),
    ADDRESS(INTINT),
    /* INTINT3P: from TCDH to TTPI; P30ZERO (zero) leaves INTYPFLG clear,
     * an Encke integration. */
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TCDH),
    ADDRESS_PLUS_ONE(TTPI),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(P30ZERO),
    ORDER(GOTO),
    ADDRESS(INTINT),
    /* S32/33.1: the return in SUBEXIT; DISDVLVC with verb 06 noun 81 in
     * X1 (VN 0681), which shows DELVLVC, the delta V in local vertical
     * coordinates; S32/33.X's matrix (at 0D) turns it into reference
     * coordinates, doubled, DELVSIN, and its length DELVSAB; back through
     * SUBEXIT with DELVSIN in MPAC. */
    ORDERS(STQ, AXT_1),
    ADDRESS(SUBEXIT),
    NUMBER(06u * 0200u + 81u), /* VN 0681: verb 06, noun 81 */
    ORDER(CALL),
    ADDRESS(DISDVLVC),
    ORDER(CALL),
    ADDRESS(S32_33_X),
    ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(DELVLVC),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(VSL1),
    STORE(DELVSIN),
    ORDERS(PUSH, ABVAL),
    STOVL(DELVSAB),
    ORDER(GOTO),
    ADDRESS(SUBEXIT),
    /* S32/33.X: the local vertical axes, as the rows of a matrix on the
     * push-down list: at 0D -UNIT(RACT1) X UP1, doubled; at 6D -UP1; at
     * 12D -UNIT(RACT1). RVQ returns. */
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE(UP1),
    ORDERS(VCOMP, PDVL),
    ADDRESS_PLUS_ONE(RACT1),
    ORDERS(UNIT, VCOMP),
    ORDERS(PUSH, VXV),
    ADDRESS_PLUS_ONE(UP1),
    ORDER(VSL1),
    STORE_D(0),
    ORDER(RVQ),
    /* CDHMVR: the return in SUBEXIT. UNVEC, UNIT(RACT2) ("UR SUB A");
     * CSTH and SNTH, the cosine and sine of the angle from RPASS2 to it,
     * the sine's sign from the side of the passive orbit's plane it is
     * on (UP1). */
    ORDERS(STQ, VLOAD),
    ADDRESS(SUBEXIT),
    ADDRESS_PLUS_ONE(RACT2),
    ORDERS(PUSH, UNIT),
    STOVL(UNVEC), /* "UR SUB A" */
    ADDRESS_PLUS_ONE(RPASS2),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(UNVEC),
    ORDERS(PUSH, SL1),
    STODL(CSTH),
    ORDERS(DSQ, PDDL),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ORDERS(SR2, DSU),
    ORDERS(SQRT, SL1),
    ORDERS(PDVL, VCOMP),
    ORDER(VXV),
    ADDRESS_PLUS_ONE(RPASS2),
    ORDERS(DOT, PDDL),
    ADDRESS_PLUS_ONE(UP1),
    ORDERS(SIGN, STADR),
    AFTER_STADR(STOVL(SNTH)),
    ADDRESS_PLUS_ONE(RPASS2),
    /* The passive vehicle through that angle (VECSHIFT, then TIMETHET with
     * RVSW clear, which gives the state there as well as the time): its
     * position, rescaled by RTX2 (VSL* 0,2), at 18D; its velocity along
     * UNVEC, the vertical, and its distance from the center. */
    ORDERS(PDVL, CALL),
    ADDRESS_PLUS_ONE(VPASS2),
    ADDRESS(VECSHIFT),
    STOVL(VVEC),
    ORDER(CLEAR),
    SWITCH(CLEAR, RVSW),
    STCALL(RVEC),
    ADDRESS(TIMETHET),
    ORDERS(LXA_2, VSL_STAR),
    ADDRESS(RTX2),
    VSL_X2(0),
    STORE_D(18),
    ORDERS(DOT, SL1R),
    ADDRESS_PLUS_ONE(UNVEC),
    ORDERS(PDVL, ABVAL), /* "0D = V SUB PV" */
    ORDERS(SL_STAR, PDVL),
    VSL_X2(0),
    ADDRESS_PLUS_ONE(RACT2),
    /* DIFFALT, the active vehicle's distance from the center less the
     * passive's ("DELTA H IN METERS"); the passive's semimajor axis, its
     * distance over R1A, 2 less R V squared over MU (which SHIFTR1
     * rescales); the active's, that less DIFFALT ("A SUB A"). */
    ORDERS(ABVAL, PDDL), /* "2D = LENGTH OF R SUB A" */
    ORDER(DSU),
    ADDRESS_PLUS_ONE_D(2),
    STODL(DIFFALT), /* "DELTA H IN METERS" */
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(NORM, PDDL), /* 2 less R V squared over MU, PL04D */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(R1),
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    ORDERS(SR1R, DDV),
    ORDERS(SL_STAR, PUSH),
    SL_X1(-5),
    ORDERS(DSU, PDDL), /* "A SUB A", PL04D */
    ADDRESS_PLUS_ONE(DIFFALT),
    /* The active vehicle's vertical speed at CDH, the passive's times
     * (A SUB P/A SUB A) to the power 3/2 ("V SUB AV METERS/CS"); its
     * horizontal speed, "SQRT(MU(2/R SUB A-1/A SUB A)-VSUBA2)". */
    ORDERS(SR2, DDV), /* "A SUB P" */
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(PUSH, SQRT), /* "A SUB P/A SUB A", PL06D */
    ORDERS(DMPR, DMP),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SL3R, PDDL),    /* "V SUB AV METERS/CS", PL08D */
    ADDRESS_PLUS_ONE_D(2), /* "R SUB A MAGNITUDE" */
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(RTMU),
    ORDERS(SR1, DDV),      /* "2MU" */
    ORDERS(SL_STAR, PDDL), /* "2 MU/R SUBAA", PL10D */
    SL_X1(-5),
    ADDRESS_PLUS_ONE_D(4), /* "ASUBA" */
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X2),
    ADDRESS_PLUS_ONE(RTMU),
    ORDERS(SR1, DDV),
    ORDERS(SL_STAR, BDSU),
    VSL_X2(-6), /* "2U/R - U/A" */
    ORDERS(PDDL, DSQ),
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(BDSU, SQRT),
    /* VACT3, the active vehicle's velocity after CDH: the horizontal
     * speed along UNIT(UP1 X UNVEC), and the vertical along UNVEC;
     * DELVEET2, the CDH delta V, VACT3 less VACT2 ("DELTA VCDH --
     * REFERENCE COORDINATES"); back through SUBEXIT. */
    ORDERS(PDVL, VXV), /* "SQRT(MU(2/R SUB A-1/A SUB A)-VSUBA2)", PL10D */
    ADDRESS_PLUS_ONE(UP1),
    ADDRESS_PLUS_ONE(UNVEC),
    ORDERS(UNIT, VXSC),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(PDVL, VXSC),
    ADDRESS_PLUS_ONE(UNVEC),
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(VAD, VSL1),
    ORDER(STADR),
    AFTER_STADR(STORE(VACT3)),
    ORDER(VSU),
    ADDRESS_PLUS_ONE(VACT2),
    STCALL(DELVEET2), /* "DELTA VCDH -- REFERENCE COORDINATES" */
    ADDRESS(SUBEXIT),
};

const struct interpretive_code ADVANCE_CODE = {034u, I_ADVANCE % 02000u + 02000u,
                                               COUNT(advance_words), advance_words};

#endif
