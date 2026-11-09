/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, MIDTOAV1 and MIDTOAV2 to TIMEDELT (LM
 * pp. 1223-1225, CM pp. 1329-1331). The interpretive words are in the
 * listing's notation (src/interpreter/notation.h); the basic code between
 * them is C.
 *
 * MIDTOAV1 and MIDTOAV2 bring this vehicle's state vector to the time a
 * burn's AVERAGEG starts from. MIDTOAV1 "INTEGRATES (PRECISION) TO THE
 * TIME SPECIFIED IN TDEC1", MIDTOAV2 "TO THE CURRENT TIME" (plus
 * TIMEDELT, about one time step's computing time). The integration's
 * loop (TESTLOOP) comes back at CKMID2 after each time step: for MIDTOAV1
 * (MID1FLAG), when TDEC1 is no longer ahead of the clock by TIMEDELT the
 * integration goes on to the present instead (NOTIME, alarm 1703, the
 * caller's return one on); for MIDTOAV2 the target moves with the clock
 * (TIMEINC) until the integration is within 3CSECS of it (MID2). The
 * result goes to RN1, VN1 and PIPTIME1, MPAC to TAT minus the time
 * ("TAT MINUS CURRENT TIME"), and the caller gets it back in basic code
 * (BANKJUMP). In the CM, T-TO-ADD is what is added to the clock, and MID2
 * also wants 5.6SECS left to the caller: otherwise TIMEDELT more. */
#include "integration_init.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

/* ---- MIDTOAV2 to ENTMID1's EXIT ---- */

static const word midtoav_words[] = {
#if LUMINARY099
    /* MIDTOAV2 -- LM p. 1223: the return kept in IRETURN1, MID1FLAG
     * cleared, and ENTMID2 ("INTEGRATE TO PRESENT TIME PLUS TIMEDELT"). */
    /* MIDTOAV2 */ ORDERS(STQ, CLRGO),
    ADDRESS(IRETURN1),
    SWITCH(CLRGO, MID1FLAG),
    ADDRESS(ENTMID2),
    /* MIDTOAV1: the return kept, MID1FLAG set; "IS TDEC1 IN THE FUTURE"
     * by TIMEDELT or more: ENTMID1; otherwise NOTIME ("SET ALARM, SWITCH
     * TO MIDTOAV2"), which comes back to ENTMID2. */
    /* MIDTOAV1 */ ORDERS(STQ, SET), /* "INTEGRATE TO TDEC1" */
    ADDRESS(IRETURN1),
    SWITCH(SET, MID1FLAG),
    ORDERS(RTB, DAD), /* "INITIAL CHECK, IS TDEC1 IN THE FUTURE" */
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TIMEDELT),
    ORDERS(BDSU, BPL),
    ADDRESS_PLUS_ONE(TDEC1),
    ADDRESS(ENTMID1),
    ORDER(CALL),
    ADDRESS(NOTIME),
    /* ENTMID2: TDEC1 the time now plus TIMEDELT. */
    /* ENTMID2 */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TIMEDELT),
    STORE(TDEC1),
    /* ENTMID1 -- LM p. 1223: the integration taken (INTSTALL); this
     * vehicle's state vector (THISVINT) by precision integration, without
     * the W-matrix, with MIDAVFLG set ("LET INTEG. KNOW THE CALL IS FOR
     * MIDTOAV."). */
    /* ENTMID1 */ ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, DIM0FLAG), /* "NO W-MATRIX" */
    ADDRESS(THISVINT),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, INTYPFLG),
    SWITCH(SET, MIDAVFLG),
    ORDER(CALL),
    ADDRESS(INTEGRV), /* "GO INTEGRATE" */
    /* The result into RN1, VN1 and PIPTIME1; the central body's index
     * (X2) and X1 kept in RTX2 and RTX1. */
    ORDERS(CLEAR, VLOAD),
    SWITCH(CLEAR, MIDAVFLG),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(RN1),
    ADDRESS_PLUS_ONE(VATT),
    STODL(VN1),
    ADDRESS_PLUS_ONE(TAT),
    STORE(PIPTIME1),
    ORDERS(SXA_2, SXA_1),
    ADDRESS(RTX2),
    ADDRESS(RTX1),
    ORDER(EXIT),
