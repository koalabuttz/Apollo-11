/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, parts 10 and 11 (Luminary099 pp. 574-579): LSR22.3, R22's
 * measurement incorporation, from its start through the range rate's.
 *
 * LSR22.3 (bank 26) "COMPUTES THE B-VECTORS ADN" [sic] "DELTA Q FOR EACH
 * OF THE QUANTITIES MEASURED BY THE RENDEZVOUS RADAR. (RANGE, RANGE RATE,
 * SHAFT AND TRUNNION ANGLES)", and LGCUPDTE
 * (MEASUREMENT_INCORPORATION.agc) corrects the state vector with each.
 * First both vehicles' state vectors go to the mark time (INTGRCAL): the
 * one not being corrected (VEHUPFLG) without the W-matrix, the one being
 * corrected with it while RENDWFLG says it is valid; on the lunar surface,
 * LSR22.4 instead. A W-matrix gone invalid is set up again (WLINIT); off
 * the surface, R65 (R65LEM) runs first. X2 keeps the sphere of influence's
 * scaling (0 for the Moon, 2 for the Earth) in SCALSHFT.
 *
 * Then the range: its B-vector, the unit line of sight from the LM to the
 * CSM (GETULC), negative when the LM is corrected; its deviation, the
 * radar's range RM less the range between the state vectors; and its
 * variance, RANGEVAR times the range squared, no less than RVARMIN, in
 * VARIANCE (B-40). LGCUPDTE (QOK) takes it from there.
 *
 * Then the range rate (part 11): its B-vectors, B0 the part of the
 * relative velocity across the line of sight, (ULC X VLC) X ULC, and B1
 * the line of sight times the range; its deviation, the radar's range
 * rate RDOTM less the one from the state vectors, times the range; and its
 * variance, RATEVAR times the range rate squared, no less than VVARMIN,
 * times the range squared. All are shifted up together as far as the
 * larger of B0 and B1 allows, and LGCUPDTE takes them. On the surface
 * R22 is done (RENDEND); else the angles, in the next part.
 *
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- The state vectors to the mark time ---- */

/* LSR22.3 -- p. 575. "ARE WE ON LUNAR SURFACE": "YES", LSR22.4. Else
 * DMENFLG set, and the CSM's state vector is brought to the mark time
 * first when the LM's is to be corrected (VEHUPFLG clear, DOLEM: the
 * other way round). "LM PRECISION INTEGRATION" (VINTFLAG clear) of the
 * vehicle not corrected, then the one corrected: "CSM WITH W-MATRIX
 * INTEGRATION" (DIM0FLAG, D6OR9FLG) when RENDWFLG says the W-matrix is
 * valid, else NOTWCSM; STATEFLG set, so its state vector is the one
 * updated. "LM WITH W-MATRIX INTEGRATION" likewise from DOLEM. Then
 * MARKTEST: "HAS W-MATRIX BEEN INVALIDATED", "YES -- REINITIALIZE"
 * (WLINIT); RANGEBQ, "DON'T CALL R65 IF ON SURFACE". */
