/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 13 (Comanche055 pp. 610-614): 9DWTO6DW, which turns
 * the nine-dimensional W matrix S22.1 leaves into the six-dimensional one
 * of the CSM's state vector, and its ROWDOT (bank 30).
 *
 * W is the square root of the navigation filter's error covariance. With
 * the landmark's position in the filter it is nine by nine, 27 vectors of
 * three DP numbers from W +0 to W +161D: nine for the CSM's position, nine
 * for its velocity (from W +54D) and nine for the landmark (W6-W8, from W
 * +108D). A row of the matrix is one component of each of nine vectors,
 * its elements six words apart: rows 0-2 start at W +0, +2 and +4, rows
 * 3-5 at W +54D, +56D and +58D.
 *
 * 9DWTO6DW keeps the CSM's part of the covariance: E, the six by six
 * W W(T) of rows 0-5 over all nine columns, its 21 elements (one
 * triangle, each "E SUB P") from ROWDOT into EMATRIX, which sits in
 * W6-W8. Then, under a restart point, "FIGURE 2.4-6": W +0 to W +107D
 * cleared, and E's triangular square root built back into them a column
 * at a time, from row J=29 (the last velocity row) down: on the diagonal
 * "W= SQRT(WSQ)" (0 when it is negative), below it "(EP-ROWI*ROWJ)/W" (0
 * on overflow). W6-W8 are cleared again at the end.
 *
 * The rows are counted in the work area as word offsets: I and J (9DWI,
 * 9DWJ) run 58, 56, 54 and then 4, 2, 0 ("TEST I=26" jumps from 26 to 2),
 * P (9DWP) over EMATRIX's 21 elements, N (9DWN) the column's six words.
 * The return goes to 9DWXX (S22UOFF, free here) and the routine comes back
 * through it with a GOTO. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* 9DWTO6DW -- pp. 610-613. The return in 9DWXX; "CLEAR WORKING AREA OF
 * PUSHLIST" ("INCLUDING P"), its first 18 words, and J at 58 ("J=29",
 * "USE 2*29 FOR DP WORDS").
 *
 * 9DWI=J: "SET I=J". 9DWEPCAL: E's element P is ROWDOT, row I by row J
 * over the nine columns, stored at EMATRIX +40D less P ("P VARIES 0-20
 * INSTEAD OF 20-0"), and P on by 2. Then I on down: "TEST I=0", "I=I-1",
 * "TEST I=26" (9DWSETI2, "I=2"), and "NEXT E SUB P". With I at 0,
 * 9DWTESTJ: "TEST J=0", "J=J-1", "TEST J=26" (9DWSETJ2, "SET J=2"), on
 * at 9DWI=J.
 *
 * 9DWFIG6: a restart point (GRP2PC), "START OF FIGURE 2.4-6": J at 58
 * again, "P,N,I=0", and "CLEAR W0 TO W54" (CLEARW54, six words at a time
 * by X1 from W +108D down).
 *
 * 9DWI=JA: "I=J"; the element of E less ROWDOT of rows I and J of the new
 * W so far (P on, "-(P+1)"), X1 at "-(I+N)": "TEST WSQ LTE 0", 0 (9DWAAB)
 * or "W= SQRT(WSQ)" (9DWAAA) stored at W +I+N, and kept in WORKW. "TEST
 * J=0": done, at 9DWEXITX. TST2I=0: "TEST I=0" (on at 9DWN=N+3), "I=I-1",
 * "TEST I=26" (9DWAAC, "I=2"). 9DWNEXEP: "(EP-ROWI*ROWJ)/W", "P=P+1", and
 * stored at W +I+N, or 0 on overflow (SETWIN=0, "W(I+N)=0"; 9DWSETWX), on
 * at TST2I=0. 9DWN=N+3: "N=N+3" (six words), "J=J-1", "TEST J=26"
 * (SETJ=2A, "J=2"), on at 9DWI=JA.
 *
 * 9DWEXITX: a restart point, "CLEAR W6,W7,W8 USED TEMP FOR EMATRIX" (by
 * X1 from W +162D down, at 9DWEXXXA), and back through 9DWXX. */
static const word nine_dwto6dw_words[] = {
    ORDERS(STQ, SETPD),
    ADDRESS(9DWXX),
    ADDRESS_PLUS_ONE_D(0),
    /* "CLEAR WORKING AREA OF PUSHLIST" */
    ORDERS(VLOAD, PUSH),
    /* "INCLUDING P" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* "PD 18D" */
    ORDERS(PUSH, PUSH),
    ORDER(SSP),
    /* "J=29" */
    ADDRESS_PLUS_ONE(9DWJ),
    NUMBER(58),
    /* 9DWI=J: "SET I=J" */
    ORDERS(LXA_1, SXA_1),
    ADDRESS(9DWJ),
    ADDRESS(9DWI),
    /* 9DWEPCAL */
    ORDER(CALL),
    ADDRESS(ROWDOT),
    /* "P VARIES 0-20 INSTEAD OF 20-0" */
    ORDER(LXA_1),
    ADDRESS(9DWP),
    STORE_AT_X1(EMATRIX, 40),
    ORDERS(INCR_1, SXA_1),
    NUMBER(2),
    ADDRESS(9DWP),
    /* "TEST I=0" */
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(9DWI),
    ADDRESS(9DWTESTJ),
    /* "I=I-1" */
    ORDER(DSU),
    ADDRESS_PLUS_ONE(9DWID),
    STORE(9DWI),
    /* "TEST I=26" */
    ORDERS(DSU, BHIZ),
    ADDRESS_PLUS_ONE(9DW26D),
    ADDRESS(9DWSETI2),
    /* "NEXT E SUB P" */
    ORDER(GOTO),
    ADDRESS(9DWEPCAL),
    /* 9DWSETI2: "I=2" */
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(9DWI),
    NUMBER(4),
    ADDRESS(9DWEPCAL),
    /* 9DWTESTJ: "TEST J=0" */
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(9DWJ),
    ADDRESS(9DWFIG6),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(9DWID),
    /* "J=J-1" */
    STORE(9DWJ),
    /* "TEST J=26" */
    ORDERS(DSU, BHIZ),
    ADDRESS_PLUS_ONE(9DW26D),
    ADDRESS(9DWSETJ2),
    ORDER(GOTO),
    ADDRESS(9DWI_EQ_J),
    /* 9DWSETJ2: "SET J=2" */
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(9DWJ),
    NUMBER(4),
    ADDRESS(9DWI_EQ_J),
    /* 9DWFIG6 */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    /* "START OF FIGURE 2.4-6" */
    ORDERS(SSP, VLOAD),
    /* "J=29" */
    ADDRESS_PLUS_ONE(9DWJ),
    NUMBER(58),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* "P,N,I=0" */
    STORE(9DWP),
    ORDERS(AXT_1, SSP),
    /* "CLEAR W0 TO W54" */
    NUMBER(108),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    /* CLEARW54 */
    STORE_AT_X1(W, 108),
    ORDER(TIX_1),
    ADDRESS(CLEARW54),
    /* 9DWI=JA: "I=J" */
    ORDERS(LXA_1, SXA_1),
    ADDRESS(9DWJ),
    ADDRESS(9DWI),
    ORDER(CALL),
    ADDRESS(ROWDOT),
    ORDERS(LXA_1, BDSU_STAR),
    ADDRESS(9DWP),
    ADDRESS_PLUS_ONE_AT_X1(EMATRIX, 40),
    /* "-(P+1)" */
    ORDERS(INCR_1, SXA_1),
    NUMBER(2),
    ADDRESS(9DWP),
    /* "-(I+N)" */
    ORDERS(LXC_1, XSU_1),
    ADDRESS(9DWI),
    ADDRESS(9DWN),
    /* "TEST WSQ LTE 0" */
    ORDERS(BPL, DLOAD),
    ADDRESS(9DWAAA),
    /* "W=0" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDER(GOTO),
    ADDRESS(9DWAAB),
    /* 9DWAAA: "W= SQRT(WSQ)" */
    ORDER(SQRT),
    /* 9DWAAB */
    STORE_AT_X1(W, 0),
    STODL(WORKW),
    /* "TEST J=0" */
    ADDRESS_PLUS_ONE(9DWJ),
    ORDER(BHIZ),
    /* "EXIT" */
    ADDRESS(9DWEXITX),
    /* TST2I=0: "TEST I=0" */
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(9DWI),
    ADDRESS(9DWN_EQ_NP3),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(9DWID),
    /* "I=I-1" */
    STORE(9DWI),
    /* "TEST I=26" */
    ORDERS(DSU, BHIZ),
    ADDRESS_PLUS_ONE(9DW26D),
    ADDRESS(9DWAAC),
    ORDER(GOTO),
    ADDRESS(9DWNEXEP),
    /* 9DWAAC: "I=2" */
    ORDER(SSP),
    ADDRESS_PLUS_ONE(9DWI),
    NUMBER(4),
    /* 9DWNEXEP */
    ORDER(CALL),
    ADDRESS(ROWDOT),
    /* "(EP-ROWI*ROWJ)/W" */
    ORDERS(LXA_1, BDSU_STAR),
    ADDRESS(9DWP),
    ADDRESS_PLUS_ONE_AT_X1(EMATRIX, 40),
    /* "P=P+1" */
    ORDERS(DDV, INCR_1),
    ADDRESS_PLUS_ONE(WORKW),
    NUMBER(2),
    ORDERS(SXA_1, LXC_1),
    ADDRESS(9DWP),
    ADDRESS(9DWI),
    /* "-(I+N)" */
    ORDERS(XSU_1, BOV),
    ADDRESS(9DWN),
    ADDRESS(SETWIN_EQ_0),
    ORDER(GOTO),
    ADDRESS(9DWSETWX),
    /* SETWIN=0: "W(I+N)=0" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* 9DWSETWX */
    STORE_AT_X1(W, 0),
    ORDER(GOTO),
    ADDRESS(TST2I_EQ_0),
    /* 9DWN=N+3: "N=N+3" */
    ORDERS(LXA_1, INCR_1),
    ADDRESS(9DWN),
    NUMBER(6),
    /* "J=J-1" */
    ORDERS(SXA_1, SLOAD),
    ADDRESS(9DWN),
    ADDRESS_PLUS_ONE(9DWJ),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(9DWID),
    STORE(9DWJ),
    /* "TEST J=26" */
    ORDERS(DSU, BHIZ),
    ADDRESS_PLUS_ONE(9DW26D),
    ADDRESS(SETJ_EQ_2A),
    ORDER(GOTO),
    ADDRESS(9DWI_EQ_JA),
    /* SETJ=2A: "J=2" */
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(9DWJ),
    NUMBER(4),
    ADDRESS(9DWI_EQ_JA),
    /* 9DWEXITX */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    /* "CLEAR W6,W7,W8 USED TEMP FOR EMATRIX" */
    ORDERS(AXT_1, SSP),
    NUMBER(54),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* 9DWEXXXA */
    STORE_AT_X1(W, 162),
    ORDERS(TIX_1, GOTO),
    ADDRESS(9DWEXXXA),
    ADDRESS(9DWXX),
};

/* ROWDOT -- pp. 613-614: row I of W times row J (9DWI, 9DWJ), as a DP
 * number. XTMP1 counts the terms (0377 shifted right until it is 0, nine
 * times); BOV +1 clears the overflow indicator first. X1 and X2 are -I
 * and -J, so W,1 and W,2 are the rows' elements; the sum starts at 0 on
 * the push-down list. ROWDOT1: an element of row I by row J's, added to
 * the sum; both rows on to the next column (-6 in each index); and the
 * count shifted down. ROWDOT2: the sum, and back (RVQ). On overflow
 * (ROWDOT3) W is no longer of use: ORBWFLAG cleared, and back the same
 * way. */
static const word rowdot_words[] = {
    ORDERS(SSP, BOV),
    ADDRESS_PLUS_ONE(XTMP1),
    NUMBER(0377),
    ADDRESS_AT(ROWDOT, 4), /* +1 */
    ORDERS(LXC_1, LXC_2),
    ADDRESS(9DWI),
    ADDRESS(9DWJ),
    ORDERS(DLOAD, PUSH),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    /* ROWDOT1 */
    ORDERS(DLOAD_STAR, DMPR_STAR),
    ADDRESS_PLUS_ONE_AT_X1(W, 0),
    ADDRESS_PLUS_ONE_AT_X2(W, 0),
    ORDERS(DAD, PUSH),
    ORDERS(BOV, INCR_1),
    ADDRESS(ROWDOT3),
    MINUS_NUMBER(6),
    ORDERS(INCR_2, SLOAD),
    MINUS_NUMBER(6),
    ADDRESS_PLUS_ONE(XTMP1),
    ORDERS(BHIZ, SR1),
    ADDRESS(ROWDOT2),
    STORE(XTMP1),
    ORDER(GOTO),
    ADDRESS(ROWDOT1),
    /* ROWDOT2 */
    ORDER(DLOAD),
    ORDER(RVQ),
    /* ROWDOT3 */
    ORDER(CLRGO),
    SWITCH(CLRGO, ORBWFLAG),
    ADDRESS(ROWDOT2),
};

/* ---- The interpretive code ---- */

const struct interpretive_code NINE_DWTO6DW_CODE = {030u, NINE_DWTO6DW, COUNT(nine_dwto6dw_words),
                                                    nine_dwto6dw_words};
const struct interpretive_code ROWDOT_CODE = {030u, ROWDOT, COUNT(rowdot_words), rowdot_words};

#endif