#else
    /* MIDTOAV2 -- CM p. 1329: the return kept in IRETURN1, the
     * integration taken (INTSTALL), TIMEDELT as T-TO-ADD, MID1FLAG
     * cleared, and ENTMID2. */
    /* MIDTOAV2 */ ORDERS(STQ, CALL),
    ADDRESS(IRETURN1),
    ADDRESS(INTSTALL),
    ORDERS(DLOAD, CLEAR),
    ADDRESS_PLUS_ONE(TIMEDELT),
    SWITCH(CLEAR, MID1FLAG),
    STCALL(T_TO_ADD),
    ADDRESS(ENTMID2),
    /* MIDTOAV1: the return kept, the integration taken, MID1FLAG set;
     * "IS TDEC1 IN THE FUTURE" by TIMEDELT or more: ENTMID1; otherwise
     * NOTIME ("SET ALARM.SWITCH TO MIDTOAV2"), which comes back to
     * ENTMID2. */
    /* MIDTOAV1 */ ORDERS(STQ, CALL),
    ADDRESS(IRETURN1),
    ADDRESS(INTSTALL),
    ORDERS(SET, RTB),
    SWITCH(SET, MID1FLAG),
    ADDRESS(LOADTIME),
    ORDERS(DAD, BDSU), /* "INITIAL CHECK.IS TDEC1 IN THE FUTURE." */
    ADDRESS_PLUS_ONE(TIMEDELT),
    ADDRESS_PLUS_ONE(TDEC1),
    ORDERS(BPL, CALL),
    ADDRESS(ENTMID1),
    ADDRESS(NOTIME),
    /* ENTMID2: TDEC1 the time now plus T-TO-ADD. */
    /* ENTMID2 */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(T_TO_ADD),
    STORE(TDEC1),
    /* ENTMID1 -- CM p. 1330: this vehicle's state vector (THISVINT) by
     * precision integration, without the W-matrix, with MIDAVFLG set
     * ("LET INTEG. KNOW THE CALL IS FOR MIDTOAV."). */
    /* ENTMID1 */ ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, DIM0FLAG), /* "NO W-MATRIX" */
    ADDRESS(THISVINT),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, INTYPFLG),
    SWITCH(SET, MIDAVFLG),
    ORDER(CALL),
    ADDRESS(INTEGRV), /* "GO INTEGRATE" */
    /* The central body's index (X2) and X1 kept in RTX2 and RTX1;
     * AMOONFLG set for the moon (RTX2 not zero); the result into RN1,
     * VN1 and PIPTIME1. */
    ORDERS(SXA_2, SXA_1),
    ADDRESS(RTX2),
    ADDRESS(RTX1),
    ORDERS(CLEAR, SLOAD),
    SWITCH(CLEAR, AMOONFLG),
    ADDRESS_PLUS_ONE(RTX2),
    ORDERS(BZE, SET),
    ADDRESS_AT(ENTMID1, 021), /* (+2) */
    SWITCH(SET, AMOONFLG),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RATT),
    STOVL(RN1),
    ADDRESS_PLUS_ONE(VATT),
    STODL(VN1),
    ADDRESS_PLUS_ONE(TAT),
    STORE(PIPTIME1),
    ORDER(EXIT),
#endif
};

const struct interpretive_code MIDTOAV_CODE = {CODE_BANK(I_MIDTOAV2), CODE_Z(I_MIDTOAV2),
                                               COUNT(midtoav_words), midtoav_words};

/* TC at "at" to a routine outside the file (TPAGREE, BANKJUMP, ALARM),
 * which reads the words after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* After ENTMID1's EXIT -- LM p. 1224 (CM p. 1330): MPAC (TAT) less the
 * time (TIME2, TIME1), brought to one sign (TPAGREE); then back to the
 * caller (BANKJUMP to IRETURN1). The words are just before CKMID2's. */
void entmid1_after_exit(void)
{
    inhint();
    dcs(ADDRESS_OF(TIME2)); /* EXTEND; DCS TIME2 */
    das(ADDRESS_OF(MPAC));
    tc_to((word)(CKMID2 - 3u), TPAGREE);
}

void entmid1_after_tpagree(void)
{
    ca(ADDRESS_OF(IRETURN1));
    tc_to((word)(CKMID2 - 1u), BANKJUMP);
}

/* ---- CKMID2 to NOTIME's EXIT ---- */

