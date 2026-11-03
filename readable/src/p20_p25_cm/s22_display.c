/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 12 (Comanche055 pp. 606-609): the end of S22.1, the
 * landmark's position shown, and two of its subroutines (bank 30).
 *
 * S22BOX42 finishes a mark's incorporation (INCORP2, the CSM's position
 * again from S22CALRC); a mark that was not the landmark's first goes on
 * at S22SAVET, and the first, which has the landmark's position too, goes
 * round S22BOX12 once more. S22BOX22, the first mark of a known landmark:
 * W6-W8 cleared but for the diagonals' S22WSUBL, and the landmark's
 * position from its latitude, longitude and altitude (LALOTORV), on at
 * S22BOX32.
 *
 * Past the last mark, S22F244 ("FIG 2.4-4"): with an offset point
 * (CXOFF), the CSM's position at the offset time and the landmark's from
 * the line of sight then (S22F2410); the landmark's latitude, longitude
 * and altitude (LAT-LONG, LLASRD) are shown with verb 06 noun 89 for the
 * crew to terminate (S22GTP), proceed (the landmark kept as the landing
 * site, RLS), or recycle (S22.981X), each through 9DWTO6DW (part 13),
 * which converts the W matrix.
 *
 * S22F2410, "COMPUTE FORMULA 2.4.10": where the line of sight from the
 * CSM meets the surface; S22CALRC: the CSM's position from its state
 * vector's deviation (DELTACSM) and reference (RCVCSM).
 *
 * Each call out of the file (BANKCALL) leaves Z at the routine and Q at
 * the word after the TC, where the routine comes back to; GOTOPOOH
 * doesn't come back. GOFLASH comes back to the word after its CADR on
 * terminate, the next on proceed, and the next on enter (recycle). */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void s22gtp(void);

/* S22BOX42 -- pp. 606-607: INCORP2, the state vector's change made, and
 * S22CALRC, "CSMPOS=RC B-29 OR B-27". DMPINCP2: with INCORFLG clear,
 * S22SAVET, "SAVE TF AND TEST I=N"; set (the landmark's position just
 * found), a restart point (GRP2PC), "FLAG=0", "RCL=RL-RC" again and on at
 * S22BOX12.
 *
 * S22BOX22: "CLEAR W6,W7,W8.  (27 ELEMENTS 54 REGS)", six words at a time
 * by X1 (CLRW678), then S22WSUBL, the landmark's variance, in the
 * diagonals W +144D, W +152D and W +160D. "SET LUNAFLAG, TIME FOR
 * LALOTORV" (LUNAFLAG as CMOONFLG); S22BX22A: the mark's time (GETTF) and
 * "COMPUTE RL" (LALOTORV), ALPHAV's "RL B-29", "SCALE RL B-27 FOR MOON";
 * S22BX22B: on at S22BOX32. */
static const word s22box42_words[] = {
    ORDER(CALL),
    ADDRESS(INCORP2),
    /* "CSMPOS=RC B-29 OR B-27" */
    ORDER(CALL),
    ADDRESS(S22CALRC),
    /* DMPINCP2 */
    ORDERS(BOFF, CALL),
    SWITCH(BOF, INCORFLG),
    /* "SAVE TF AND TEST I=N" */
    ADDRESS(S22SAVET),
    ADDRESS(GRP2PC),
    ORDERS(CLEAR, VLOAD),
    /* "FLAG=0" */
    SWITCH(CLEAR, INCORFLG),
    ADDRESS_PLUS_ONE(X789),
    ORDER(VSU),
    ADDRESS_PLUS_ONE(CSMPOS),
    /* "RCL=RL-RC" */
    STCALL(RCLP),
    ADDRESS(S22BOX12),
    /* S22BOX22: "CLEAR W6,W7,W8.  (27 ELEMENTS 54 REGS)" */
    ORDERS(AXT_1, SSP),
    NUMBER(54),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    /* CLRW678 */
    STORE_AT_X1(W, 162),
    ORDERS(TIX_1, SLOAD),
    ADDRESS(CLRW678),
    ADDRESS_PLUS_ONE(S22WSUBL),
    STORE_AT(W, 144),
    STORE_AT(W, 152),
    STORE_AT(W, 160),
    /* "SET LUNAFLAG, TIME FOR LALOTORV" */
    ORDERS(CLEAR, BOFF),
    /* "ERADFLAG,LAT,LONG,ALT SET PREVIOUSLY" */
    SWITCH(CLEAR, LUNAFLAG),
    /* "CHECK SCALING OF ITEMS,ALT INPUT AND" */
    SWITCH(BOF, CMOONFLG),
    /* "RL OUTPUT IN ALPHAV BOTH B-29" */
    ADDRESS(S22BX22A),
    ORDER(SET),
    SWITCH(SET, LUNAFLAG),
    /* S22BX22A */
    ORDER(CALL),
    ADDRESS(GETTF),
    /* "COMPUTE RL" */
    ORDER(CALL),
    ADDRESS(LALOTORV),
    ORDERS(VLOAD, BOFF),
    /* "RL B-29" */
    ADDRESS_PLUS_ONE(ALPHAV),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(S22BX22B),
    /* "SCALE RL B-27 FOR MOON" */
    ORDER(VSL2),
    /* S22BX22B */
    ORDER(GOTO),
    ADDRESS(S22BOX32),
};

