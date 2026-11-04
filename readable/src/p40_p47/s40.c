/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 2 (Luminary099 pp. 764-772, bank 27): S40.1, S40.2,3
 * and S40.8, interpretive code in the listing's notation, and the basic
 * code of S40.8's alarm. The faithful C is c64-port/src/lm_p40_p47.c.
 *
 * S40.1, which P40, P41 and P42 call before the burn: "COMPUTE INITIAL
 * THRUST DIRECTION(UT) AND INITIAL VALUE OF VG", for an external delta V
 * (XDELVFLG set) or for an aim point (S40.1B, by LEMPREC and INITVEL).
 * S40.2,3 turns the thrust direction to the stable member (POINTVSM) and
 * works out the preferred orientation, XSCREF, YSCREF and ZSCREF. S40.8,
 * which UPDATEVG calls each SERVICER pass of the burn, brings the velocity
 * to be gained (VG) up to date, and the time to go (TGO) with it.
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC. */
#include "p40_p47.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- S40.1 -- pp. 764-767 ---- */

/* S40.1: the return kept in QTEMP (STQ), TIG kept in TIGSAVE. With
 * XDELVFLG set (DELVTEST), CALCTHET: from the delta V asked for
 * (DELVSIN, DELVSAB) and the state at TIG (RTIG, VTIG), UT the unit
 * normal to the orbit, UNIT(VTIG X RTIG), "UP IN UT", and in 14D the
 * angle THETAT the burn turns through (THETACON, with RTIG, the delta V,
 * the mass and the thrust). "VG IGNITION SCALED AT 2(+7) M/CS" is the
 * delta V in the orbit's plane turned by THETAT/2, with the part along
 * UP added back, and UT its unit vector, "THRUST DIRECTION SCALED AT
 * 2(+1)". Then the delta V in local vertical coordinates (GET.LVC), and
 * back through QTEMP. */
static const word s40_1_words[] = {
    ORDERS(STQ, DLOAD),
    ADDRESS(QTEMP),
    ADDRESS_PLUS_ONE(TIG),
    STORE(TIGSAVE),
    /* DELVTEST */
    ORDER(BOF),
    SWITCH(BOF, XDELVFLG),
    ADDRESS(S40_1B),
    /* CALCTHET */
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
    ORDERS(DMP, DDV),
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
    ORDERS(PUSH, CALL),
    ADDRESS(GET_LVC), /* "VGTIG IN LV COOR AT 2(+7) M/CS IN DELVLVC" */
    ORDER(GOTO),
    ADDRESS(QTEMP),
    /* S40.1B: an aim point. TDEC1 TIG, DELLT4 the time from TIG to the
     * intercept (TPASS4), the state at TIG from LEMPREC "FOR INITVEL" in
     * RTIG, RINIT, VTIG and VINIT, UNIT/R/ the unit of RTIG. INITVEL with
     * no iterations ("NUMIT = 0") and its cone, "EPSILON4 = 10 DEGREES OR
     * 45 DEGREES" (EPS1, or EPS2 more with NORMSW set); "VGTIG = VR - VN."
     * (DELVEET3), "UT = UNIT (VGTIG)", and the delta V in local vertical
     * coordinates (VGDISP, GET.LVC); back through QTEMP. */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TIG),
    STORE(TDEC1),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(TPASS4),
    STCALL(DELLT4), /* "INTERCEPT TIME -- TIG." */
    ADDRESS(LEMPREC),
    ORDERS(VLOAD, SETPD), /* "LOAD STATE VECTOR AT TIG FOR INITVEL." */
    ADDRESS_PLUS_ONE(RATT),
    ADDRESS_PLUS_ONE_D(0),
    STORE(RTIG),
    STORE(RINIT),
    ORDER(UNIT),
    STOVL(UNIT_R_),
    ADDRESS_PLUS_ONE(VATT),
    STORE(VTIG),
    STORE(VINIT),
    ORDERS(DLOAD, PDDL), /* "NUMIT = 0" */
    ADDRESS_PLUS_ONE(ZEROVECS),
    ADDRESS_PLUS_ONE(EPS1),
    ORDERS(BOF, DAD),
    SWITCH(BOF, NORMSW),
    ADDRESS(SMALLEPS),
    ADDRESS_PLUS_ONE(EPS2),
    /* SMALLEPS */
    ORDERS(PUSH, SXA_1),
    ADDRESS(RTX1),
    ORDERS(SXA_2, CALL),
    ADDRESS(RTX2),
    ADDRESS(INITVEL),
    ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(DELVEET3), /* "VGTIG = VR - VN." */
    STORE(VGTIG),
    ORDER(UNIT), /* "UT = UNIT (VGTIG)" */
    STODL(UT),
    ADDRESS_PLUS_ONE_D(36),
    STCALL(VGDISP),   /* "CONVERT VGTIG (IN PUSHLIST) TO LOCAL" */
    ADDRESS(GET_LVC), /* "VERTICAL COORDINATES." */
    ORDER(GOTO),
    ADDRESS(QTEMP),
};

/* ---- S40.2,3 -- pp. 768-769 ---- */

