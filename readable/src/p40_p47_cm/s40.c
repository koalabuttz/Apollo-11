/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 4 (Comanche055 pp. 709-718): S40.1, S40.2,3 and
 * S41.1, the burn's targeting, all interpretive code. The faithful C is
 * c64-port/src/cm_p40_p47.c.
 *
 * S40.1 (bank 16) finds the thrust direction (UT) and the velocity to be
 * gained at ignition (VGTIG): for an external delta V burn from the
 * delta V asked for (DELVSIN); for a Lambert burn (S40.1B) from the
 * velocity Lambert needs (AGAIN, bank 30) at ignition and two seconds
 * before, with CALCUT steering UT by the change. S40.2,3 (bank 24) turns
 * the engine bell's axis (SCAXIS), the preferred IMU orientation
 * (XSCREF, YSCREF, ZSCREF) and the thrust direction in stable member
 * coordinates (POINTVSM) out of UT. S41.1 (bank 22) turns the velocity
 * to be gained into control coordinates. */
#include "p40_p47_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- S40.1 -- pp. 709-712 ---- */

/* S40.1: FIRSTFLG set, BDT zero, the return in QTEMP (STQ). A Lambert
 * burn (XDELVFLG clear), S40.1B. An external delta V: DELVSAB its length,
 * "COMPUTE FOR P30/P40 INTERFACE"; VTIG to VINIT, RTIG to RINIT; UT,
 * "UP", the unit of VTIG cross RTIG. The burn's angle from DELVSAB,
 * WEIGHT/G and the thrust F (THETACON); VGTIG, "VG IGNITION SCALED AT
 * 2(+7) M/CS", from DELVSIN's parts along UP and across it; UT its unit,
 * "THRUST DIRECTION SCALED AT 2(+1)". AVFLAG set; MIDGIM, "VGTIG IN LV
 * COOR AT 2(+7)M/CS IN DELVLVC"; back through QTEMP.
 *
 * S40.1B, "LAMBERT": DELLT4, the time from TIG less two seconds (TWODT,
 * TDEC1) to TPASS4, and AGAIN; VIPRIME to UT; again from TIG itself,
 * which leaves the velocity to be gained in DELVEET3, to VGTIG. AVFLAG
 * set, MIDGIM, and on at CALCUT. */