/* S22F244 -- pp. 607-608, "FIG 2.4-4": "TEST OFF=0" (CXOFF, the offset
 * point's mark number); none, on at S22BOX44. Otherwise "SCALE OFFSET
 * B-14 THEN GET GR. 8NN" (SR 9D, into ALPHAV as a "TEMP"): an offset past
 * the marks taken, "OFFSET GR. NO. MARKS.  FORGET IT" (S22BOX44). Else a
 * "GROUP 2 PHASE CHANGE" (GRP2PC), and "CALC RC AT OFFSET TIME" (CSMPREC
 * to S22TOFF): CSMPOS from RATT1, "RC METERS B-29 OR B-27", and "U=UOFF";
 * ERADM, X789's length ("SCALE MOON R0 B-29 FOR S22F2410 SUBR"), and the
 * landmark's position at the offset time from S22F2410; on at S22BX44A.
 *
 * S22BOX44: S22TOFF the mark's time, "PRESENT TIME FOR LAT-LONG SETUP".
 * S22BX44A: LUNAFLAG "SET = 1 FOR LAT-LONG" for the moon, and "SCALE RL
 * MOON B-29 FOR LAT-LONG"; S22BX44B: "RL SCALED B-29 FOR LAT-LONG" in
 * ALPHAV, and S22TOFF, "EITHER PRESENT OR OFFSET TIME", to LAT-LONG ("ALT
 * OUTPUT ALWAYS B-29") and LLASRD: "DISPLAY LAT/LONG/ALT". The display is
 * in basic code. */
static const word s22f244_words[] = {
    /* "FIG 2.4-4", "TEST OFF=0" */
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(CXOFF),
    ADDRESS(S22BOX44),
    /* "SCALE OFFSET B-14 THEN GET GR. 8NN" */
    ORDER(SR),
    SR_COUNT(9),
    /* "TEMP" */
    STORE(ALPHAV),
    ORDERS(SLOAD, DSU),
    ADDRESS_PLUS_ONE(8NN),
    ADDRESS_PLUS_ONE(ALPHAV),
    /* "OFFSET GR. NO. MARKS.  FORGET IT" */
    ORDERS(BMN, CALL),
    ADDRESS(S22BOX44),
    /* "GROUP 2 PHASE CHANGE" */
    ADDRESS(GRP2PC),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(S22TOFF),
    /* "CALC RC AT OFFSET TIME" */
    STCALL(TDEC1),
    ADDRESS(CSMPREC),
    ORDER(VLOAD),
    /* "RC METERS B-29 OR B-27" */
    ADDRESS_PLUS_ONE(RATT1),
    STOVL(CSMPOS),
    ADDRESS_PLUS_ONE(S22UOFF),
    /* "U=UOFF" */
    STOVL(UM),
    ADDRESS_PLUS_ONE(X789),
    ORDERS(ABVAL, BOFF),
    SWITCH(BOF, CMOONFLG),
    ADDRESS_AT(S22F244, 032), /* +2 */
    /* "SCALE MOON R0 B-29 FOR S22F2410 SUBR" */
    ORDER(SR2),
    STCALL(ERADM),
    ADDRESS(S22F2410),
    ORDER(GOTO),
    ADDRESS(S22BX44A),
    /* S22BOX44 */
    ORDER(CALL),
    ADDRESS(GETTF),
    /* "PRESENT TIME FOR LAT-LONG SETUP" */
    STORE(S22TOFF),
    /* S22BX44A */
    ORDERS(CLEAR, VLOAD),
    SWITCH(CLEAR, LUNAFLAG),
    ADDRESS_PLUS_ONE(X789),
    ORDERS(BOFF, SET),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(S22BX44B),
    /* "SET = 1 FOR LAT-LONG" */
    SWITCH(SET, LUNAFLAG),
    /* "SCALE RL MOON B-29 FOR LAT-LONG" */
    ORDER(VSR2),
    /* S22BX44B: "RL SCALED B-29 FOR LAT-LONG" */
    STODL(ALPHAV),
    /* "EITHER PRESENT OR OFFSET TIME" */
    ADDRESS_PLUS_ONE(S22TOFF),
    ORDER(CALL),
    /* "**** ALT OUTPUT ALWAYS B-29" */
    ADDRESS(LAT_LONG),
    /* "DISPLAY LAT/LONG/ALT" */
    ORDER(CALL),
    ADDRESS(LLASRD),
    ORDER(EXIT),
};

