/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, LAMBERT (LM pp. 1193-1197, CM pp. 1296-1300),
 * in the listing's notation (src/interpreter/notation.h), which the
 * readable interpreter runs.
 *
 * "THIS SUBROUTINE CALCULATES THE INITIAL VELOCITY REQUIRED TO TRANSFER A
 * POINT-MASS ALONG A CONIC TRAJECTORY FROM AN INITIAL POSITION TO A
 * TERMINAL POSITION IN A PRESCRIBED TIME INTERVAL" (LM p. 1163, CM p.
 * 1266). In: R1VEC and R2VEC, the two positions; TDESIRED, the time; X1,
 * the central body (-2D for the earth, -10D for the moon); GEOMSGN, the
 * sign of the transfer angle's sine (negative past 180 degrees); GUESSW,
 * set if there is no guess of COGA; NORMSW and UN, the orbit's normal if
 * given; VTARGTAG, zero if the velocity at R2VEC is wanted too; ITERCTR,
 * the iterations allowed (the CM sets 20 itself). Out: VVEC, the velocity
 * at R1VEC, and VTARGET, the one at R2VEC; SOLNSW, set if there is no
 * solution.
 *
 * The independent variable is COGA, the cotangent of the flight path
 * angle at R1VEC (from the vertical). Its bounds come from the geometry
 * (COGAMIN, COGAMAX); each guess gives P and R1A, GETX the X and DELTIME
 * the time, and ITERATOR the next guess, until the time is close enough
 * (EPSILONL) or CHECKCTR's count runs out (SUFFCHEK). A guess with no
 * conic, or one whose time overflows, closes a bound and halves the step
 * (HIENERGY, LOENERGY, COMMONLM). INITV makes the velocity from COGA and
 * P, and TARGETV the one at R2VEC through LAMENTER. */
#include "conics.h"
#include "../agc/names.h"

static const word lambert_words[] = {
    /* LAMBERT -- LM p. 1193 (CM p. 1296): the return to RTNLAMB, SETPD 0
     * and BOV +1 (which clears the overflow indicator). The LM clears
     * SOLNSW here, the CM puts 20 in ITERCTR; both load the body's
     * constants from MUTABLE by X1 (1/MU, ROOTMU, 1/ROOTMU at 14D), and
     * EPSILONL, the time's tolerance, is TDESIRED times BEE19. SLOPESW
     * set (ITERATOR's first step), and GEOM for the two positions. */
    /* LAMBERT */ ORDERS(STQ, SETPD),
    ADDRESS(RTNLAMB),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(BOV),
    ADDRESS_AT(LAMBERT, 5), /* (+1) */
#if LUMINARY099
    ORDERS(CLEAR, VLOAD_STAR),
    SWITCH(CLEAR, SOLNSW),
#else
    ORDERS(SSP, VLOAD_STAR),
    ADDRESS_PLUS_ONE(ITERCTR),
    NUMBER(20), /* 20D */
#endif
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, 0),
    STODL(1_MU),
    ADDRESS_PLUS_ONE(TDESIRED),
    ORDER(DMPR),
    ADDRESS_PLUS_ONE(BEE19),
    STORE(EPSILONL),
    ORDERS(SET, VLOAD),
    SWITCH(SET, SLOPESW),
    ADDRESS_PLUS_ONE(R1VEC),
    ORDERS(PDVL, CALL),      /* "0D=R1VEC (+29 OR +27) PL AT 6" */
    ADDRESS_PLUS_ONE(R2VEC), /* "MPAC=R2VEC (+29 OR +27)" */
    ADDRESS(GEOM),
    /* The sine to SNTH; R1/R2 (normalized, shifted back) to 0D and the
     * cosine to CSTH, and 1-CSTH from it. At zero (CSTH 1), 360LAMB.
     * Then COGAMAX from R1/R2, 1-CSTH and SNTH: on an overflow, or above
     * COGUPLIM, COGUPLIM ("MAX VALUE OF COGA"). */
    STODL(SNTH), /* "0D=CSTH (+1) PL AT 2" */
    ADDRESS_PLUS_ONE(MAGVEC2),
#if LUMINARY099
    ORDERS(NORM, PDDL),
#else
    ORDERS(NORM, PDDL), /* "PL AT 4" */