static const word s40_1_words[] = {
    ORDERS(SET, VLOAD),
    SWITCH(SET, FIRSTFLG),
    ADDRESS_PLUS_ONE(LO6ZEROS),
    STORE(BDT),
    ORDERS(STQ, BOF),
    ADDRESS(QTEMP),
    SWITCH(BOF, XDELVFLG),
    ADDRESS(S40_1B),      /* "LAMBERT" */
    ORDERS(VLOAD, ABVAL), /* "EXTERNAL DELTA-V" */
    ADDRESS_PLUS_ONE(DELVSIN),
    STORE(DELVSAB), /* "COMPUTE FOR P30/P40 INTERFACE" */
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VTIG),
    STORE(VINIT),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RTIG),
    STOVL(UT), /* "UP IN UT" */
    ADDRESS_PLUS_ONE(RTIG),
    STORE(RINIT),
    ORDERS(VSQ, PDDL),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(DMPR, DDV),
    ADDRESS_PLUS_ONE(THETACON),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(DELVSAB),
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDER(DDV),
    ADDRESS_PLUS_ONE(F),
    STOVL_D(14),
    ADDRESS_PLUS_ONE(DELVSIN),
    ORDERS(DOT, VXSC),
    ADDRESS_PLUS_ONE(UT),
    ADDRESS_PLUS_ONE(UT),
    ORDERS(VSL2, PUSH), /* "(DELTAV.UP)UP SCALED AT 2(+7) P.D.L. 0" */
    ORDERS(BVSU, PDDL), /* "DELTA VP SCALED AT 2(+7) P.D.L. 6" */
    ADDRESS_PLUS_ONE(DELVSIN),
    ADDRESS_PLUS_ONE_D(14),
    ORDERS(SIN, PDVL),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UT),
    ORDERS(VXSC, STADR),
    AFTER_STADR(STOVL(VGTIG)), /* "UNIT(VP X UP)SIN(THETAT/2) IN VGTIG." */
    ORDERS(UNIT, PDDL),        /* "UNIT(DELTA VP) IN P.D.L. 6" */
    ADDRESS_PLUS_ONE_D(14),
    ORDERS(COS, VXSC),
    ORDERS(VAD, VXSC),
    ADDRESS_PLUS_ONE(VGTIG),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(VSL2, VAD),
    ORDER(STADR),
    AFTER_STADR(STORE(VGTIG)), /* "VG IGNITION SCALED AT 2(+7) M/CS" */
    ORDER(UNIT),
    STOVL(UT), /* "THRUST DIRECTION SCALED AT 2(+1)" */
    ADDRESS_PLUS_ONE(VGTIG),
    ORDERS(PUSH, SET),
    SWITCH(SET, AVFLAG),
    ORDER(CALL),
    ADDRESS(MIDGIM), /* "VGTIG IN LV COOR AT 2(+7)M/CS IN DELVLVC" */
    ORDER(GOTO),
    ADDRESS(QTEMP),
    /* S40.1B */ ORDERS(DLOAD, DSU), /* "LAMBERT" */
    ADDRESS_PLUS_ONE(TIG),
    ADDRESS_PLUS_ONE(TWODT),
    STODL(TDEC1),
    ADDRESS_PLUS_ONE(TPASS4),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(TDEC1),
    STCALL(DELLT4),
    ADDRESS(AGAIN),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VIPRIME),
    STODL(UT),
    ADDRESS_PLUS_ONE(TIG),
    STORE(TDEC1),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(TPASS4),
    STCALL(DELLT4),
    ADDRESS(AGAIN),
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(DELVEET3),
    STORE(VGTIG),
    ORDERS(SET, CALL),
    SWITCH(SET, AVFLAG),
    ADDRESS(MIDGIM),
    ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(CALCUT),
};

/* AGAIN: the return in QTEMP1 (STQ); the state vector at TDEC1
 * (THISPREC), X2 and X1 kept in RTX2 and RTX1; RATT to RTIG and RINIT,
 * VATT to VTIG and VINIT; INITVEL, Lambert's velocity, with the
 * tolerance EP4(45)H, or EP4(10)H with NORMSW clear; back through
 * QTEMP1.
 *
 * CALCUT: the gravity over the step (CALCGRAV, "GDELTAT IN MPAC AT 2(+7)
 * M/CS") and the change of the velocity needed over it, both per
 * centisecond (200CS); B times CSTEER, "B.C SCALED AT 2(-15) PDL 12D";
 * from it and the unit of VGTIG, "UG", the part across UG ("Q"); the
 * thrust per unit mass squared, F over WEIGHT/G; UT, the unit of Q plus
 * UG times the root of the difference of their squares; back through
 * QTEMP. */
static const word again_words[] = {
    ORDERS(STQ, CALL),
    ADDRESS(QTEMP1),
    ADDRESS(THISPREC),
    ORDERS(SXA_2, SXA_1),
    ADDRESS(RTX2),
    ADDRESS(RTX1),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STORE(RTIG),
    STOVL(RINIT),
    ADDRESS_PLUS_ONE(VATT),
    STORE(VTIG),
    STORE(VINIT),
    ORDERS(SETPD, SLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDERS(PDDL, BON),
    ADDRESS_PLUS_ONE(EP4_45H),
    SWITCH(BON, NORMSW),
    ADDRESS_AT(AGAIN, 026), /* +3 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(EP4_10H),
    ORDERS(PUSH, CALL),
    ADDRESS(INITVEL),
    ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(QTEMP1),
    /* CALCUT */ ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(RTIG),
    ADDRESS(CALCGRAV), /* "GDELTAT IN MPAC AT 2(+7) M/CS" */
    ORDERS(VSL1, V_SC),
    ADDRESS_PLUS_ONE(200CS), /* "G AT 2(-5) M/CS. CS" */
    ORDERS(PDVL, VSU),
    ADDRESS_PLUS_ONE(VIPRIME),
    ADDRESS_PLUS_ONE(UT),
    ORDERS(V_SC, VSU),
    ADDRESS_PLUS_ONE(200CS),
    ORDERS(VXSC, VSL2),
    ADDRESS_PLUS_ONE(CSTEER),
    STOVL_D(12), /* "B.C SCALED AT 2(-15) PDL 12D" */
    ADDRESS_PLUS_ONE(VGTIG),
    ORDERS(UNIT, PUSH), /* "UG PDL 0 SCALED AT 2(+1)" */
    ORDERS(DOT, VXSC),
    ADDRESS_PLUS_ONE_D(12),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VSL2, BVSU),
    ADDRESS_PLUS_ONE_D(12),
    STODL_D(12), /* "Q PDL SCALED AT 2(-5)" */
    ADDRESS_PLUS_ONE(F),
    ORDERS(SRR, DDV),
    SRR_COUNT(4),
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(DSQ, PDVL), /* "F/MASS SQUARED PDL 6 AT 2(-10M/(CS.CS)" */
    ADDRESS_PLUS_ONE_D(12),
    ORDER(VSQ),
    ORDERS(BDSU, SQRT),
    ORDERS(VXSC, VSL1),
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE_D(12),
    STCALL(UT),
    ADDRESS(QTEMP),
};

