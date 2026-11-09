/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, DIFEQ+0 to WMATEND (LM pp. 1241-1244, CM pp.
 * 1348-1351). The interpretive words are in the listing's notation
 * (src/interpreter/notation.h); the basic code between them is C.
 *
 * "THE THREE DIFEQ ROUTINES -- DIFEQ+0, DIFEQ+12, DIFEQ+24 -- ARE ENTERED
 * TO PROCESS THE CONTRIBUTIONS AT THE" beginning, middle and end of the
 * time step, by the Nystrom method: each takes the acceleration FV (from
 * ACCOMP or DOW..) into PHIV and PSIV, and at the end DIFEQ+2 moves the
 * deviation from the conic on by the step, the position YV and the
 * velocity ZV. DIFEQCOM moves H on by half a step and DIFEQCNT to the
 * next stage, sets ALPHAV, the position at H, and goes on to the next
 * stage's acceleration (FBR3, or DOW.. for the W-matrix).
 *
 * The W-matrix (JSWITCH set) is integrated a column at a time: COLREG
 * counts the columns, each kept in W (the position) and W +54D (the
 * velocity), from the vehicle's positions the state vector's steps left
 * in VECTAB. A column too large (BOV) gives up on the W-matrix: WMATEND
 * clears DIM0FLAG, ORBWFLAG and RENDWFLG, sets STATEFLG, alarm 421, and
 * finishes the state vector alone. When the last column is done, RELOADSV starts
 * the state vector again (INTEGRV2), the W-matrix having moved.
 *
 * ENDSTATE ends a step of the state vector: TNUV and TDELTAV the new
 * deviations, then on to CKMID2 or TESTLOOP; or, with DIM0FLAG set (the
 * W-matrix to be integrated too), out of the interpreter for group 2's
 * phase change and REINTFLG, and the state vector into the permanent one
 * (ATOPCSM or ATOPLEM, by VINTFLAG) before the W-matrix: AMOVED, with
 * COLREG -30 (six columns) or -48 (nine, D6OR9FLG). */
#include "orbital_integration.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

static const word difeq_words[] = {
    /* DIFEQ+0 -- LM p. 1241 (CM p. 1348): PHIV FV/8. */
    /* DIFEQ+0 */ ORDERS(VLOAD, VSR3),
    ADDRESS_PLUS_ONE(FV),
    STCALL(PHIV),
    ADDRESS(DIFEQCOM),
    /* DIFEQ+1: PSIV FV/2 plus PHIV; PHIV FV/4 plus PHIV. */
    /* DIFEQ+1 */ ORDERS(VLOAD, VSR1),
    ADDRESS_PLUS_ONE(FV),
    ORDERS(PUSH, VAD),
    ADDRESS_PLUS_ONE(PHIV),
    STOVL(PSIV),
    ORDERS(VSR1, VAD),
    ADDRESS_PLUS_ONE(PHIV),
    STCALL(PHIV),
    ADDRESS(DIFEQCOM),
    /* DIFEQ+2: the step's end. YV moved on by the step's velocity and
     * PHIV, ZV by PSIV and FV/8. */
    /* DIFEQ+2 */ ORDERS(DLOAD, DMPR),
    ADDRESS_PLUS_ONE(H),
    ADDRESS_PLUS_ONE(DP2_3),
    ORDERS(PUSH, VXSC),
    ADDRESS_PLUS_ONE(PHIV),
    ORDERS(VSL1, VAD),
    ADDRESS_PLUS_ONE(ZV),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(H),
    ADDRESS_PLUS_ONE(YV),
    STOVL(YV),
    ADDRESS_PLUS_ONE(FV),
    ORDERS(VSR3, VAD),
    ADDRESS_PLUS_ONE(PSIV),
    ORDERS(VXSC, VSL1),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(ZV),
    STORE(ZV),
    /* The state vector (JSWITCH clear): ENDSTATE. The W-matrix: the
     * column into W +54D and W, X2 COLREG; too large, WMATEND. */
    ORDERS(BOFF, CALL),
    SWITCH(BOFF, JSWITCH),
    ADDRESS(ENDSTATE),
    ADDRESS(GRP2PC),
    ORDERS(LXA_2, VLOAD),
    ADDRESS(COLREG),
    ADDRESS_PLUS_ONE(ZV),
    ORDER(VSL3), /* "ADJUST W-POSITION FOR STORAGE" */
    STORE_AT_X2(W, 54),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(YV),
    ORDERS(VSL3, BOV),
    ADDRESS(WMATEND),
    STORE_AT_X2(W, 0),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    /* LM p. 1242 (CM p. 1349): S2 zero, COLREG on by 6 (TIX: on to the
     * next column; after the last, RELOADSV). */
    ORDERS(LXA_2, SSP),
    ADDRESS(COLREG),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(0),
    ORDERS(INCR_2, SXA_2),
    NUMBER(6),
    ADDRESS(YV),
    ORDERS(TIX_2, CALL),
    ADDRESS(RELOADSV),
    ADDRESS(GRP2PC),
    ORDERS(LXA_2, SXA_2),
    ADDRESS(YV),
    ADDRESS(COLREG),
    /* NEXTCOL: the column's position and velocity into YV and ZV, X1
     * zero, and its first stage (DIFEQ0). */
    /* NEXTCOL */ ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(LXA_2, VLOAD_STAR),
    ADDRESS(COLREG),
    ADDRESS_PLUS_ONE_AT_X2(W, 0),
    ORDER(VSR3), /* "ADJUST W-POSITION FOR INTEGRATION" */
    STORE(YV),
    ORDERS(VLOAD_STAR, AXT_1),
    ADDRESS_PLUS_ONE_AT_X2(W, 54),
    NUMBER(0),
    ORDER(VSR3), /* "ADJUST W-VELOCITY FOR INTEGRATION" */
    STCALL(ZV),
    ADDRESS(DIFEQ0),
    /* ENDSTATE: on an overflow, GOBAQUE; the deviations into TNUV and
     * TDELTAV; MIDAVFLG set, CKMID2; DIM0FLAG clear, TESTLOOP. */
    /* ENDSTATE */ ORDERS(BOV, VLOAD),
    ADDRESS(GOBAQUE),
    ADDRESS_PLUS_ONE(ZV),
    STOVL(TNUV),
    ADDRESS_PLUS_ONE(YV),
    STORE(TDELTAV),
    ORDERS(BON, BOFF),
    SWITCH(BON, MIDAVFLG),
    ADDRESS(CKMID2), /* "CHECK FOR MID2 BEFORE GOING TO TIMEINC" */
    SWITCH(BOFF, DIM0FLAG),
    ADDRESS(TESTLOOP),
    ORDER(EXIT),
};

