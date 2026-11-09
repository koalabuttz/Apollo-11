/* SPDX-License-Identifier: MIT */
/* MEASUREMENT_INCORPORATION.agc (Luminary099 pages 1149-1158,
 * Comanche055 pages 1252-1261): its interpretive code in the listing's
 * notation (src/interpreter/notation.h), which the readable interpreter
 * runs (src/binding/measurement_inc_routines.c). The faithful C is
 * c64-port/src/lm_measurement_inc.c and cm_measurement_inc.c.
 *
 * W, the error transition matrix, is 9x9 (6x6 when DMENFLG is clear), kept
 * as rows of vectors: its first three columns' block from W, the next from
 * W +54D, the last from W +108D, each 18 words a row group. The work
 * registers X1, X2 (index registers) and S1, S2 (their steps) walk it. */
#include "measurement_inc.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word incorp1[] = {
    /* INCORP1 -- p. 1149 (CM p. 1252).
     * In: "DMENFLG = 0 (6-DIMENSIONAL BVECTOR), =1 (9-DIMENSIONAL)", "W =
     * ERROR TRANSITION MATRIX 6X6 OR 9X9", "VARIANCE = VARIANCE (SCALAR)",
     * "DELTAQ = MEASURED DEVIATION (SCALAR)" and "BVECTOR = 6 OR 9
     * DIMENSIONAL BVECTOR". Out: DELTAX, the "STATE VECTOR DEVIATIONS", ZI,
     * the "VECTOR USED FOR THE INCORPORATION", GAMMA and OMEGA, the "OMEGA
     * WEIGHTING VECTOR".
     *
     * The return to EGRESS (STQ). Z123: ZI's three vectors, each the
     * BVECTOR's three vectors times three of W's blocks (MXV*), X1 stepping
     * W down 18 words a row group and X2 ZI along; the third left zero for
     * a 6-dimensional W. */
    /* INCORP1 */ ORDER(STQ),
    ADDRESS(EGRESS),
    ORDERS(AXT_1, SSP),
    NUMBER(54),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(18), /* "IX1 = 54 S1= 18" */
    ORDERS(AXT_2, SSP),
    NUMBER(18),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(6), /* "IX2 = 18 S2=6" */
    /* Z123 */ ORDERS(VLOAD, MXV_STAR),
    ADDRESS_PLUS_ONE(BVECTOR), /* "BVECTOR (0)" */
    ADDRESS_PLUS_ONE_AT_X1(W, 54),
    STORE_AT_X2(ZI, 18),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(BVECTOR, 6), /* "BVECTOR (1)" */
    ORDERS(MXV_STAR, VAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(W, 108),
    ADDRESS_PLUS_ONE_AT_X2(ZI, 18),
    STORE_AT_X2(ZI, 18),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(BVECTOR, 12), /* "BVECTOR (2)" */
    ORDERS(MXV_STAR, VAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(W, 162),
    ADDRESS_PLUS_ONE_AT_X2(ZI, 18), /* "B(0)*W+B(1)*(W+54)+B(2)*(W+108) FIRST PASS" */
    STORE_AT_X2(ZI, 18),            /* "ZI THEN Z2 THEN Z3" */
    ORDER(TIX_1),
    ADDRESS(INCOR1),
    /* INCOR1 */ ORDERS(TIX_2, BON),
    ADDRESS(Z123), /* "LOOP FOR Z1,Z2,Z3" */
    SWITCH(BON, DMENFLG),
    ADDRESS(INCOR1A),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_AT(ZI, 12),

    /* INCOR1A -- pp. 1150-1151 (CM pp. 1253-1254).
     * A = ZI*ZI + Z2*Z2 + Z3*Z3 + VARIANCE in triple precision (TRIPA; TPMODE
     * makes each square a triple), and GAMMA = 1 / (A + SQRT(A * VARIANCE)),
     * with VARIANCE and A shifted up as far as they go (the count in X2,
     * then NORMGAM, the normalization count for GAMMA). Then DELTAQ / A on
     * the push-down list, both normalized, and NEWZCOMP. */
    /* INCOR1A */ ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(ZI),
    ORDERS(VSQ, RTB),
    ADDRESS(TPMODE),
    ORDERS(PDVL, VSQ),
    ADDRESS_PLUS_ONE_AT(ZI, 6),
    ORDERS(RTB, TAD),
    ADDRESS(TPMODE),
    ORDERS(PDVL, VSQ),
    ADDRESS_PLUS_ONE_AT(ZI, 12),
    ORDERS(RTB, TAD),
    ADDRESS(TPMODE),
    ORDERS(TAD, AXT_2),
    ADDRESS_PLUS_ONE(VARIANCE),
    NUMBER(0),
    STORE(TRIPA), /* "ZI*2 + Z2*2 + Z3*2 + VARIANCE" */
    ORDERS(TLOAD, BOV),
    ADDRESS_PLUS_ONE(VARIANCE), /* "CLEAR OVFIND" */
    ADDRESS_AT(INCOR1A, 024),   /* (+1) */
    STORE(TEMPVAR),             /* "TEMP STORAGE FOR VARIANCE" */
    ORDER(BZE),
    ADDRESS(INCOR1C),
    /* INCOR1B */ ORDERS(SL2, BOV),
    ADDRESS(INCOR1C),
    STORE(TEMPVAR),
    ORDERS(INCR_2, GOTO),
    NUMBER(1),
    ADDRESS(INCOR1B),
    /* INCOR1C */ ORDERS(TLOAD, ROUND),
    ADDRESS_PLUS_ONE(TRIPA),
    ORDERS(DMP, SQRT),
    ADDRESS_PLUS_ONE(TEMPVAR),
    ORDERS(SL_STAR, TAD),
    SL_X2(0),
    ADDRESS_PLUS_ONE(TRIPA),
    ORDERS(NORM, INCR_2),
    ADDRESS_PLUS_ONE(X2),
    MINUS_NUMBER(2),
    ORDERS(SXA_2, AXT_2),
    ADDRESS(NORMGAM), /* "NORMALIZATION COUNT -2 FOR GAMMA" */
    NUMBER(162),
    ORDERS(BDDV, SETPD),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ADDRESS_PLUS_ONE_D(0),
    STORE(GAMMA),
    ORDERS(TLOAD, NORM),
    ADDRESS_PLUS_ONE(TRIPA),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DLOAD, PDDL), /* "PD 0-1 = NORM (A)" */
    ADDRESS_PLUS_ONE(MPAC),
    ADDRESS_PLUS_ONE(DELTAQ),
    ORDER(NORM),
    ADDRESS_PLUS_ONE(S1),
    ORDERS(XSU_1, SR1),
    ADDRESS(S1),
    ORDERS(DDV, PUSH), /* "PD 0-1 = DELTAQ/A" */
    ORDER(GOTO),
    ADDRESS(NEWZCOMP),

    /* INCOR2 -3 -- pp. 1151-1152 (CM pp. 1254-1255), where NEWZCOMP comes
     * back. OMEGA's three vectors, ZI's times W's blocks transposed (VXM*),
     * X2 stepping 54 words; the third zero for a 6-dimensional W. Then
     * (INCOR3) DELTAX, OMEGA times DELTAQ / A, shifted back by X1. Out
     * through EGRESS. */
    ORDER(SSP),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(54),
    /* INCOR2 */ ORDERS(VLOAD, VXM_STAR), /* "COMPUT OMEGA1,2,3" */
    ADDRESS_PLUS_ONE(ZI),
    ADDRESS_PLUS_ONE_AT_X2(W, 162),
    ORDERS(PUSH, VLOAD),
    ADDRESS_PLUS_ONE_AT(ZI, 6),
    ORDERS(VXM_STAR, VAD),
    ADDRESS_PLUS_ONE_AT_X2(W, 180),
    ORDERS(PUSH, VLOAD),
    ADDRESS_PLUS_ONE_AT(ZI, 12),
    ORDERS(VXM_STAR, VAD),
    ADDRESS_PLUS_ONE_AT_X2(W, 198),
    ORDERS(PUSH, TIX_2), /* "PD 2-7=OMEGA1, 8-13=OMEGA2, 14-19=OMEGA3" */
    ADDRESS(INCOR2),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STORE_AT(OMEGA, 12)),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STORE_AT(OMEGA, 6)),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STORE(OMEGA)),
    ORDERS(BON, VLOAD),
    SWITCH(BON, DMENFLG),
    ADDRESS(INCOR2AB),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_AT(OMEGA, 12),
    /* INCOR2AB */ ORDERS(AXT_2, SSP),
    NUMBER(18),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(6),
    /* INCOR3 */ ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X2(OMEGA, 18),
    ORDERS(VXSC, VSL_STAR),
    ADDRESS_PLUS_ONE_D(0), /* "DELTAQ/A" */
    SL_X1(0),
    STORE_AT_X2(DELTAX, 18),
    ORDERS(TIX_2, VLOAD),
    ADDRESS(INCOR3),
    ADDRESS_PLUS_ONE_AT(DELTAX, 6),
    ORDER(VSL3),
    STORE_AT(DELTAX, 6),
    ORDER(GOTO),
    ADDRESS(EGRESS),
};