static const word lsr22_3_words[] = {
    /* LSR22.3 */ ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(BON, SET),
    SWITCH(BON, SURFFLAG), /* "ARE WE ON LUNAR SURFACE" */
    ADDRESS(LSR22_4),      /* "YES" */
    SWITCH(SET, DMENFLG),
    ORDERS(BOFF, CALL),
    SWITCH(BOF, VEHUPFLG),
    ADDRESS(DOLEM),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, CALL), /* "LM PRECISION INTEGRATION" */
    SWITCH(CLEAR, VINTFLAG),
    ADDRESS(SETIFLGS),
    ORDER(CALL),
    ADDRESS(INTGRCAL),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, BOFF),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(BOF, RENDWFLG),
    ADDRESS(NOTWCSM),
    ORDERS(SET, SET), /* "CSM WITH W-MATRIX INTEGRATION" */
    SWITCH(SET, DIM0FLAG),
    SWITCH(SET, D6OR9FLG),
    /* NOTWCSM */ ORDERS(SET, CLEAR),
    SWITCH(SET, VINTFLAG),
    SWITCH(CLEAR, INTYPFLG),
    ORDERS(SET, CALL),
    SWITCH(SET, STATEFLG),
    ADDRESS(INTGRCAL),
    ORDER(GOTO),
    ADDRESS(MARKTEST),
    /* DOLEM */ ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(SET, CALL),
    SWITCH(SET, VINTFLAG),
    ADDRESS(SETIFLGS),
    ORDER(CALL),
    ADDRESS(INTGRCAL),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, BOFF),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(BOF, RENDWFLG),
    ADDRESS(NOTWLEM),
    ORDERS(SET, SET), /* "LM WITH W-MATRIX INTEGRATION" */
    SWITCH(SET, DIM0FLAG),
    SWITCH(SET, D6OR9FLG),
    /* NOTWLEM */ ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, INTYPFLG),
    SWITCH(CLEAR, VINTFLAG),
    ORDERS(SET, CALL),
    SWITCH(SET, STATEFLG),
    ADDRESS(INTGRCAL),
    /* MARKTEST */ ORDERS(BON, CALL), /* "HAS W-MATRIX BEEN INVALIDATED" */
    SWITCH(BON, RENDWFLG),
    ADDRESS(RANGEBQ),
    ADDRESS(WLINIT),                 /* "YES -- REINITIALIZE" */
    /* RANGEBQ */ ORDERS(BON, EXIT), /* "DON'T CALL R65 IF ON SURFACE" */
    SWITCH(BON, SURFFLAG),
    ADDRESS(RANGEBQ1),
};

/* RANGEBQ +3: R65 off the surface, its count (R65CNTR) from zero. (A +0
 * never overflows: the TS does not skip.) */
void rangebq_r65(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(R65CNTR));
    tc_to(RANGEBQ + 5u, BANKCALL); /* CADR R65LEM */
}

void rangebq_after_r65(void)
{
    tc_at(RANGEBQ + 7u);
    intpret();
}

/* ---- The range ---- */

/* RANGEBQ1 -- p. 576: X2 0, or 2 outside the Moon's sphere of influence
 * (LMOONFLG clear): "0 -- MOON. 2 -- EARTH." in SCALSHFT. "STORE RANGE
 * CODE (1) FOR R3 IN NOUN 49" (WHCHREAD); "GET SINGLE PRECISION RVARMIN
 * (B-12)", "SHIFT TO TRIPLE PRECISION (B-40)", "AND SAVE IN 20D". "BEGIN
 * COMPUTING THE B-VECTORS, DELTAQ": "B-VECTORS FOR RANGE", the unit line
 * of sight (GETULC); "B0, COMP. IF LM BEING CORRECTED", and B1 and B2
 * zero. The deviation: "RM - (MAGNITUDE RCSM-RLM)", shifted "FROM
 * EARTH/MOON SPHERE TO B-29" and back ("EARTH B-29. MOON B-27") in DELTAQ.
 * The variance: "RLC B-29/B-27" (36D), "NORMALIZE AND SQUARE", "MULTIPLY
 * BY RANGEVAR (B12) THEN UNNORMALIZE" (by X1, the normalizing shift, and
 * twice by X2), to triple precision in VARIANCE ("B-40"); if it is less
 * than RVARMIN (20D), RVARMIN. Then LGCUPDTE (QOK). */