const struct interpretive_code DIFEQ_CODE = {CODE_BANK(I_DIFEQP0), CODE_Z(I_DIFEQP0),
                                             COUNT(difeq_words), difeq_words};

/* TC at "at" to a routine outside the file (PHASCHNG, UPFLAG, ALARM),
 * which reads the word after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* After ENDSTATE's EXIT: group 2's phase (OCT 04022, "PHASE 1"), REINTFLG
 * set, and back into the interpreter. */
void endstate_after_exit(void)
{
    tc_to((word)(ENDSTATE + 014u), PHASCHNG); /* OCT 04022 */
}

void endstate_after_phaschng(void)
{
    tc_to((word)(ENDSTATE + 016u), UPFLAG); /* ADRES REINTFLG */
}

void endstate_after_upflag(void)
{
    tc_at((word)(ENDSTATE + 020u)); /* TC INTPRET */
    intpret();
}

static const word amoved_words[] = {
    /* The return into QPRET (AMOVED); the state vector into the CSM's
     * permanent one (VINTFLAG set) or the LM's. */
    ORDER(SSP),
    ADDRESS_PLUS_ONE(QPRET),
    ADDRESS(AMOVED),
    ORDERS(BON, GOTO),
    SWITCH(BON, VINTFLAG),
    /* LM p. 1243 (CM p. 1350) */
    ADDRESS(ATOPCSM),
    ADDRESS(ATOPLEM),
    /* AMOVED: JSWITCH set; COLREG -30, or -48 with D6OR9FLG set; then
     * NEXTCOL. */
    /* AMOVED */ ORDERS(SET, SSP),
    SWITCH(SET, JSWITCH),
    ADDRESS_PLUS_ONE(COLREG),
    MINUS_NUMBER(30),
    ORDERS(BOFF, SSP),
    SWITCH(BOFF, D6OR9FLG),
    ADDRESS(NEXTCOL),
    ADDRESS_PLUS_ONE(COLREG),
    MINUS_NUMBER(48),
    ORDER(GOTO),
    ADDRESS(NEXTCOL),
    /* RELOADSV: "RELOAD TEMPORARY STATE VECTOR", TDEC into TDEC1, and the
     * state vector again from INTEGRV2. */
    /* RELOADSV */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TDEC),
    STCALL(TDEC1),
    ADDRESS(INTEGRV2),
    /* DIFEQCOM: "INCREMENT H AND DIFEQCNT."; ALPHAV the position at H
     * (YV, ZV and FV); the next stage's acceleration: DOW.. for the
     * W-matrix, otherwise FBR3. */
    /* DIFEQCOM */ ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(DT_2),
    ADDRESS_PLUS_ONE(H),
    ORDERS(INCR_1, SXA_1),
    MINUS_NUMBER(12),
    ADDRESS(DIFEQCNT), /* "DIFEQCNT SET FOR NEXT ENTRY." */
    STORE(H),
    ORDERS(VXSC, VSR1),
    ADDRESS_PLUS_ONE(FV),
    ORDERS(VAD, VXSC),
    ADDRESS_PLUS_ONE(ZV),
    ADDRESS_PLUS_ONE(H),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(YV),
    STORE(ALPHAV),
    ORDERS(BON, GOTO),
    SWITCH(BON, JSWITCH),
    ADDRESS(DOW__),
    ADDRESS(FBR3),
    /* WMATEND: the W-matrix given up (DIM0FLAG, ORBWFLAG and RENDWFLG
     * cleared), STATEFLG set, and out of the interpreter. */
    /* WMATEND */ ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(CLEAR, ORBWFLAG),
    ORDER(CLEAR),
    SWITCH(CLEAR, RENDWFLG),
    ORDERS(SET, EXIT),
    SWITCH(SET, STATEFLG), /* "PICK UP STATE VECTOR UPDATE" */
};

/* The words after ENDSTATE's TC INTPRET. */
const struct interpretive_code AMOVED_CODE = {
    CODE_BANK(I_ENDSTATE), (word)(CODE_Z(I_ENDSTATE) + 021u), COUNT(amoved_words), amoved_words};

/* After WMATEND's EXIT: alarm 421, and back into the interpreter. */
void wmatend_after_exit(void)
{
    tc_to((word)(WMATEND + 7u), ALARM); /* OCT 421 */
}

void wmatend_after_alarm(void)
{
    tc_at((word)(WMATEND + 011u)); /* TC INTPRET */
    intpret();
}
