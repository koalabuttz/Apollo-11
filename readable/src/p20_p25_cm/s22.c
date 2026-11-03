/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 9 (Comanche055 pp. 598-600 and 609-610): the start
 * of S22.1, the "ORBITAL NAVIGATION ROUTINE" P22 runs on its marks (bank
 * 30), and its small subroutines in bank 31.
 *
 * S22.1's three jobs: "UPDATE CSM STATE VECTOR", "UPDATE LANDMARK
 * POSITION" and "CONVERT W MATRIX FROM 9 TO 6 DIMENSIONS". This part
 * copies the marks to SVMRKDAT, sets up the W matrix (cleared, with its
 * diagonals from WORBPOS and WORBVEL, when it is not valid), and starts on
 * each mark: the CSM's state vector integrated to the mark's time, the
 * line of sight UM from its angles (GETUM), and the test of whether the
 * mark is the landmark's offset one (OFF = I). The rest of S22.1 is the
 * next parts.
 *
 * Each call out of the file (BANKCALL, 2PHSCHNG) leaves Z at the routine
 * and Q at the word after the TC, where the routine comes back to. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* S22.1 -- pp. 598-599: the return in S22RTNEX; "SET I=1" (8KK, "ITEM
 * 8KK IS I"), and the "MARK DATA ADDRESS" in S22LOC. Then the marks, six
 * words each (S1), from the address in MARKSTAT (X2 its complement) to
 * the SVMRKDAT table ("5 SETS FROM ADDR. IN MARKSTAT TO SVMRKDAT TABLE TO
 * AVOID LOSS IF RESTART OCCURS"), X1 counting 36 down by six; P22MKFLG
 * set ("DOWNLINKED SVMRKDAT HOLDS PRESENT MARKS"), and out of the
 * interpreter. */
static const word s22_1_words[] = {
    ORDERS(STQ, SSP),
    ADDRESS(S22RTNEX),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    /* "SET I=1" */
    ORDERS(SSP, SSP),
    ADDRESS_PLUS_ONE(8KK),
    NUMBER(1),
    ADDRESS_PLUS_ONE(S22LOC),
    /* ECADR SVMRKDAT: "SET MARK DATA ADDRESS INTO S22LOC" */
    ADDRESS(SVMRKDAT),
    ORDERS(LXC_2, AXT_1),
    ADDRESS(MARKSTAT),
    NUMBER(36),
    /* S22.111: "MOVE MARK DATA" */
    ORDER(VLOAD_STAR),
    ADDRESS_PLUS_ONE_X2(0),
    STORE_AT_X1(SVMRKDAT, 36),
    ORDERS(INCR_2, TIX_1),
    MINUS_NUMBER(6),
    ADDRESS(S22_111),
    ORDERS(SET, EXIT),
    /* "DOWNLINKED SVMRKDAT HOLDS PRESENT MARKS" */
    SWITCH(SET, P22MKFLG),
};

/* S22.1 +024 -- after the EXIT: "RELEASE VAC AREA WHERE MARK DATA WAS"
 * (MKRELEAS), */
void s22_1_after_exit(void)
{
    tc_to(S22_1 + 024u, BANKCALL); /* CADR MKRELEAS */
}

/* S22.1 +026: 2PHSCHNG. OCT 4 makes restart group 4 inactive, and OCT
 * 05022 restarts group 2 at S22.1 +032, the word after the phase words,
 * as a job of priority 13 (OCT 13000). */
void s22_1_after_mkreleas(void)
{
    tc_to(S22_1 + 026u, TWO_PHSCHNG);
}

/* S22.1 +032: in interpretive code, */
void s22_1_after_2phschng(void)
{
    tc_at(S22_1 + 032u);
    intpret();
}

/* S22.1 +033: S22EORM, the shift for the CSM's position (S22CALRC): 0
 * for the earth, -2 for the moon (CMOONFLG); the push-down pointer 0.
 * FIG2EXIT: the integration set up (INTSTALL, and S22FLGS's flags).
 *
 * Then the W matrix, by the flowchart's D: a valid W matrix (ORBWFLAG)
 * of nine dimensions (D6OR9FLG set) is kept, and one of six (D6OR9FLG
 * clear: the flag cleared on the way) too; SETWW5D, with none valid
 * (DIM0FLAG clear), clears W0-W5 (the words W to W +107D, a vector at a
 * time) and sets the diagonals of W0, from WORBPOS, and of W4, from
 * WORBVEL. SETVANDI: DMENFLG clear ("0=6X6W, 1=9X9W").
 *
 * S22NXTIN, each mark: the CSM's state vector integrated (INTEGRV) to the
 * mark's time (GETTF), its position in CSMPOS (S22CALRC), and the mark's
 * address, S22LOC, in MARKDATA for GETUM, which gives the line of sight
 * UM. DMPINTEG: "TEST OFF=I", the mark's number (8KK) less the offset
 * mark's (CXOFF, "SCALED B-5, MUST MOVE TO B-14 BEFORE SUBT."): equal, on
 * at S22OFF=I. Else, with DMENFLG set, at S22D=9; on the first pass,
 * GRP2PC's group 2 phase change, ORBWFLAG and DMENFLG set, 22DSPFLG ("=1
 * TO DISPLAY DR,DV ON FIRST PASS") and ERADFLAG ("=1 TO COMPUTE FISCHER
 * RADIUS") set; a known landmark (KNOWNFLG) on at S22BOX22. */
static const word s22shift_words[] = {
    ORDERS(AXT_1, BOFF),
    NUMBER(0),
    /* "=0 EARTH, =1 MOON" */
    SWITCH(BOF, CMOONFLG),
    ADDRESS(S22SHIFT),
    ORDER(INCR_1),
    MINUS_NUMBER(2),
    /* S22SHIFT */
    ORDERS(SXA_1, SETPD),
    /* "SET =0 EARTH, =-2 MOON FOR SHIFTING" */
    ADDRESS(S22EORM),
    ADDRESS_PLUS_ONE_D(0),
    /* FIG2EXIT */
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDER(CALL),
    ADDRESS(S22FLGS),
    ORDERS(BOFF, CLRGO),
    SWITCH(BOF, ORBWFLAG),
    /* "BRANCH TO SET W0-W5,ORBWFLAG,D" */
    ADDRESS(SETWW5D),
    /* "FLOWCHART D=6 PATH" */
    SWITCH(CLRGO, D6OR9FLG),
    ADDRESS(SETVANDI),
    /* SETWW5D */
    ORDER(CLEAR),
    /* "FLOWCHART D=0 PATH" */
    SWITCH(CLEAR, DIM0FLAG),
    ORDERS(AXT_1, SSP),
    NUMBER(108),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDERS(CLEAR, VLOAD),
    /* "GSOP CHANGE 8/18/67" */
    SWITCH(CLEAR, RENDWFLG),
    ADDRESS_PLUS_ONE(ZEROVECS),
    /* CLEARWW5 */
    STORE_AT_X1(W, 108),
    ORDERS(TIX_1, SLOAD),
    ADDRESS(CLEARWW5),
    ADDRESS_PLUS_ONE(WORBPOS),
    /* "SET DIAGONALS OF W0" */
    STORE(W),
    STORE_AT(W, 8),
    STORE_AT(W, 16),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(WORBVEL),
    /* "SET DIAGONALS OF W4" */
    STORE_AT(W, 72),
    STORE_AT(W, 80),
    STORE_AT(W, 88),
    /* SETVANDI */
    ORDER(CLEAR),
    /* "0=6X6W, 1=9X9W" */
    SWITCH(CLEAR, DMENFLG),
    /* S22NXTIN */
    ORDER(CALL),
    ADDRESS(GETTF),
    STCALL(TDEC1),
    ADDRESS(INTEGRV),
    ORDER(CALL),
    /* "CALC. RC B-29 OR B-27 (CSMPOS)" */
    ADDRESS(S22CALRC),
    ORDERS(LXA_1, SXA_1),
    /* "SETUP ADDR. OF MARK DATA FOR GETUM SUBR." */
    ADDRESS(S22LOC),
    ADDRESS(MARKDATA),
    /* "COMPUTE UM" */
    ORDER(CALL),
    /* S2GETUM */
    ADDRESS(GETUM),
    STORE(UM),
    /* DMPINTEG: "TEST OFF=I" */
    ORDERS(SLOAD, PUSH),
    ADDRESS_PLUS_ONE(8KK),
    /* "CXOFF SCALED B-5, MUST MOVE TO B-14" */
    ORDERS(SLOAD, SR3),
    /* "BEFORE SUBT." */
    ADDRESS_PLUS_ONE(CXOFF),
    ORDERS(SR3, SR3),
    ORDER(DSU),
    ORDERS(BHIZ, BON),
    /* "BRANCH HERE IF OFF=I" */
    ADDRESS(S22OFF_EQ_I),
    /* "0=6X6W, 1=9X9W" */
    SWITCH(BON, DMENFLG),
    ADDRESS(S22D_EQ_9),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(SET),
    SWITCH(SET, ORBWFLAG),
    ORDERS(SET, SET),
    /* "=0 ON FIRST PASS THRU HERE FOR D=0, OR 6" */
    SWITCH(SET, DMENFLG),
    /* "=1 TO DISPLAY DR,DV ON FIRST PASS" */
    SWITCH(SET, 22DSPFLG),
    ORDERS(SET, BON),
    /* "=1 TO COMPUTE FISCHER RADIUS" */
    SWITCH(SET, ERADFLAG),
    SWITCH(BON, KNOWNFLG),
    ADDRESS(S22BOX22),
};

/* S2231X13 -- pp. 609-610, bank 31: "MULT 3X1 BY 1X3, STORE RESULTING
 * 3X3 IN S22UMRL - S22UMRL+17D": the 3X1 vector in MPAC (kept in
 * S221X3) times each component of S223X1 in turn (X2 6, 4 and 2, down by
 * S2), a row of S22UMRL each (X1 18, 12 and 6).
 *
 * GETTF: "SET MPAC= TF", the mark's time, from the address in S22LOC.
 *
 * S22FLGS: the "INTEGRATION FLAGS": DIM0FLAG and D6OR9FLG set (the W
 * matrix integrated, of nine dimensions), the CSM's state vector
 * (VINTFLAG) as the one to update (STATEFLG), by precision integration
 * (INTYPFLG clear).
 *
 * LLASRD: the landmark's altitude (ALT, "METERS B-29") to LANDALT, and its
 * longitude, half, to LANDLONG; LLASRDA the other way, from LANDALT and
 * LANDLONG (twice) to ALT and LONG, "AFTER LMK DATA LOADED BY
 * ASTRONAUT". */
static const word s2231x13_words[] = {
    STORE(S221X3),
    /* "S22UMRL - S22UMRL+17D" */
    ORDERS(SSP, AXT_2),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(2),
    NUMBER(6),
    ORDER(AXT_1),
    NUMBER(18),
    /* S2231NXT */
    ORDERS(VLOAD, VXSC_STAR),
    ADDRESS_PLUS_ONE(S221X3),
    ADDRESS_PLUS_ONE_AT_X2(S223X1, 6),
    STORE_AT_X1(S22UMRL, 18),
    ORDERS(INCR_1, TIX_2),
    MINUS_NUMBER(6),
    ADDRESS(S2231NXT),
    ORDER(RVQ),
    /* GETTF: "SET MPAC= TF" */
    ORDERS(LXC_1, DLOAD_STAR),
    ADDRESS(S22LOC),
    ADDRESS_PLUS_ONE_X1(0),
    ORDER(RVQ),
    /* S22FLGS: "INTEGRATION FLAGS" */
    ORDERS(SET, SET),
    SWITCH(SET, DIM0FLAG),
    SWITCH(SET, D6OR9FLG),
    ORDERS(SET, SET),
    SWITCH(SET, VINTFLAG),
    SWITCH(SET, STATEFLG),
    ORDERS(CLEAR, RVQ),
    SWITCH(CLEAR, INTYPFLG),
    /* LLASRD: "ALT, LANDALT METERS B-29" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ALT),
    STODL(LANDALT),
    ADDRESS_PLUS_ONE(LONG),
    ORDER(SR1),
    STORE(LANDLONG),
    ORDER(RVQ),
    /* LLASRDA: "ALT, LANDALT METERS B-29" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(LANDALT),
    STODL(ALT),
    ADDRESS_PLUS_ONE(LANDLONG),
    ORDER(SL1),
    STORE(LONG),
    ORDER(RVQ),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S22_1_CODE = {030u, S22_1, COUNT(s22_1_words), s22_1_words};
const struct interpretive_code S22SHIFT_CODE = {030u, S22_1 + 033u, COUNT(s22shift_words),
                                                s22shift_words};
const struct interpretive_code S2231X13_CODE = {031u, S2231X13, COUNT(s2231x13_words),
                                                s2231x13_words};

#endif