/* S40.2,3: "TRANSFORM THRUST DIRECTION TO STABLE-", "MEMBER FROM REFERENCE
 * COORDS (RESCALE).", into POINTVSM, "SAVE FOR "VECPOINT" ROUTINE
 * (LEMMANU)."; SCAXIS +X, "ASSUME +X BURN ALWAYS, EVEN FOR RCS." PLUSX:
 * "XSCREF = UT (DESIRED THRUST DIRECTION)", "YSCREF = UNIT(UT X RTIG)",
 * or with that too small (BHIZ) UNIT(XSCREF X VTIG) (FIXY); STORY:
 * "ZSCREF = - (YSCREF X XSCREF)", PFRATFLG set, and back (RVQ). */
static const word s40_2_3_words[] = {
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UT),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(POINTVSM),
    ADDRESS_PLUS_ONE(UNITX),
    STOVL(SCAXIS),
    ADDRESS_PLUS_ONE(UT),
    /* PLUSX */
    STORE(XSCREF),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(RTIG),
    ORDERS(PDDL, BHIZ),
    ADDRESS_PLUS_ONE_D(36), /* "TEST MAGNITUDE OF UT X RTIG" */
    ADDRESS(FIXY),          /* "IF SMALL, USE UT X VTIG AS YSC" */
    /* STORY */
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STORE(YSCREF)),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(XSCREF),
    ORDER(VCOMP),
    STORE(ZSCREF),
    ORDERS(SET, RVQ),
    SWITCH(SET, PFRATFLG),
    /* FIXY */
    ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(XSCREF),
    ADDRESS_PLUS_ONE(VTIG),
    ORDERS(UNIT, PUSH),
    ORDER(GOTO),
    ADDRESS(STORY),
};

/* ---- S40.8 -- pp. 770-772 ---- */

/* S40.8: for a Lambert burn (XDELVFLG clear) VG from RASTEER1, else
 * VGPREV less DELVREF. VGAIN*: VG, "VELOCITY TO BE GAINED SCALED AT (7)
 * M/CS", and in stable member coordinates UNFC/2; BDTOK: its magnitude
 * in VGDISP. TGOCALC: VGPREV VG. With STEERSW clear, back (through
 * QPRET). Otherwise, with DELVREF more than 90 degrees from VG, the
 * alarm (ALARMIT); else TGO from VG along DELVREF, the exhaust velocity
 * (VEX) and the tail-off time (TDECAY), and TIG PIPTIME plus TGO. Four
 * seconds or more to go, on at FINDCDUW -2, which steers; less, IMPULSW
 * set, STEERSW clear, and back through QPRET. ALARMIT: to basic code. */
static const word s40_8_words[] = {
    ORDER(BOF), /* "GENERATE VR IF NOT EXTERNAL DELTA-V BURN" */
    SWITCH(BOF, XDELVFLG),
    ADDRESS(RASTEER1),
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(VGPREV),
    ADDRESS_PLUS_ONE(DELVREF),
    /* VGAIN* */
    STORE(VG),
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(UNFC_2),
    /* BDTOK */
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(VG),
    STORE(VGDISP),
    /* TGOCALC */
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VG),
    STOVL(VGPREV),
    ADDRESS_PLUS_ONE(DELVREF),
    ORDERS(BOF, VCOMP),
    SWITCH(BOF, STEERSW),
    ADDRESS(QPRET),
    ORDER(UNIT),
    ORDERS(DOT, PUSH),
    ADDRESS_PLUS_ONE(VG),
    ORDERS(BPL, DDV),
    ADDRESS(ALARMIT), /* "DELV IS MORE THAN 90 DEGREES FROM VG." */
    ADDRESS_PLUS_ONE(VEX),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(DPHALF),
    ORDERS(SR, DDV),
    SR_COUNT(10),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(MFOURDT),
    ADDRESS_PLUS_ONE(TDECAY),
    STORE(TGO),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(PIPTIME),
    STODL(TIG),
    ADDRESS_PLUS_ONE(TGO),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(FOURSECS), /* "400 CS" */
    ADDRESS_AT(FINDCDUW, -2),
    ORDERS(SET, CLRGO),
    SWITCH(SET, IMPULSW),
    SWITCH(CLRGO, STEERSW),
    ADDRESS(QPRET),
    /* ALARMIT */
    ORDER(EXIT),
};

/* ALARMIT, after the EXIT: alarm 1407 (TC ALARM, OCT 01407), */
void alarmit_after_exit(void)
{
    tc_at(ALARMIT + 1u);
    continue_at(ALARM);
}

/* and back to the interpreter, for ALARMIT_CODE. */
void alarmit_after_alarm(void)
{
    tc_at(ALARMIT + 3u);
    intpret();
}

/* ALARMIT_CODE: "SKIP TGO COMPUTATION BUT CALL FINDCDUW.", "FINDCDUW WILL
 * EXIT TO UPDATEVG +3." */
static const word alarmit_words[] = {
    ORDER(GOTO),
    ADDRESS_AT(FINDCDUW, -2),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S40_1_CODE = {027u, S40_1, COUNT(s40_1_words), s40_1_words};
const struct interpretive_code S40_2_3_CODE = {027u, S40_2_3, COUNT(s40_2_3_words), s40_2_3_words};
const struct interpretive_code S40_8_CODE = {027u, S40_8, COUNT(s40_8_words), s40_8_words};
const struct interpretive_code ALARMIT_CODE = {027u, ALARMIT + 4u, COUNT(alarmit_words),
                                               alarmit_words};

#endif