static const word rangebq1_words[] = {
    /* RANGEBQ1 */ ORDERS(AXT_2, BON), /* "CLEAR X2" */
    NUMBER(0),
    SWITCH(BON, LMOONFLG), /* "IS MOON SPHERE OF INFLUENCE" */
    ADDRESS(SETX2),        /* "YES. STORE ZERO IN SCALSHFT REGISTER" */
    ORDER(INCR_2),
    NUMBER(2),
    /* SETX2 */ ORDERS(SXA_2, CALL),
    ADDRESS(SCALSHFT), /* "0 -- MOON. 2 -- EARTH." */
    ADDRESS(GRP2PC),
    ORDERS(AXT_1, SXA_1), /* "STORE RANGE CODE (1) FOR R3 IN NOUN 49" */
    NUMBER(1),
    ADDRESS(WHCHREAD),
    ORDERS(SLOAD, SR),         /* "GET SINGLE PRECISION RVARMIN (B-12)" */
    ADDRESS_PLUS_ONE(RVARMIN), /* "SHIFT TO TRIPLE PRECISION (B-40)" */
    SR_COUNT(28),
    ORDER(RTB),
    ADDRESS(TPMODE), /* "AND SAVE IN 20D" */
    STORE_D(20),
    ORDER(CALL),        /* "BEGIN COMPUTING THE B-VECTORS, DELTAQ" */
    ADDRESS(GETULC),    /* "B-VECTORS FOR RANGE" */
    ORDERS(BON, VCOMP), /* "B0, COMP. IF LM BEING CORRECTED" */
    SWITCH(BON, VEHUPFLG),
    ADDRESS_AT(LSR22_3, 0135), /* "+1": past the VCOMP */
    STOVL(BVECTOR),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_AT(BVECTOR, 6),  /* "B1" */
    STODL_AT(BVECTOR, 12), /* "B2" */
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(SRR_STAR, BDSU),
    SRR_X2(2),            /* "SHIFT FROM EARTH/MOON SPHERE TO B-29" */
    ADDRESS_PLUS_ONE(RM), /* "RM - (MAGNITUDE RCSM-RLM)" */
    ORDER(SLR_STAR),
    SLR_X2(2),              /* "SHIFT TO EARTH/MOON SPHERE" */
    STODL(DELTAQ),          /* "EARTH B-29. MOON B-27" */
    ADDRESS_PLUS_ONE_D(36), /* "RLC B-29/B-27" */
    ORDERS(NORM, DSQ),      /* "NORMALIZE AND SQUARE" */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMP, SR_STAR),
    ADDRESS_PLUS_ONE(RANGEVAR), /* "MULTIPLY BY RANGEVAR (B12) THEN" */
    SR_X1(-2),                  /* "UNNORMALIZE" */
    ORDERS(SR_STAR, SR_STAR),
    SR_X1(0),
    SR_X2(0),
    ORDERS(SR_STAR, RTB),
    SR_X2(0),
    ADDRESS(TPMODE),
    STORE(VARIANCE), /* "B-40" */
    ORDERS(DCOMP, TAD),
    ADDRESS_PLUS_ONE_D(20), /* "B-40" */
    ORDERS(BMN, TLOAD),
    ADDRESS(QOK),
    ADDRESS_PLUS_ONE_D(20), /* "B-40" */
    STORE(VARIANCE),
    /* QOK */ ORDER(CALL),
    ADDRESS(LGCUPDTE),
};

/* ---- The range rate ---- */

/* QOK +2 -- pp. 577-579: "STORE R-RATE CODE (2) FOR R3 IN NOUN 49"
 * (WHCHREAD); "B-VECTOR, DELTAQ FOR RANGE RATE", the unit line of sight
 * again (GETULC, ULC to the push-down list). "GET RLC SCALED B-29/B-27"
 * "AND SHIFT TO B-23" (by X2, the sphere's scaling) "THEN STORE BACK IN
 * 36D". "B1, COMP. IF LM BEING CORRECTED": "B1 = RLC (B-24/B-22)", the
 * line of sight times the range. The LM's velocity (NUVLEM, VCVLEM) and
 * the CSM's (NUVCSM, VCVCSM), each "SHIFT FOR EARTH/MOON SPHERE": "VC - VL
 * = VLC TO PD6". The range rate, VLC dotted with ULC: "RDOT B-8/B-6 TO
 * PD12", "SHIFT FROM EARTH/MOON SPHERE TO B-8"; "RDOT**2 B-16 X RATEVAR
 * B12" in VARIANCE. "GET SINGLE PRECISION VVARMIN (B+12)", "SHIFT TO DP
 * (B-4)" "AND SAVE IN 24D"; "IS MIN. VARIANCE > COMPUTED VARIANCE":
 * "BRANCH -- NO" (VOK), "YES -- USE MINIMUM VARIANCE". */