#endif
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(SR1, DDV),      /* "PL AT 2" */
    ORDERS(SL_STAR, PDDL), /* "DXCH WITH 0D, 0D=R1/R2 (+7) PL AT 0,2" */
    SL_X1(-6),
    ORDER(STADR),
    AFTER_STADR(STORE(CSTH)), /* "CSTH (+1)" */
    ORDERS(SR1, BDSU),
    ADDRESS_PLUS_ONE(D1_4),
    STORE(1_CSTH), /* "1-CSTH (+2)" */
    ORDERS(ROUND, BZE),
    ADDRESS(360LAMB),
    ORDERS(NORM, PDDL), /* "PL AT 4" */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SR1, DDV), /* "PL AT 2" */
    ORDERS(SL_STAR, SQRT),
    SL_X1(-3),
    ORDERS(PDDL, SR), /* "2D=SQRT(2R1/R2(1-CSTH)) (+5) PL AT 4" */
    ADDRESS_PLUS_ONE(SNTH),
    SR_COUNT(6),
    ORDERS(DDV, DAD), /* "PL AT 2" */
    ADDRESS_PLUS_ONE(1_CSTH),
    ORDER(STADR),
    AFTER_STADR(STORE(COGAMAX)),
    ORDERS(BOV, BMN), /* "IF OVFL, COGAMAX=COGUPLIM" */
    ADDRESS(UPLIM),   /* "IF NEG, USE EVEN IF LT COGLOLIM, SINCE" */
    ADDRESS(MAXCOGA), /* "THIS WOULD BE RESET IN LAMBLOOP" */
    ORDERS(DSU, BMN), /* "IF COGAMAX GT COGUPLIM, COGAMAX=COGUPLIM" */
    ADDRESS_PLUS_ONE(COGUPLIM),
    ADDRESS(MAXCOGA), /* "OTHERWISE OK, SO GO TO MAXCOGA" */
    /* UPLIM */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(COGUPLIM), /* "COGUPLIM=.999511597 = MAX VALUE OF COGA" */
    STORE(COGAMAX),             /* "NOT CAUSING OVFL IN R1A CALCULATION" */
                                /* MAXCOGA: CSTH-RHO, from CSTH and the push-down list's R1/R2.
     * COGAMIN, CSTH-RHO over SNTH; past 180 degrees (GEOMSGN negative),
     * or on an overflow, COGLOLIM (LOLIM). */
    /* MAXCOGA */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(CSTH),
    ORDERS(SR, DSU), /* "PL AT 0" */
    SR_COUNT(6),
    ORDER(STADR),
    AFTER_STADR(STODL(CSTH_RHO)),
    ADDRESS_PLUS_ONE(GEOMSGN),
    ORDERS(BMN, DLOAD),
    ADDRESS(LOLIM),
    ADDRESS_PLUS_ONE(CSTH_RHO),
    ORDERS(SL1, DDV),
    ADDRESS_PLUS_ONE(SNTH),
    ORDER(BOV),
    ADDRESS(LOLIM),
    /* MINCOGA: with GUESSW set, NOGUESS; otherwise TWEEKIT 1, and the
     * guess in COGA. */
    /* MINCOGA */ STORE(COGAMIN), /* "COGAMIN (+5)" */
    ORDERS(BON, SSP),
    SWITCH(BON, GUESSW),
    ADDRESS(NOGUESS),
    ADDRESS_PLUS_ONE(TWEEKIT),
    NUMBER(1), /* 00001 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(COGA),
    /* LAMBLOOP -- LM p. 1194 (CM p. 1297): P, and R1A from it. Negative
     * or zero P is NEGP; an overflow, HIENERGY. GETX gives X (no X,
     * INFINFLG, is NEGP too), DELTIME the time, with the last kept in
     * TPREV (an overflow is BIGTIME). TERRLAMB, the time still wanted:
     * within EPSILONL, INITV. Otherwise CHECKCTR counts the iteration
     * (none left: SUFFCHEK), ITERATOR steps COGA, and a zero step is
     * SUFFCHEK too. */
    /* LAMBLOOP */ ORDER(DMP),
    ADDRESS_PLUS_ONE(SNTH),
    ORDERS(SR1, DSU),
    ADDRESS_PLUS_ONE(CSTH_RHO),
    ORDERS(NORM, PDDL), /* "0D=SNTH COGA-(CSTH-RHO)" */
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(1_CSTH),
    ORDERS(SL_STAR, DDV), /* "1-CSTH (+2) PL AT 0" */
    SL_X1(-9),
    ORDERS(BMN, BZE),
    ADDRESS(NEGP),
    ADDRESS(NEGP),
    STODL(P), /* "P=(1-CSTH)/(SNTH COGA-(CSTH-RHO)) (+4)" */
    ADDRESS_PLUS_ONE(COGA),
    ORDERS(DSQ, DAD),
    ADDRESS_PLUS_ONE(D1_1024),
    ORDERS(NORM, DMP),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(P),
    ORDERS(SR_STAR, BDSU),
    SR_X1(-8),
    ADDRESS_PLUS_ONE(D1_32),
    STODL(R1A), /* "R1A=2-P(1+COGA COGA) (+6)" */
    ADDRESS_PLUS_ONE(P),
    ORDERS(BOV, CALL),
    ADDRESS(HIENERGY),
    ADDRESS(GETX),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(T),
    STODL(TPREV),
    ADDRESS_PLUS_ONE(XI),
    ORDERS(BON, CALL),
    SWITCH(BON, INFINFLG),
    ADDRESS(NEGP), /* "HAVE EXCEEDED THEORETICAL BOUNDS" */
    ADDRESS(DELTIME),
    ORDERS(BOV, BDSU),
    ADDRESS(BIGTIME),
    ADDRESS_PLUS_ONE(TDESIRED),
    STORE(TERRLAMB),
    ORDERS(ABS, BDSU),
    ADDRESS_PLUS_ONE(EPSILONL),
    ORDERS(BPL, RTB),
    ADDRESS(INITV),
    ADDRESS(CHECKCTR),
    ORDERS(BHIZ, CALL),
    ADDRESS(SUFFCHEK),
    ADDRESS(ITERATOR),
    ORDERS(DLOAD, BZE),
    ADDRESS_PLUS_ONE(MPAC),
    ADDRESS(SUFFCHEK),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(COGA),