/* ---- S40.2,3 -- pp. 714-717 ---- */

/* S40.2,3: POINTVSM, "THRUST IN SM AT 2" (UT by REFSMMAT); the return in
 * QTEMP (STQ). An RCS burn (ENG2FLAG set), S40.2,3B. An SPS burn: the
 * trim angles (YACTOFF, PACTOFF, at TRIMSCAL) and the mechanical biases
 * (YBIAS, PBIAS), their sines and cosines, give the engine bell's axis
 * in spacecraft coordinates (XSCREF, then SCAXIS, "ENGINE BELL SCALED AT
 * 2") and new spacecraft axes; the preferred IMU orientation, those axes
 * in reference coordinates from UT and the unit of UT cross RTIG (or
 * TSTRXUT's), "X OF PREF. IMU,X OF SC IN REF COOR. AT 2" and the same for
 * Y and Z; back through QTEMP.
 *
 * S40.2,3B: SCAXIS +X; XSCREF UT, YSCREF the unit of UT cross RTIG (or
 * TSTRXUT's), ZSCREF, "ZNB AXIS IN REF COOR", and back through QTEMP.
 *
 * TSTRXUT: with UT cross RTIG long enough (BHIZ), it (from 6D) and back
 * (RVQ); else BADVCTOR, the unit of the unit of UT cross (RTIG plus VTIG
 * shifted), complemented, in 6D. */
