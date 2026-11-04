/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, parts 4 and 5 (Comanche055 pp. 709-722): S40.1, S40.2,3
 * and S41.1, the burn's targeting, all interpretive code; and S40.8, the
 * steering during the burn. The faithful C is c64-port/src/cm_p40_p47.c.
 *
 * S40.1 (bank 16) finds the thrust direction (UT) and the velocity to be
 * gained at ignition (VGTIG): for an external delta V burn from the
 * delta V asked for (DELVSIN); for a Lambert burn (S40.1B) from the
 * velocity Lambert needs (AGAIN, bank 30) at ignition and two seconds
 * before, with CALCUT steering UT by the change. S40.2,3 (bank 24) turns
 * the engine bell's axis (SCAXIS), the preferred IMU orientation
 * (XSCREF, YSCREF, ZSCREF) and the thrust direction in stable member
 * coordinates (POINTVSM) out of UT. S41.1 (bank 22) turns the velocity
 * to be gained into control coordinates.
 *
 * S40.8 (banks 16 and 20), each pass of SERVICER during the burn, brings
 * the velocity to be gained up to date (VG), watches for the thrust
 * failing (LOTHRUST), computes the time to go (TGOCALC), and steers by
 * the cross product of VG and its change (XPRODUCT, OMEGAC); with less
 * than four seconds to go it sets up the engine's shutdown instead
 * (S40.81).
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC. */
#include "p40_p47_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

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

/* ---- S40.8 -- pp. 719-722 ---- */

/* S40.8, the cross-product steering, each pass of SERVICER during the
 * burn: the return in QTEMP (STQ); VG, the "VELOCITY-TO-BE-GAINED, SC.AT
 * B+7 M/CS", VGPREV plus BDT less DELVREF; VGDISP its length, "FOR
 * DISPLAY PURPOSES"; to basic code. */
static const word s40_8_words[] = {
    ORDERS(SETPD, STQ),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(QTEMP),
    ORDERS(VLOAD, BVSU), /* "CONSTRUCT DELVG, SC.AT B+7 M/CS" */
    ADDRESS_PLUS_ONE(DELVREF),
    ADDRESS_PLUS_ONE(BDT),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(VGPREV),
    STORE(VG), /* "VELOCITY-TO-BE-GAINED, SC.AT B+7 M/CS" */
    ORDER(ABVAL),
    STORE(VGDISP), /* "FOR DISPLAY PURPOSES" */
    ORDER(EXIT),
};

/* After the EXIT: restart group 5 (PHASCHNG, OCT 10035, "TYPE B RESTART
 * RESTART BELOW AND 5.3 REREADAC"), */
void s40_8_after_exit(void)
{
    tc_to(S40_8 + 014u, PHASCHNG);
}

/* and to the interpreter, for LOTHRUST_TEST_CODE. */
void s40_8_after_phaschng(void)
{
    tc_at(S40_8 + 016u);
    intpret();
}

/* LOTHRUST_TEST_CODE: VG to VGPREV. With STEERSW clear, back through
 * QTEMP ("SKIP TGO AND CROSS-PRODUCT"). "CHECK FOR LOTHRUST": the length
 * of DELVREF less the threshold (DVTHRESH), negative, LOTHRUST; else to
 * basic code. */
static const word lothrust_test_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VG),
    STORE(VGPREV),
    ORDERS(BOFF, VLOAD),
    SWITCH(BOFF, STEERSW), /* "SKIP TGO AND CROSS-PRODUCT" */
    ADDRESS(QTEMP),
    ADDRESS_PLUS_ONE(DELVREF),
    ORDERS(ABVAL, PUSH), /* "CHECK FOR LOTHRUST" */
    ORDERS(SLOAD, DMP),
    ADDRESS_PLUS_ONE(DVTHRESH), /* "SC.AT B-2 M/CS" */
    ADDRESS_PLUS_ONE(DPB_9),
    ORDER(BDSU),
    ORDERS(BMN, EXIT),
    ADDRESS(LOTHRUST),
};

/* After the EXIT: "ENABLE TVCDAP CG TRACKING", REPFRAC from EREPFRAC by
 * bit 14 of DAPDATR1 (CCS A, INDEX A: "LM-OFF, LM-ON VALUE"); to the
 * interpreter, for TGOCALC_CODE. */
void lothrust_test_after_exit(void)
{
    ca(ADDRESS_OF(DAPDATR1)); /* CAE: "ENABLE TVCDAP CG TRACKING" */
    mask(BIT14);
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(BIT1);
    ca(ADDRESS_OF(EREPFRAC) + A); /* INDEX A: "LM-OFF, LM-ON VALUE" */
    ts(ADDRESS_OF(REPFRAC));
    tc_at(TGOCALC - 1u);
    intpret();
}