/* S22.1 +0730 -- after the EXIT: verb 06 noun 89 (V06N89B), the landmark's
 * latitude, longitude and altitude, flashed by GOFLASH. */
void s22f244_after_exit(void)
{
    set_A(V06N89B);
    tc_to(S22_1 + 0731u, BANKCALL); /* CADR GOFLASH */
}

/* S22.1 +0733: "V34E TERMINATE" (S22GTP). */
void s22f244_terminate(void)
{
    tc_at(S22_1 + 0733u);
    s22gtp();
}

/* S22.1 +0734: proceed, "SAVE LANDING SITE COORD", TC +2: */
void s22f244_proceed(void)
{
    tc_at(S22_1 + 0734u);
    s22_landing_site();
}

/* S22.1 +0735: recycle, "POINT A IN GSOP" (S22.981X). */
void s22f244_recycle(void)
{
    tc_at(S22_1 + 0735u);
    s22_981x();
}

/* S22.1 +0736: TC INTPRET, */
void s22_landing_site(void)
{
    tc_at(S22_1 + 0736u);
    intpret();
}

/* S22.1 +0737 -- p. 608: the landmark kept as the landing site, at
 * S22TOFF ("6-7D= LANDING SITE TIME FOR R-TO-RP"), X789 ("0-5D= LANDING
 * SITE VEC FOR R-TO-RP"), the moon's ("ANY NON-ZERO FOR MOON"): R-TO-RP,
 * "CONVERT RLS TO MOON-FIXED COORD", in RLS, and on into S22.981X. */
static const word s22rls_words[] = {
    ORDER(DLOAD),
    /* "EITHER PRESENT OR OFFSET TIME" */
    ADDRESS_PLUS_ONE(S22TOFF),
    /* "6-7D= LANDING SITE TIME FOR R-TO-RP" */
    STOVL_D(6),
    ADDRESS_PLUS_ONE(X789),
    /* "0-5D= LANDING SITE VEC FOR R-TO-RP" */
    STORE_D(0),
    ORDERS(SLOAD, CALL),
    /* "ANY NON-ZERO FOR MOON" */
    ADDRESS_PLUS_ONE(HIDPHALF),
    /* "CONVERT RLS TO MOON-FIXED COORD" */
    ADDRESS(R_TO_RP),
    /* "LANDING SITE VECTOR" */
    STORE(RLS),
    ORDER(EXIT),
};

/* S22.981X: TC INTPRET, */
void s22_981x(void)
{
    tc_at(S22_981X);
    intpret();
}

/* S22.981X +1: the W matrix converted (9DWTO6DW), and "GO TO POINT A IN
 * CHAPTER 5": the EXIT goes on into S22EXEX. */
static const word s22_981x_words[] = {
    ORDER(CALL),
    ADDRESS(9DWTO6DW),
    /* "GO TO POINT A IN CHAPTER 5" */
    ORDER(EXIT),
};

/* S22EXEX: TC INTPRET, "WITHOUT CONVERTING W", */
void s22exex(void)
{
    tc_at(S22EXEX);
    intpret();
}

/* S22EXEX +1: to S22RTNEX (part 9), the routine's exit. */
static const word s22exex_words[] = {
    ORDER(GOTO),
    ADDRESS(S22RTNEX),
};

/* S22GTP: TC INTPRET, "CONVERT W BEFORE TC GOTOPOOH": */
static void s22gtp(void)
{
    tc_at(S22GTP);
    intpret();
}

/* S22GTP +1 */
static const word s22gtp_words[] = {
    ORDER(CALL),
    ADDRESS(9DWTO6DW),
    ORDER(EXIT),
};

/* S22GTP +4 -- after the EXIT: */
void s22gtp_after_exit(void)
{
    tc_to(S22GTP + 4u, GOTOPOOH);
}