#if LUMINARY099
    STORE(COGA),
    ORDER(GOTO),
    ADDRESS(LAMBLOOP),
#else
    STCALL(COGA),
    ADDRESS(LAMBLOOP),
#endif
    /* NEGP: "IMPOSSIBLE TRAJECTORY DUE TO INACCURATE BOUND CALCULATION".
     * The last step's sign says which bound: LOENERGY for a positive
     * DCOGA, HIENERGY otherwise. */
    /* NEGP */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE(DCOGA),
    ADDRESS(LOENERGY),
    /* HIENERGY -- LM p. 1195 (CM p. 1298): COGA is the new COGAMIN. */
    /* HIENERGY */ ORDERS(SETPD, DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(COGA), /* "IN OVFL OF P OR R1A, OR XI EXCEEDING 50." */
    STORE(COGAMIN),         /* "THIS IS THE NEW BOUND." */
                            /* COMMONLM: DCOGA halved ("USE DCOGA/2 AS DECREMENT"), and taken
     * from COGA for LAMBLOOP again; when it reaches zero, SUFFCHEK. */
    /* COMMONLM */ ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(DCOGA),
    STORE(DCOGA),
    ORDERS(BZE, BDSU),
    ADDRESS(SUFFCHEK),
    ADDRESS_PLUS_ONE(COGA),
#if LUMINARY099
    STORE(COGA),
    ORDER(GOTO), /* "RESTART THIS LOOP" */
    ADDRESS(LAMBLOOP),
#else
    STCALL(COGA),
    ADDRESS(LAMBLOOP),
#endif
    /* BIGTIME: the time overflowed; T the last one (TPREV). */
    /* BIGTIME */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TPREV),
    STORE(T),
    /* LOENERGY: COGA is the new COGAMAX, and COMMONLM. */
    /* LOENERGY */ ORDERS(SETPD, DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(COGA), /* "IN OVERFLOW OF TIME." */
#if LUMINARY099
    STORE(COGAMAX), /* "THIS IS THE NEW BOUND." */
    ORDER(GOTO),
    ADDRESS(COMMONLM),
#else
    STCALL(COGAMAX), /* "THIS IS THE NEW BOUND." */
    ADDRESS(COMMONLM),
#endif
    /* SUFFCHEK -- LM p. 1196 (CM p. 1299): with the iterations out or the
     * step gone, is the time close enough after all? TDESIRED times
     * BEE17 (the CM's D1/4) plus ONEBIT, less TERRLAMB's size: not
     * negative, the velocity anyway (INITV). Otherwise SOLNSW set, and
     * INITV (the LM) or the return (the CM). */
    /* SUFFCHEK */ ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(TERRLAMB),
#if LUMINARY099
    ORDERS(PDDL, DMP), /* "PL AT 2D" */
    ADDRESS_PLUS_ONE(TDESIRED),
    ADDRESS_PLUS_ONE(BEE17),
#else
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE(TDESIRED), /* "PL AT 2D" */
    ADDRESS_PLUS_ONE(D1_4),
#endif
    ORDERS(DAD, DSU), /* "PL AT 0D" */
    ADDRESS_PLUS_ONE(ONEBIT),
    ORDERS(BPL, SETGO),
    ADDRESS(INITV),
    SWITCH(SETGO, SOLNSW),
#if LUMINARY099
    ADDRESS(INITV),
#else
    ADDRESS(RTNLAMB),
#endif
    /* 360LAMB: "LAMBERT CANNOT HANDLE CSTH=1": SOLNSW set, and back. */
    /* 360LAMB */ ORDERS(SETPD, SETGO),
    ADDRESS_PLUS_ONE_D(0),
    SWITCH(SETGO, SOLNSW),
    ADDRESS(RTNLAMB),
    /* NOGUESS: TWEEKIT 20000 (the operand, which yaYUL assembles as
     * 10000), and the first COGA halfway between the bounds, the first
     * step DCOGA the same. */
    /* NOGUESS */ ORDERS(SSP, DLOAD),
    ADDRESS_PLUS_ONE(TWEEKIT),
    NUMBER(010000),
    ADDRESS_PLUS_ONE(COGAMIN),
    ORDERS(SR1, PDDL), /* "PL AT 2" */
    ADDRESS_PLUS_ONE(COGAMAX),
    ORDERS(SR1, DAD),
    ORDER(STADR), /* "PL AT 0" */
    AFTER_STADR(STORE(COGA)),
#if LUMINARY099
    STORE(DCOGA),
    ORDER(GOTO),
    ADDRESS(LAMBLOOP),
#else
    STCALL(DCOGA),
    ADDRESS(LAMBLOOP),
#endif
    /* LOLIM: COGAMIN is COGLOLIM ("COGLOLIM=-.999511597"). */
    /* LOLIM */ ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(COGLOLIM),
    ADDRESS(MINCOGA),
    /* INITV -- LM p. 1197 (CM p. 1300): VTAN, the root of P over R1
     * times ROOTMU, the velocity across the radius ("0D=VTAN (+7)"); its
     * COGA multiple along UR1, plus VTAN along UN cross UR1, is VVEC.
     * (The CM clears SOLNSW here.) With VTARGTAG zero, TARGETV; otherwise
     * back through RTNLAMB. */
    /* INITV */ ORDERS(DLOAD, NORM),
    ADDRESS_PLUS_ONE(R1),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, SR1), /* "PL AT 2" */
    ADDRESS_PLUS_ONE(P),
    ORDER(DDV), /* "PL AT 0" */
    ORDERS(SL_STAR, SQRT),
    SL_X1(-4),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(ROOTMU),
    ORDERS(PUSH, DMP), /* "0D=VTAN (+7) PL AT 2" */
    ADDRESS_PLUS_ONE(COGA),
    ORDERS(SL, VXSC),
    SL_COUNT(5),
    ADDRESS_PLUS_ONE(UR1),
    ORDER(PDDL), /* "XCH WITH 0D PL AT 0,6" */
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(UN),
#if LUMINARY099
    ORDERS(VXV, VAD), /* "PL AT 0" */
    ADDRESS_PLUS_ONE(UR1),
    ORDER(VSL1),
#else
    ORDERS(VXV, VAD),
    ADDRESS_PLUS_ONE(UR1), /* "PL AT 0" */
    ORDERS(VSL1, CLEAR),
    SWITCH(CLEAR, SOLNSW),
#endif
    STORE(VVEC),
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(VTARGTAG),
    ADDRESS(TARGETV),
    ORDER(GOTO),
    ADDRESS(RTNLAMB),
    /* TARGETV: VTARGET, by LAMENTER from R2 (MAGVEC2), and back. */
    /* TARGETV */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(MAGVEC2),
    ADDRESS(LAMENTER),
#if LUMINARY099
    STORE(VTARGET),
    ORDER(GOTO),
    ADDRESS(RTNLAMB),
#else
    STCALL(VTARGET),
    ADDRESS(RTNLAMB),
#endif
};

const struct interpretive_code LAMBERT_CODE = {I_LAMBERT >> 10, K_LAMBERT, COUNT(lambert_words),
                                               lambert_words};