/* TGOCALC_CODE: "GET DELVG"; with its unit along VG not negative ("ANGLE
 * SHOULD BE GREATER THAN PI/2", else INCRSVG), TGO from the length of
 * DELVG, the exhaust velocity (2VEXHUST), -FOURDT and the tail-off
 * (ETDECAY), "TIME TO GO IN CS. AT +28"; TIG, PIPTIME plus TGO. With
 * less than four seconds to go (FOURSEC), S40.81. XPRODUCT: the steering,
 * CSTEER times BDT less DELVREF, crossed with VG, in body axes (*SMNB*)
 * times KPRIMEDT; OMEGACLC: to OMEGAC; back through QTEMP. */
static const word tgocalc_words[] = {
    ORDERS(VLOAD, BVSU), /* "GET DELVG" */
    ADDRESS_PLUS_ONE(DELVREF),
    ADDRESS_PLUS_ONE(BDT),
    ORDER(UNIT),
    ORDERS(DOT, PUSH),
    ADDRESS_PLUS_ONE(VG),
    ORDERS(BPL, DDV), /* "ANGLE SHOULD BE GREATER THAN PI/2" */
    ADDRESS(INCRSVG),
    ADDRESS_PLUS_ONE(2VEXHUST),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(LODPHALF),
    ORDERS(NORM, SR1),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE_D(36), /* "(MAG DELVG)" */
    ADDRESS_PLUS_ONE(X2),
    ORDER(BDDV),
    ORDERS(XSU_2, SL_STAR),
    ADDRESS(X1),
    VSL_X2(-9), /* 0 -9D,2 */
    ORDERS(DMP, PUSH),
    ADDRESS_PLUS_ONE(MFOURDT),
    ORDERS(SLOAD, SR),
    ADDRESS_PLUS_ONE(ETDECAY), /* "ETDECAY SC.AT B+14 CS" */
    SR_COUNT(14),
    ORDERS(BDSU, STADR),
    AFTER_STADR(STORE(TGO)), /* "TIME TO GO IN CS. AT +28" */
    ORDER(DAD),
    ADDRESS_PLUS_ONE(PIPTIME),
    STODL(TIG),
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(FOURSEC),
    ADDRESS(S40_81),
    /* XPRODUCT */ ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(BDT),
    ADDRESS_PLUS_ONE(CSTEER),
    ORDERS(VSL2, VSU),
    ADDRESS_PLUS_ONE(DELVREF),
    ORDERS(UNIT, PDVL),
    ADDRESS_PLUS_ONE(VG),
    ORDERS(UNIT, VXV),
    ORDERS(MXV, CALL),
    ADDRESS_PLUS_ONE(REFSMMAT), /* "(REFSMMAT/2)" */
    ADDRESS(STAR_SMNB_STAR),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(KPRIMEDT),
    /* OMEGACLC */ STORE(OMEGAC),
    ORDER(GOTO),
    ADDRESS(QTEMP),
};

/* S40.81, "TGO LESS THAN 4 SECONDS": IMPULSW set, "FOR ENGINE-OFF CALL";
 * RATEZRO: OMEGAC zero, "TVC TO ATTITUDE HOLD"; to basic code. */
static const word s40_81_words[] = {
    ORDERS(SET, VLOAD),
    SWITCH(SET, IMPULSW), /* "FOR ENGINE-OFF CALL" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* RATEZRO */ STORE(OMEGAC), /* "TVC TO ATTITUDE HOLD" */
    ORDER(EXIT),
};

/* After the EXIT: CNTR POSMAX, "INHIBIT SWITCHOVER/TVC EG TRACKING"; to
 * the interpreter, for S40_81_END_CODE. */
void s40_81_after_exit(void)
{
    set_A(POSMAX); /* "INHIBIT SWITCHOVER/TVC EG TRACKING" */
    ts(ADDRESS_OF(CNTR));
    tc_at(S40_81 + 7u);
    intpret();
}

/* S40_81_END_CODE: STEERSW clear ("RESTARTS OK"), back through QTEMP. */
static const word s40_81_end_words[] = {
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, STEERSW), /* "RESTARTS OK" */
    ADDRESS(QTEMP),
};

/* INCRSVG: the "ALARM INDICATING THAT THRUST IS POINTING", to basic
 * code. */
static const word incrsvg_words[] = {
    ORDER(EXIT),
};

/* After the EXIT: alarm 1407 (TC ALARM, OCT 01407), */
void incrsvg_after_exit(void)
{
    tc_to(INCRSVG + 1u, ALARM);
}