static const word incorp2[] = {
    /* INCORP2 -- p. 1153 (CM p. 1256).
     * In: the "PERMANENT STATE VECTOR FOR EITHER THE LEM OR CSM",
     * "VEHUPFLG = UPDATE VEHICLE 0=LEM 1=CSM", W, DELTAX, DMENFLG, GAMMA, ZI
     * and OMEGA. Out: the "UPDATED PERMANENT STATE VECTOR".
     *
     * The return to EGRESS, INTSTALL (the integration routines become
     * this job's), and GAMMA times OMEGA's three vectors, OMEGAM1 to
     * OMEGAM3. Then to basic code (incorp2.c) for the first phase. */
    /* INCORP2 */ ORDERS(STQ, CALL),
    ADDRESS(EGRESS),
    ADDRESS(INTSTALL),
    ORDERS(VLOAD, VXSC), /* "CALC. GAMMA * OMEGA1,2,3" */
    ADDRESS_PLUS_ONE(OMEGA),
    ADDRESS_PLUS_ONE(GAMMA),
    STOVL(OMEGAM1),
    ADDRESS_PLUS_ONE_AT(OMEGA, 6),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(GAMMA),
    STOVL(OMEGAM2),
    ADDRESS_PLUS_ONE_AT(OMEGA, 12),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(GAMMA),
    STORE(OMEGAM3),
    ORDER(EXIT),
};