/* S22F2410 -- pp. 608-609, "COMPUTE FORMULA 2.4.10": the distance along
 * the line of sight UM from the CSM's position (CSMPOS, "RC B-29 EARTH,
 * B-27 MOON") to the surface, radius ERADM. "UNIT ALSO SETS 36D=ABVAL(RC)
 * USED BELOW"; "8D=COSA=-(UM.RC)/ABVAL(RC)" ("GSOP CHANGE 8/18/67"),
 * "2D=1-COSA SQ=SINA SQ", "(R0/RC)", "(RP/RC) SQ - SINA SQ", and
 * "COSA-SQRT((R0/RC)SQ-SINA SQ)", times ABVAL(RC): "RHO FOR W INIT. OF
 * UNKNOWN LMK B-28,B-30" in S22RHO. The landmark, UM times RHO "AND ADD
 * RC", in X789.
 *
 * S22CALRC -- p. 609: "COMPUTE RC B-29 OR B-27": DELTACSM, shifted by
 * S22EORM ("=0 FOR EARTH, -2 FOR MOON"), plus RCVCSM, in CSMPOS. */
static const word s22f2410_words[] = {
    /* "COMPUTE FORMULA 2.4.10" */
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    /* "RC B-29 EARTH, B-27 MOON" */
    ADDRESS_PLUS_ONE(CSMPOS),
    /* "UNIT ALSO SETS 36D=ABVAL(RC) USED BELOW" */
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE(UM),
    /* "GSOP CHANGE 8/18/67" */
    ORDERS(SL1, DCOMP),
    /* "PD 2D 8D=COSA=-(UM.RC)/ABVAL(RC)" */
    ORDER(PUSH),
    ORDERS(DSQ, BDSU),
    ADDRESS_PLUS_ONE(DEC1B2),
    /* "PD 4D 2D=1-COSA SQ=SINA SQ" */
    ORDERS(PDDL, BOFF),
    /* "R0 ALWAYS B-29 FROM SETRE" */
    ADDRESS_PLUS_ONE(ERADM),
    SWITCH(BOF, CMOONFLG),
    ADDRESS_AT(S22F2410, 016), /* +2 */
    /* "SCALE R0 B-27 FOR MOON" */
    ORDER(SL2),
    /* "(R0/RC)" */
    ORDERS(SR1R, DDV),
    ADDRESS_PLUS_ONE_D(36),
    /* "PD 2D (RP/RC) SQ - SINA SQ" */
    ORDERS(DSQ, DSU),
    /* "PD 0D COSA-SQRT((R0/RC)SQ-SINA SQ)" */
    ORDERS(SQRT, BDSU),
    /* "DMP RESULT B-28 MOON, B-30 EARTH" */
    ORDER(DMP),
    /* "VXSC RESULT B-29 MOON, B-31 EARTH" */
    ADDRESS_PLUS_ONE_D(36),
    /* "RHO FOR W INIT. OF UNKNOWN LMK B-28,B-30" */
    STORE(S22RHO),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(UM),
    /* "SCALE B-27 MOON, B-29 EARTH AND ADD RC" */
    ORDERS(VSL2, VAD),
    ADDRESS_PLUS_ONE(CSMPOS),
    STORE(X789),
    /* "B-27 FOR EARTH OR B-29 FOR MOON" */
    ORDER(RVQ),
    /* S22CALRC: "COMPUTE RC B-29 OR B-27" */
    ORDERS(LXA_1, VLOAD),
    /* "=0 FOR EARTH, -2 FOR MOON" */
    ADDRESS(S22EORM),
    ADDRESS_PLUS_ONE(DELTACSM),
    ORDERS(VSR_STAR, VAD),
    SR_X1(7),
    ADDRESS_PLUS_ONE(RCVCSM),
    STORE(CSMPOS),
    ORDER(RVQ),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S22BOX42_CODE = {030u, S22_1 + 0572u, COUNT(s22box42_words),
                                                s22box42_words};
const struct interpretive_code S22F244_CODE = {030u, S22F244X + 1u, COUNT(s22f244_words),
                                               s22f244_words};
const struct interpretive_code S22RLS_CODE = {030u, S22_1 + 0737u, COUNT(s22rls_words),
                                              s22rls_words};
const struct interpretive_code S22_981X_CODE = {030u, S22_981X + 1u, COUNT(s22_981x_words),
                                                s22_981x_words};
const struct interpretive_code S22EXEX_CODE = {030u, S22EXEX + 1u, COUNT(s22exex_words),
                                               s22exex_words};
const struct interpretive_code S22GTP_CODE = {030u, S22GTP + 1u, COUNT(s22gtp_words), s22gtp_words};
const struct interpretive_code S22F2410_CODE = {030u, S22GTP + 5u, COUNT(s22f2410_words),
                                                s22f2410_words};

#endif