/* and to the interpreter, for INCRSVG_END_CODE. */
void incrsvg_after_alarm(void)
{
    tc_at(INCRSVG + 3u);
    intpret();
}

/* INCRSVG_END_CODE: back through QTEMP. */
static const word incrsvg_end_words[] = {
    ORDER(GOTO),
    ADDRESS(QTEMP),
};

/* LOTHRUST, "THRUST FAILURE (LO-OR-NO) INDICATED": with IDLEFAIL set
 * ("SET BY V97P.  ALLOWS 1 BYPASS IN CASE OF"), back through QTEMP;
 * else "START OF ENGINE-FAIL (R40) OPERATIONS", OMEGAC zero, "PUT TVC
 * IN ATTITUDE HOLD"; to basic code. */
static const word lothrust_words[] = {
    ORDERS(BON, VLOAD), SWITCH(BON, IDLEFAIL),      /* "SET BY V97P.  ALLOWS 1 BYPASS IN CASE OF" */
    ADDRESS(QTEMP),     ADDRESS_PLUS_ONE(HI6ZEROS), /* "START OF ENGINE-FAIL (R40) OPERATIONS" */
    STORE(OMEGAC),                                  /* "PUT TVC IN ATTITUDE HOLD" */
    ORDER(EXIT),
};

/* After the EXIT: -0 (CS ZERO) to VCNTR, "KILL CSMMASS UPDATING", to
 * VCNTRTMP, to REPFRAC, "KILL TVCDAP CG TRIM TRACKING", and to NVWORD1,
 * "SET UP ENGINE-FAIL V97FLASH (CLOCKJOB)"; to the interpreter, for
 * LOTHRUST_END_CODE. */
void lothrust_after_exit(void)
{
    cs(ZERO_ADDRESS);
    ts(ADDRESS_OF(VCNTR)); /* "KILL CSMMASS UPDATING" */
    ts(ADDRESS_OF(VCNTRTMP));
    ts(ADDRESS_OF(REPFRAC)); /* "KILL TVCDAP CG TRIM TRACKING" */
    ts(ADDRESS_OF(NVWORD1)); /* "SET UP ENGINE-FAIL V97FLASH (CLOCKJOB)" */
    tc_at(LOTHRUST + 013u);
    intpret();
}

/* LOTHRUST_END_CODE: "INHIBIT STEERING AND TGO CALC" (STEERSW clear),
 * "RESTARTS OK"; back through QTEMP. */
static const word lothrust_end_words[] = {
    ORDERS(CLEAR, GOTO), /* "INHIBIT STEERING AND TGO CALC (MANUAL" */
    SWITCH(CLEAR, STEERSW),
    ADDRESS(QTEMP), /* "RESTARTS OK" */
};

/* ---- The interpretive code ---- */

const struct interpretive_code S40_1_CODE = {016u, S40_1, COUNT(s40_1_words), s40_1_words};
const struct interpretive_code AGAIN_CODE = {030u, AGAIN, COUNT(again_words), again_words};
const struct interpretive_code S40_2_3_CODE = {024u, S40_2_3, COUNT(s40_2_3_words), s40_2_3_words};
const struct interpretive_code S41_1_CODE = {022u, S41_1, COUNT(s41_1_words), s41_1_words};
const struct interpretive_code S40_8_CODE = {016u, S40_8, COUNT(s40_8_words), s40_8_words};
const struct interpretive_code LOTHRUST_TEST_CODE = {016u, S40_8 + 017u, COUNT(lothrust_test_words),
                                                     lothrust_test_words};
const struct interpretive_code TGOCALC_CODE = {016u, TGOCALC, COUNT(tgocalc_words), tgocalc_words};
const struct interpretive_code S40_81_CODE = {020u, S40_81, COUNT(s40_81_words), s40_81_words};
const struct interpretive_code S40_81_END_CODE = {020u, S40_81 + 010u, COUNT(s40_81_end_words),
                                                  s40_81_end_words};
const struct interpretive_code INCRSVG_CODE = {020u, INCRSVG, COUNT(incrsvg_words), incrsvg_words};
const struct interpretive_code INCRSVG_END_CODE = {020u, INCRSVG + 4u, COUNT(incrsvg_end_words),
                                                   incrsvg_end_words};
const struct interpretive_code LOTHRUST_CODE = {020u, LOTHRUST, COUNT(lothrust_words),
                                                lothrust_words};
const struct interpretive_code LOTHRUST_END_CODE = {020u, LOTHRUST + 014u,
                                                    COUNT(lothrust_end_words), lothrust_end_words};

#endif