static const word faza1_interpretive[] = {
    /* FAZA1 +5 -- pp. 1154-1155 (CM pp. 1257-1258), the first phase, from
     * its TC INTPRET: one column of the new W into HOLDW, a vector per
     * block (two for a 6-dimensional W). For the column at X1 (WIXA) and
     * ZI's component at X2 (ZIXA): W less OMEGAMn times that component,
     * each product shifted by NORMGAM less the component's own
     * normalization ("CALC UPPER 3X9 PARTITION OF W MATRIX", the middle,
     * the lower). Then a restart point (GRP2PC) and basic code again. */
    ORDERS(LXA_1, LXA_2),
    ADDRESS(WIXA),
    ADDRESS(ZIXA),
    ORDERS(SSP, DLOAD_STAR),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ADDRESS_PLUS_ONE_AT_X2(ZI, 0),
    ORDERS(DCOMP, NORM), /* "CALC UPPER 3X9 PARTITION OF W MATRIX" */
    ADDRESS_PLUS_ONE(S2),
    ORDERS(VXSC, XCHX_2),
    ADDRESS_PLUS_ONE(OMEGAM1),
    ADDRESS(S2),
    ORDERS(LXC_2, XAD_2),
    ADDRESS(X2),
    ADDRESS(NORMGAM),
    ORDERS(VSL_STAR, XCHX_2),
    VSL_X2(0),
    ADDRESS(S2),
    ORDER(VAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(W, 54),
    STORE(HOLDW),
    ORDERS(DLOAD_STAR, DCOMP), /* "CALC MIDDLE 3X9 PARTITION OF W MATRIX" */
    ADDRESS_PLUS_ONE_AT_X2(ZI, 0),
    ORDERS(NORM, VXSC),
    ADDRESS_PLUS_ONE(S2),
    ADDRESS_PLUS_ONE(OMEGAM2),
    ORDERS(XCHX_2, LXC_2),
    ADDRESS(S2),
    ADDRESS(X2),
    ORDERS(XAD_2, VSL_STAR),
    ADDRESS(NORMGAM),
    VSL_X2(0),
    ORDERS(XCHX_2, VAD_STAR),
    ADDRESS(S2),
    ADDRESS_PLUS_ONE_AT_X1(W, 108),
    STORE_AT(HOLDW, 6),
    ORDER(BOFF),
    SWITCH(BOFF, DMENFLG), /* "BRANCH IF 6 DIMENSIONAL" */
    ADDRESS(FAZB),
    ORDERS(DLOAD_STAR, DCOMP), /* "CALC LOWER 3X9 PARTITION OF W MATRIX" */
    ADDRESS_PLUS_ONE_AT_X2(ZI, 0),
    ORDERS(NORM, VXSC),
    ADDRESS_PLUS_ONE(S2),
    ADDRESS_PLUS_ONE(OMEGAM3),
    ORDERS(XCHX_2, LXC_2),
    ADDRESS(S2),
    ADDRESS(X2),
    ORDERS(XAD_2, VSL_STAR),
    ADDRESS(NORMGAM),
    VSL_X2(0),
    ORDERS(XCHX_2, VAD_STAR),
    ADDRESS(S2),
    ADDRESS_PLUS_ONE_AT_X1(W, 162),
    STORE_AT(HOLDW, 12),
    /* FAZB */ ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(EXIT),
};

static const word fazb1_interpretive[] = {
    /* FAZB1 +7 -- pp. 1155-1156 (CM pp. 1258-1259), the second phase, from
     * its TC INTPRET: HOLDW into W's column (WIXA). If more columns are
     * left (TIX,1), FAZA again by RTB for the next; a 6-dimensional W ends
     * when ZIXB reaches -12 (FAZB5). "DONE WITH W MATRIX. UPDATE STATE
     * VECTOR": FAZC. */
    ORDERS(LXA_1, SSP),
    ADDRESS(WIXA),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(HOLDW),
    STORE_AT_X1(W, 54),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE_AT(HOLDW, 6),
    STORE_AT_X1(W, 108),
    ORDERS(BOFF, VLOAD),
    SWITCH(BOFF, DMENFLG),
    ADDRESS(FAZB5),
    ADDRESS_PLUS_ONE_AT(HOLDW, 12),
    STORE_AT_X1(W, 162),
    /* FAZB2 */ ORDERS(TIX_1, GOTO),
    ADDRESS_AT(FAZB2, 3), /* (+2) */
    ADDRESS(FAZC),        /* "DONE WITH W MATRIX. UPDATE STATE VECTOR" */
    ORDER(RTB),
    ADDRESS(FAZA),
    /* FAZB5 */ ORDERS(SLOAD, DAD),
    ADDRESS_PLUS_ONE(ZIXB),
    ADDRESS_PLUS_ONE(12DD),
    ORDERS(BHIZ, GOTO),
    ADDRESS(FAZC),
    ADDRESS(FAZB2),

    /* FAZC -- pp. 1155-1157 (CM pp. 1258-1260), the third phase: "START
     * 3RD PHASE OF INCORP2". The "7TH, 8TH, 9TH COMPONENT OF STATE VECTOR"
     * (X789, the bias) plus DELTAX's third vector to TX789. The vehicle's
     * permanent state vector into the integration's (RTB MOVEPLEM, or
     * MOVEPCSM at DOCSM). Then (FAZAB) DELTAX's position and velocity added
     * to TDELTAV and TNUV, the deviations from the conic, shifted for the
     * planet ("B27 IF MOON ORBIT, B29 IF EARTH"); if either overflows,
     * added to the conic RCV and VCV instead, and the orbit rectified
     * (RECTIFY). The integration's state vector back into the vehicle's
     * (RTB MOVEALEM, or MOVEACSM at DOCSM1), the downlink's (SVDWN2 or
     * SVDWN1), the bias from TX789 for a 9-dimensional W, and out: the
     * return from EGRESS to QPRET, then basic code (POSTJUMP to
     * INTWAKE). */
    /* FAZC */ ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(VLOAD, VAD), /* "START 3RD PHASE OF INCORP2" */
    ADDRESS_PLUS_ONE(X789),
    ADDRESS_PLUS_ONE_AT(DELTAX, 12), /* "INCORPORATION FOR X789" */
    STORE(TX789),
    ORDERS(BON, RTB),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(DOCSM),
    ADDRESS(MOVEPLEM),
    /* FAZAB */ ORDERS(BOVB, AXT_2),
    ADDRESS(TCDANZIG),
    NUMBER(0),
    ORDERS(BOFF, AXT_2),
    SWITCH(BOFF, MOONTHIS),
    ADDRESS_AT(FAZAB, 7), /* (+2) */
    NUMBER(2),
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(DELTAX), /* "B27 IF MOON ORBIT, B29 IF EARTH" */
    SR_X2(-7),
    ORDERS(VAD, BOV),
    ADDRESS_PLUS_ONE(TDELTAV),
    ADDRESS(FAZAB1),
    STOVL(TDELTAV),
    ADDRESS_PLUS_ONE_AT(DELTAX, 6), /* "B5 IF MOON ORBIT, B7 IF EARTH" */
    ORDERS(VSR_STAR, VAD),
    SR_X2(-4),
    ADDRESS_PLUS_ONE(TNUV),
    ORDER(BOV),
    ADDRESS(FAZAB2),
    STCALL(TNUV),
    ADDRESS(FAZAB3),
    /* FAZAB1 */ ORDERS(VLOAD, VAD),
    ADDRESS_PLUS_ONE(RCV),
    ADDRESS_PLUS_ONE(DELTAX),
    STORE(RCV),
    /* FAZAB2 */ ORDERS(VLOAD, VAD),
    ADDRESS_PLUS_ONE(VCV),
    ADDRESS_PLUS_ONE_AT(DELTAX, 6),
    STORE(VCV),
    ORDERS(SXA_2, CALL),
    ADDRESS(PBODY),
    ADDRESS(RECTIFY),
    /* FAZAB3 */ ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(BON, RTB),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(DOCSM1),
    ADDRESS(MOVEALEM),
    ORDER(CALL),
    ADDRESS(SVDWN2), /* "STORE DOWNLINK STATE VECTOR" */
    /* FAZAB4 */ ORDER(CALL),
    ADDRESS(GRP2PC), /* "PHASE CHANGE" */
    ORDERS(BOFF, VLOAD),
    SWITCH(BOFF, DMENFLG),
    ADDRESS(FAZAB5),         /* "6 DIMENSIONAL" */
    ADDRESS_PLUS_ONE(TX789), /* "9 DIMENSIONAL" */
    STORE(X789),
    /* FAZAB5 */ ORDERS(LXA_1, SXA_1),
    ADDRESS(EGRESS),
    ADDRESS(QPRET),
    ORDER(EXIT),
};

static const word docsm[] = {
    /* DOCSM, DOCSM1 -- p. 1157 (CM p. 1260): the CSM's state vector moves,
     * for VEHUPFLG set. */
    /* DOCSM */ ORDERS(RTB, GOTO),
    ADDRESS(MOVEPCSM),
    ADDRESS(FAZAB),
    /* DOCSM1 */ ORDERS(RTB, CALL),
    ADDRESS(MOVEACSM),
    ADDRESS(SVDWN1), /* "STORE DOWNLINK STATE VECTOR" */
    ORDER(GOTO),
    ADDRESS(FAZAB4),
    /* (ZEROD = ZEROVECS) */
    /* 54DD */ NUMBER(54),     /* DEC 54 */
    /* 6DD */ MINUS_NUMBER(6), /* DEC -6 */
    /* 12DD */ NUMBER(12),     /* DEC 12 */
};

static const word newzcomp[] = {
    /* NEWZCOMP -- pp. 1157-1158 (CM pp. 1260-1261).
     * NORMZI (36D) gets the largest of ZI's three vectors' lengths ("LARGEST
     * ABVAL"), and is normalized: the shift (less 2) in X1, kept at NORMZI
     * +1. ZI's vectors are shifted up by it, and NORMGAM less twice the
     * shift; X1 keeps the shift once more, and the push-down list at 2D.
     * Back to INCOR2 -3. (The CM's goes on in another bank, at NEWZCMP1.) */
    /* NEWZCOMP */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(ZI),
    STOVL(NORMZI),
    ADDRESS_PLUS_ONE_AT(ZI, 6),
    ORDERS(ABVAL, PUSH),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(NORMZI),
    ADDRESS_AT(NEWZCOMP, 012), /* (+3) */
    ORDERS(DLOAD, STADR),
    AFTER_STADR(STORE(NORMZI)),
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE_AT(ZI, 12),
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(NORMZI),
    ORDERS(BMN, DLOAD),
#if LUMINARY099
    ADDRESS_AT(NEWZCOMP, 022), /* (+3) */
    ORDER(STADR),
    AFTER_STADR(STORE(NORMZI)), /* "LARGEST ABVAL" */
#else
    ADDRESS(NEWZCMP1),
    ORDER(STADR),
    AFTER_STADR(STCALL(NORMZI)),
    ADDRESS(NEWZCMP1),
};

static const word newzcmp1[] = {
/* NEWZCMP1 -- CM p. 1261, in its own bank. */
#endif
    /* (the CM's NEWZCMP1) */ ORDERS(DLOAD, SXA_1),
    ADDRESS_PLUS_ONE(NORMZI),
    ADDRESS(NORMZI), /* "SAVE X1" */
    ORDERS(NORM, INCR_1),
    ADDRESS_PLUS_ONE(X1),
    NUMBER(2),
    ORDERS(VLOAD, VSL_STAR),
    ADDRESS_PLUS_ONE(ZI),
    SL_X1(0),
    STOVL(ZI),
    ADDRESS_PLUS_ONE_AT(ZI, 6),
    ORDER(VSL_STAR),
    SL_X1(0),
    STOVL_AT(ZI, 6),
    ADDRESS_PLUS_ONE_AT(ZI, 12),
    ORDERS(VSL_STAR, SXA_1),
    SL_X1(0),
    ADDRESS_AT(NORMZI, 1), /* "SAVE SHIFT" */
    STORE_AT(ZI, 12),
    ORDERS(LXA_1, XSU_1),
    ADDRESS(NORMGAM),
    ADDRESS_AT(NORMZI, 1),
    ORDER(XSU_1),
    ADDRESS_AT(NORMZI, 1),
    ORDERS(SXA_1, LXC_1),
    ADDRESS(NORMGAM),
    ADDRESS_AT(NORMZI, 1),
    ORDERS(XAD_1, SETPD),
    ADDRESS(NORMZI),
    ADDRESS_PLUS_ONE_D(2),
    ORDER(GOTO),
    ADDRESS_AT(INCOR2, -3),
};

/* The bank of a fixed word's name (bank * 02000 plus its address in the
 * bank, as names.h has it). */
#define BANK_OF(fixed) ((unsigned)((fixed) >> 10))

const struct interpretive_code MEASUREMENT_INC_CODE[MEASUREMENT_INC_PARTS] = {
    {BANK_OF(I_INCORP1), K_INCORP1, COUNT(incorp1), incorp1},
    {BANK_OF(I_INCORP2), K_INCORP2, COUNT(incorp2), incorp2},
    {BANK_OF(I_FAZA1), (word)(K_FAZA1 + 5u), COUNT(faza1_interpretive), faza1_interpretive},
    {BANK_OF(I_FAZB1), (word)(K_FAZB1 + 7u), COUNT(fazb1_interpretive), fazb1_interpretive},
    {BANK_OF(I_DOCSM), K_DOCSM, COUNT(docsm), docsm},
    {BANK_OF(I_NEWZCOMP), K_NEWZCOMP, COUNT(newzcomp), newzcomp},
#if COMANCHE055
    {BANK_OF(I_NEWZCMP1), K_NEWZCMP1, COUNT(newzcmp1), newzcmp1},
#endif
};