static const word s40_2_3_words[] = {
    ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(UT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(VSL1, STQ),
    ADDRESS(QTEMP),
    STORE(POINTVSM), /* "THRUST IN SM AT 2" */
    ORDERS(SETPD, BON),
    ADDRESS_PLUS_ONE_D(0),
    SWITCH(BON, ENG2FLAG),
    ADDRESS(S40_2_3B),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDERS(PUSH, SLOAD), /* "ZERO PDL 0" */
    ADDRESS_PLUS_ONE(YACTOFF),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(TRIMSCAL),
    ORDERS(DAD, PUSH),
    ADDRESS_PLUS_ONE(YBIAS),
    ORDERS(COS, PDDL), /* "COS(Y +Y0) PDL 2" */
    ORDERS(SIN, PUSH), /* "SIN(Y +Y0) PDL 4" */
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(PACTOFF),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(TRIMSCAL),
    ORDERS(DAD, PUSH),
    ADDRESS_PLUS_ONE(PBIAS),
    ORDERS(COS, PDDL), /* "COS(P +P0) PDL 6" */
    ORDERS(SIN, PUSH), /* "SIN(P +P0) PDL 8D" */
    STODL(ZSCREF),     /* "SIN(P +P0)" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DCOMP, PDDL), /* "-SIN(Y+Y0)COS(P+P0) PDL 10" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(2),
    ORDER(VDEF),
    STODL(XSCREF), /* "PD POINTER AT 6 NEW SC X AXIS SCALED AT" */
    ADDRESS_PLUS_ONE(ZSCREF),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(ZSCREF),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SL1, DCOMP),
    ORDER(VDEF),
    STODL(ZSCREF), /* "PD POINTER AT 4 NEW SC Z AXIS SCALED AT 2" */
    ORDER(VDEF),
    STODL(YSCREF), /* "PD POINTER AT 0 NEW SC Y AXIS SCALED AT 2" */
    ADDRESS_PLUS_ONE(ZSCREF),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(YSCREF),
    ADDRESS_PLUS_ONE(XSCREF),
    ORDER(VDEF),
    STOVL(SCAXIS), /* "ENGINE BELL SCALED AT 2" */
    ADDRESS_PLUS_ONE(UT),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(RTIG),
    ORDERS(VXV, VCOMP),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(UNIT, PUSH),
    ORDER(CALL),
    ADDRESS(TSTRXUT),
    ORDERS(VXV, VCOMP),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VSL1, PDVL), /* "2 RF/SC IN PDL 12D" */
    ADDRESS_PLUS_ONE(XSCREF),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE_D(0),
    STOVL(XSCREF), /* "X OF PREF. IMU,X OF SC IN REF COOR. AT 2" */
    ADDRESS_PLUS_ONE(YSCREF),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE_D(0),
    STOVL(YSCREF), /* "Y OF PREF. IMU,Y OF SC IN REF COOR. AT 2" */
    ADDRESS_PLUS_ONE(ZSCREF),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE_D(0),
    STORE(ZSCREF), /* "Z OF PREF. IMU,Z OF SC IN REF COOR. AT 2" */
    ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(QTEMP),
    /* S40.2,3B */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNITX),
    STOVL(SCAXIS),
    ADDRESS_PLUS_ONE(UT),
    STORE(XSCREF),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RTIG),
    STCALL_D(6),
    ADDRESS(TSTRXUT),
    STORE(YSCREF),
    ORDERS(VXV, VCOMP),
    ADDRESS_PLUS_ONE(XSCREF),
    ORDER(VSL1),
    STCALL(ZSCREF), /* "ZNB AXIS IN REF COOR" */
    ADDRESS(QTEMP),
    /* TSTRXUT */ ORDERS(DLOAD, BHIZ),
    ADDRESS_PLUS_ONE_D(36),
    ADDRESS(BADVCTOR),
    ORDERS(VLOAD, RVQ),
    ADDRESS_PLUS_ONE_D(6),
    /* BADVCTOR */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RTIG),
    ORDERS(PDVL, UNIT),
    ADDRESS_PLUS_ONE(VTIG),
    ORDERS(VSR3, VAD),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UT),
    ORDER(VCOMP),
    STORE_D(6),
    ORDER(RVQ),
};

/* ---- S41.1 -- pp. 717-718 ---- */

/* S41.1: the return in QTEMP (STQ); CDUTRIG; the velocity to be gained
 * (from the push-down list) by REFSMMAT to stable member coordinates,
 * *SMNB* to body axes, QUADROT and TENBNK14 to control coordinates, "VG
 * IN CONTROL COORDS. IN MPAC AT 2(+7)"; back through QTEMP. */
static const word s41_1_words[] = {
    ORDERS(STQ, CALL),
    ADDRESS(QTEMP),
    ADDRESS(CDUTRIG),
    ORDER(VLOAD),
    ORDERS(MXV, CALL),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ADDRESS(STAR_SMNB_STAR),
    ORDERS(MXV, VXSC),
    ADDRESS_PLUS_ONE(QUADROT),
    ADDRESS_PLUS_ONE(TENBNK14),
    ORDERS(VSL5, GOTO), /* "VG IN CONTROL COORDS. IN MPAC AT 2(+7)" */
    ADDRESS(QTEMP),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S40_1_CODE = {016u, S40_1, COUNT(s40_1_words), s40_1_words};
const struct interpretive_code AGAIN_CODE = {030u, AGAIN, COUNT(again_words), again_words};
const struct interpretive_code S40_2_3_CODE = {024u, S40_2_3, COUNT(s40_2_3_words), s40_2_3_words};
const struct interpretive_code S41_1_CODE = {022u, S41_1, COUNT(s41_1_words), s41_1_words};

#endif