static const word ckmid2_words[] = {
    /* CKMID2 -- LM p. 1224 (CM p. 1330), after each time step: for
     * MIDTOAV2 (MID1FLAG clear), MID2. For MIDTOAV1, the time now plus
     * TIMEDELT still before TDEC: the next step (TESTLOOP, "YES");
     * otherwise NOTIME. */
    /* CKMID2 */ ORDERS(BOF, RTB),
    SWITCH(BOF, MID1FLAG),
    ADDRESS(MID2),
    ADDRESS(LOADTIME),
    ORDERS(DAD, BDSU),
    ADDRESS_PLUS_ONE(TIMEDELT),
    ADDRESS_PLUS_ONE(TDEC),
    ORDERS(BPL, CALL),
    ADDRESS(TESTLOOP), /* "YES" */
    ADDRESS(NOTIME),
    /* TIMEINC: TDEC the time now plus TIMEDELT (the CM's T-TO-ADD), and
     * the next step. */
    /* TIMEINC */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
#if LUMINARY099
    ADDRESS_PLUS_ONE(TIMEDELT),
#else
    ADDRESS_PLUS_ONE(T_TO_ADD),
#endif
    STCALL(TDEC),
    ADDRESS(TESTLOOP),
    /* MID2: the integration within 3CSECS of TDEC is done (A-PCHK);
     * otherwise TIMEINC. In the CM, done only with 5.6SECS left before
     * TET ("SEE IF 5.6 SECS. AVAILABLE TO CALLER"); otherwise TIMEDELT
     * more in T-TO-ADD ("TRY AGAIN"). */
    /* MID2 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TDEC),
    ADDRESS_PLUS_ONE(TET),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(3CSECS),
#if LUMINARY099
    ORDERS(BMN, GOTO),
    ADDRESS(A_PCHK),
    ADDRESS(TIMEINC),
#else
    ORDER(BPL),
    ADDRESS(TIMEINC),
    ORDERS(RTB, BDSU), /* "SEE IF 5.6 SECS. AVAILABLE TO CALLER" */
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(TET),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(5_6SECS),
    ADDRESS(A_PCHK), /* "YES,GET OUT." */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(T_TO_ADD),
    ADDRESS_PLUS_ONE(TIMEDELT),
    STCALL(T_TO_ADD),
    ADDRESS(TIMEINC),
#endif
    /* NOTIME -- LM p. 1225 (CM p. 1331): "TOO LATE". MID1FLAG cleared,
     * and out of the interpreter. */
    /* NOTIME */ ORDERS(CLEAR, EXIT), /* "TOO LATE" */
    SWITCH(CLEAR, MID1FLAG),
};

const struct interpretive_code CKMID2_CODE = {CODE_BANK(I_CKMID2), CODE_Z(I_CKMID2),
                                              COUNT(ckmid2_words), ckmid2_words};

/* After NOTIME's EXIT: the caller's return one on ("SET ERROR EXIT"),
 * alarm 1703 ("INSUFFICIENT TIME FOR INTEGRATION"), and back into the
 * interpreter. */
void notime_after_exit(void)
{
    incr(ADDRESS_OF(IRETURN1));        /* "SET ERROR EXIT" */
    tc_to((word)(NOTIME + 3u), ALARM); /* OCT 1703 */
}

void notime_after_alarm(void)
{
    tc_at((word)(NOTIME + 5u)); /* TC INTPRET */
    intpret();
}

/* ---- After NOTIME's TC INTPRET, to TIMEDELT ---- */

static const word notime_words[] = {
#if COMANCHE055
    /* T-TO-ADD back to TIMEDELT. */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TIMEDELT),
    STORE(T_TO_ADD),
#endif
    ORDER(RVQ),
    /* 3CSECS and TIMEDELT (the CM's 5.6SECS too). */
    /* 3CSECS */ NUMBER(000000), /* 2DEC 3 */
    NUMBER(000003),
#if LUMINARY099
    /* TIMEDELT */ NUMBER(000000), /* 2DEC 2000 */
    NUMBER(003720),
#else
    /* TIMEDELT */ NUMBER(000000), /* 2DEC 1250 */
    NUMBER(002342),
    /* 5.6SECS */ NUMBER(000000), /* 2DEC 560 */
    NUMBER(001060),
#endif
};

/* The words after NOTIME's TC INTPRET. */
const struct interpretive_code NOTIME_RVQ_CODE = {
    CODE_BANK(I_NOTIME), (word)(CODE_Z(I_NOTIME) + 6u), COUNT(notime_words), notime_words};