static const word range_rate_words[] = {
    ORDERS(SSP, CALL),
    ADDRESS_PLUS_ONE(WHCHREAD),
    NUMBER(2), /* "STORE R-RATE CODE (2) FOR R3 IN NOUN 49" */
    ADDRESS(GRP2PC),
    ORDER(CALL), /* "B-VECTOR, DELTAQ FOR RANGE RATE" */
    ADDRESS(GETULC),
    ORDERS(PDDL, SR_STAR),  /* "GET RLC SCALED B-29/B-27" */
    ADDRESS_PLUS_ONE_D(36), /* "AND SHIFT TO B-23" */
    SR_X2(-4),
    STOVL_D(36),        /* "THEN STORE BACK IN 36D" */
    ORDERS(BON, VCOMP), /* "B1, COMP. IF LM BEING CORRECTED" */
    SWITCH(BON, VEHUPFLG),
    ADDRESS_AT(QOK, 017), /* "+1": past the VCOMP */
    ORDER(VXSC),
    ADDRESS_PLUS_ONE_D(36), /* "B1 = RLC (B-24/B-22)" */
    STOVL_AT(BVECTOR, 6),
    ADDRESS_PLUS_ONE(NUVLEM),
    ORDERS(VSR_STAR, VAD),
    SR_X2(6),                 /* "SHIFT FOR EARTH/MOON SPHERE" */
    ADDRESS_PLUS_ONE(VCVLEM), /* "EARTH B-7. MOON B-5" */
    ORDERS(PDVL, VSR_STAR),   /* "VL TO PD6" */
    ADDRESS_PLUS_ONE(NUVCSM),
    SR_X2(6), /* "SHIFT FOR EARTH/MOON SPHERE" */
    ORDERS(VAD, VSU),
    ADDRESS_PLUS_ONE(VCVCSM),
    ORDERS(PDVL, DOT), /* "VC - VL = VLC TO PD6" */
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(PUSH, SRR_STAR), /* "RDOT B-8/B-6 TO PD12" */
    SRR_X2(2),              /* "SHIFT FROM EARTH/MOON SPHERE TO B-8" */
    ORDERS(DSQ, DMPR),      /* "RDOT**2 B-16 X RATEVAR B12" */
    ADDRESS_PLUS_ONE(RATEVAR),
    STORE(VARIANCE),
    ORDERS(SLOAD, SR),
    ADDRESS_PLUS_ONE(VVARMIN), /* "GET SINGLE PRECISION VVARMIN (B+12)" */
    SR_COUNT(16),              /* "SHIFT TO DP (B-4)" */
    STORE_D(24),               /* "AND SAVE IN 24D" */
    ORDERS(DSU, BMN),          /* "IS MIN. VARIANCE > COMPUTED VARIANCE" */
    ADDRESS_PLUS_ONE(VARIANCE),
    ADDRESS(VOK), /* "BRANCH -- NO" */
    ORDER(DLOAD), /* "YES -- USE MINIMUM VARIANCE" */
    ADDRESS_PLUS_ONE_D(24),
    STORE(VARIANCE),
    /* VOK -- the deviation: "RDOT (PD12) FROM B-8/B-6" "TO B-10/B-8";
     * RDOTM "SHIFT TO EARTH/MOON SPHERE" ("B-7 TO B-10/B-8"), less RDOT,
     * times RLC: DELTAQ, "B-33". "NOW GET B0": "(ULC X VLC) X ULC",
     * "B0, COMP. IF LM BEING CORRECTED", "SCALED B-5"; B2 zero, and "ZERO
     * OUT 20 TO 25 IN PUSHLIST". */
    /* VOK */ ORDERS(DLOAD, SR2), /* "RDOT (PD12) FROM B-8/B-6" */
    ORDERS(PDDL, SLR_STAR),       /* "TO B-10/B-8" */
    ADDRESS_PLUS_ONE(RDOTM),      /* "SHIFT TO EARTH/MOON SPHERE" */
    SLR_X2(-1),                   /* "B-7 TO B-10/B-8" */
    ORDER(DSU),
    ORDER(DMPR),
    ADDRESS_PLUS_ONE_D(36),
    STOVL(DELTAQ),         /* "B-33" */
    ADDRESS_PLUS_ONE_D(0), /* "NOW GET B0" */
    ORDERS(VXV, VXV),      /* "(ULC X VLC) X ULC" */
    ORDERS(BON, VCOMP),    /* "B0, COMP. IF LM BEING CORRECTED" */
    SWITCH(BON, VEHUPFLG),
    ADDRESS_AT(QOK, 072), /* "+1": past the VCOMP */
    ORDER(VSR_STAR),
    SR_X2(-2), /* "SCALED B-5" */
    STOVL(BVECTOR),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_D(20), /* "ZERO OUT 20 TO 25 IN PUSHLIST" */
    STOVL_AT(BVECTOR, 12),
    ADDRESS_PLUS_ONE(BVECTOR),
    /* The shift: "LOAD B0, GET MAGNITUDE AND NORMALIZE", "SHIFT COUNT IN
     * 20D"; B1's in 22D; "FIND WHICH SHIFT IS SMALLER", X1 that one (the
     * listing's "LOAD X2 WITH THE SMALLER SHIFT COUNT" means X1). "THEN
     * ADJUST B0, B1, DELTAQ AND VARIANCE", the variance by twice the
     * count, as "MULTIPLY RLC**2 BY VARIANCE" does with RLC shifted by
     * it, and "SHIFT TO CONFORM TO BVECTORS AND DELTAQ" (to triple
     * precision, TPMODE). "AND STORE TP VARIANCE": LGCUPDTE. Then, on
     * the surface ("ARE ANGLES TO BE DONE": "NO"), RENDEND. */
    ORDERS(ABVAL, NORM),    /* "LOAD B0, GET MAGNITUDE AND NORMALIZE" */
    ADDRESS_PLUS_ONE_D(20), /* "SHIFT COUNT IN 20D" */
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE_AT(BVECTOR, 6), /* "LOAD B1, GET MAGNITUDE AND NORMALIZE" */
    ORDERS(NORM, DLOAD),
    ADDRESS_PLUS_ONE_D(22), /* "SHIFT COUNT IN 22D" */
    ADDRESS_PLUS_ONE_D(22), /* "FIND WHICH SHIFT IS SMALLER" */
    ORDERS(DSU, BMN),       /* "BRANCH -- B0 HAS A SMALLER SHIFT COUNT" */
    ADDRESS_PLUS_ONE_D(20),
    ADDRESS(VOK1),
    ORDERS(LXA_1, GOTO),
    ADDRESS_D(22), /* "LOAD X2 WITH THE SMALLER SHIFT COUNT" */
    ADDRESS(VOK2),
    /* VOK1 */ ORDER(LXA_1),
    ADDRESS_D(20),
    /* VOK2 */ ORDERS(VLOAD, VSL_STAR), /* "THEN ADJUST B0, B1, DELTAQ AND VARIANCE" */
    ADDRESS_PLUS_ONE(BVECTOR),          /* "WITH THI SSHIFT COUNT" */
    SL_X1(0),
    STOVL(BVECTOR),
    ADDRESS_PLUS_ONE_AT(BVECTOR, 6),
    ORDER(VSL_STAR),
    SL_X1(0),
    STODL_AT(BVECTOR, 6),
    ADDRESS_PLUS_ONE(DELTAQ),
    ORDER(SL_STAR),
    SL_X1(0),
    STORE(DELTAQ),
    ORDERS(DLOAD, SL_STAR), /* "GET RLC AND ADJUST FOR SCALE SHIFT" */
    ADDRESS_PLUS_ONE_D(36),
    SL_X1(-1),
    ORDERS(DSQ, DMP), /* "MULTIPLY RLC**2 BY VARIANCE" */
    ADDRESS_PLUS_ONE(VARIANCE),
    ORDERS(SL4, RTB), /* "SHIFT TO CONFORM TO BVECTORS AND DELTAQ" */
    ADDRESS(TPMODE),
    STCALL(VARIANCE), /* "AND STORE TP VARIANCE" */
    ADDRESS(LGCUPDTE),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(BON, EXIT), /* "ARE ANGLES TO BE DONE" */
    SWITCH(BON, SURFFLAG),
    ADDRESS(RENDEND), /* "NO" */
};

/* ---- The interpretive code ---- */

const struct interpretive_code LSR22_3_CODE = {026u, LSR22_3, COUNT(lsr22_3_words), lsr22_3_words};
const struct interpretive_code RANGEBQ1_CODE = {026u, RANGEBQ + 010u, COUNT(rangebq1_words),
                                                rangebq1_words};
const struct interpretive_code RANGE_RATE_CODE = {026u, QOK + 2u, COUNT(range_rate_words),
                                                  range_rate_words};

#endif
