/* SPDX-License-Identifier: MIT */
/* STABLE_ORBIT_-_P38-P39.agc (Luminary099 pp. 723-730, Comanche055 pp.
 * 525-532): the "STABLE ORBIT RENDEZVOUS PROGRAMS (P38 AND P78)" and the
 * "STABLE ORBIT MIDCOURSE PROGRAM (P39 AND P79)". Its basic code, and its
 * interpretive code in the listing's notation (src/interpreter/
 * notation.h), which the readable interpreter runs (src/binding/
 * stable_orbit_routines.c). The faithful C is c64-port/src/
 * lm_stable_orbit.c and cm_stable_orbit.c. The LM's is in bank 34, the
 * CM's in bank 4.
 *
 * P38 and P78 "CALCULATE THE REQUIRED DELTA V AND OTHER INITIAL CONDITIONS
 * REQUIRED BY THE AGC TO (1) PUT THE ACTIVE VEHICLE ON A TRANSFER
 * TRAJECTORY THAT INTERCEPTS THE PASSIVE VEHICLE ORBIT A GIVEN DISTANCE,
 * DELTA R, EITHER AHEAD OF OR BEHIND THE PASSIVE VEHICLE" (the SOI
 * maneuver) "AND (2) ACTUALLY PLACE THE ACTIVE VEHICLE IN THE PASSIVE
 * VEHICLE ORBIT" (the SOR maneuver). The crew loads TIG and CENTANG (verb
 * 06 noun 33, then noun 55), the option (verb 04 noun 06: "1 = SOI, 2 =
 * SOR") and for the SOI DELTA R (noun 57). For the SOI, PREC/TT works out
 * the passive vehicle's time from TIG over CENTANG, which with TIG is the
 * time of intercept, TINT (noun 34), and DELTTIME the time it takes to
 * cover DELTA R; for the SOR, TINT is the SOI's TINTSOI plus that time.
 * TARGTIME, TINT less DELTTIME, is where the passive vehicle is (INTRPVP)
 * when the active one meets its orbit: MAINRTNE has S3435.25 work out the
 * burn, shows DELTA V and the perigee after it (noun 58) and the burn in
 * local vertical coordinates (noun 81), and VN1645 shows the marks, time to
 * go and middle gimbal angle. P39 and P79 "MAKE A MIDCOURSE CORRECTION
 * MANEUVER AFTER COMPLETING THE SOI MANEUVER BUT BEFORE MAKING THE SOR
 * MANEUVER": TIG is now plus the time to prepare (ATIGINC or PTIGINC), and
 * TARGTIME the P38's.
 *
 * VNDSPLY flashes a display (GOFLASH) and keeps its caller's return in
 * RTRN (QXCH RTRN); on proceed, TC RTRN runs that word, TC to the word
 * after the caller's TC VNDSPLY. In the CM's code RTRN is a word in
 * erasable bank 4 (E4,1767), which the routine leaves Z at; the LM's goes
 * on.
 *
 * Each routine ends where its code leaves it. A call out of the file
 * (BANKCALL, the CM's AVFLAGA, AVFLAGP and P20FLGON, BLANKET) or a TC
 * INTPRET leaves Z at the routine and Q at the word after the TC (for
 * INTPRET the first interpretive word); a TCF out of the file (GOTOPOOH,
 * ENDOFJOB) leaves Q as it was. A TC or TCF within the file (TC +3, TC
 * VNDSPLY, TCF -5 ...) goes on into the code it names. Each TS after a CAF
 * or a QXCH would skip the next word on an overflow, which a CAF never
 * leaves. */
#include "stable_orbit.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#if LUMINARY099
#define STABLE_ORBIT_BANK 034u
#else
#define STABLE_ORBIT_BANK 04u
#endif

/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

/* TC at, to a routine out of the file or to INTPRET: Q gets the return
 * (for INTPRET, the first interpretive word), and the routine runs
 * next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

static void vndsply(void);
static void p38_option(void);
static void p38_option_code(void);
static void p38_intercept(void);
static void p39_p79a(void);

/* ---- P38 and P78 -- LM pp. 724-725, CM pp. 526-527 ---- */

/* P38: "THIS VEHICLE ACTIVE" (AVFLAGA, the LM's by BANKCALL), */
void p38(void)
{
#if LUMINARY099
    tc_to(P38, BANKCALL); /* CADR AVFLAGA */
#else
    tc_to(P38, AVFLAGA);
#endif
}

/* and on past P78's AVFLAGP (the LM's TC +3, the CM's TC +2). */
void p38_active(void)
{
#if LUMINARY099
    tc_at(P38 + 2u); /* TC +3 */
#else
    tc_at(P38 + 1u); /* TC +2 */
#endif
    p78_active();
}

/* P78: "OTHER VEHICLE ACTIVE" (AVFLAGP); */
void p78(void)
{
#if LUMINARY099
    tc_to(P78, BANKCALL); /* CADR AVFLAGP */
#else
    tc_to(P78, AVFLAGP);
#endif
}

/* then either "SET UPDATFLG, TRACKFLG" (P20FLGON). */
void p78_active(void)
{
#if LUMINARY099
    tc_to(P78 + 2u, BANKCALL); /* CADR P20FLGON */
#else
    tc_to(P78 + 1u, P20FLGON);
#endif
}

/* "DISPLAY TIG", verb 06 noun 33 (VNDSPLY; the LM's NN 2 first, which the
 * CM's interpretive code sets), */
void p38_tig(void)
{
#if LUMINARY099
    set_A(DECTWO);
    ts(ADDRESS_OF(NN));
    set_A(V06N33SR);
    tc_at(P78 + 7u);
#else
    set_A(V06N33SR);
    tc_at(P78 + 3u);
#endif
    vndsply();
}

/* then "DISPLAY CENTANG", verb 06 noun 55, flashed (BANKCALL, CADR
 * GOFLASHR). */
void p38_centang(void)
{
    set_A(V06N55SR);
    tc_to(CENTANG_DISPLAY + 1u, BANKCALL);
}

/* GOFLASHR's returns: "TERMINATE" (GOTOPOOH), */
void p38_centang_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* "PROCEED" (TCF +5), */
void p38_centang_proceed(void)
{
    p38_option_code();
}

/* "RECYCLE" (TCF -5), */
void p38_centang_recycle(void)
{
    p38_centang();
}

/* and "IMMEDIATE RETURN - BLANK R1, R2" (BLANKET with 3), after which the
 * job ends (TCF ENDOFJOB). */
void p38_centang_blank(void)
{
    set_A(THREE);
    tc_to(CENTANG_DISPLAY + 7u, BLANKET);
}

void p38_centang_end(void)
{
    continue_at(ENDOFJOB);
}

/* OPTION1 5 and OPTION2 1: "OPTION CODE IS SET TO 1", */
static void p38_option_code(void)
{
    set_A(FIVE);
    ts(ADDRESS_OF(OPTION1));
    set_A(ONE);
    ts(ADDRESS_OF(OPTION2)); /* "OPTION CODE IS SET TO 1" */
    p38_option();
}

/* and "DISPLAY OPTION CODE", verb 04 noun 06, flashed. */
static void p38_option(void)
{
    set_A(V04N06SR);
    tc_to(OPTION_DISPLAY + 1u, BANKCALL); /* CADR GOFLASHR */
}

/* Its returns: terminate, */
void p38_option_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* proceed, to the interpreter for P38_CODE (TCF +5; TC INTPRET), */
void p38_option_proceed(void)
{
    tc_to(OPTION_DISPLAY + 011u, INTPRET);
}

/* recycle, */
void p38_option_recycle(void)
{
    p38_option();
}

/* and "IMMEDIATE RETURN - BLANK R3". */
void p38_option_blank(void)
{
    set_A(BIT3);
    tc_to(OPTION_DISPLAY + 7u, BLANKET);
}

void p38_option_end(void)
{
    continue_at(ENDOFJOB);
}

/* The CM's NN 2 (SSP). OPTION2 from the display, "1 = SOI, 2 = SOR": SR1
 * leaves the SOR's 1 high-order zero and the SOI's 0 all zero (BHIZ). The
 * SOR's TINT is saved as TINTSOI ("STORE FOR SOR PHASE") and OPTNSW cleared
 * ("OPTNSW; ON = SOI, OFF = SOR"), on at JUNCTN1. OPTN1, "SOI": OPTNSW set
 * and UPDATFLG clear while PREC/TT works out TF, the passive vehicle's
 * time over CENTANG; then UPDATFLG set again and "TI = TIG + TF" (TINT;
 * the CM's DSPTEM1 too, "FOR DISPLAY"). */
static const word p38_words[] = {
#if !LUMINARY099
    ORDER(SSP),
    ADDRESS_PLUS_ONE(NN),
    NUMBER(2u),
#endif
    ORDERS(SLOAD, SR1),
    ADDRESS_PLUS_ONE(OPTION2),
    ORDERS(BHIZ, DLOAD),
    ADDRESS(OPTN1),
    ADDRESS_PLUS_ONE(TINT),
    STORE(TINTSOI), /* "STORE FOR SOR PHASE" */
    ORDER(CLRGO),
    SWITCH(CLRGO, OPTNSW), /* "OPTNSW; ON = SOI, OFF = SOR" */
    ADDRESS(JUNCTN1),
    /* OPTN1 */ ORDERS(SET, CLEAR), /* "SOI" */
    SWITCH(SET, OPTNSW),
    SWITCH(CLEAR, UPDATFLG),
    ORDER(CALL),
    ADDRESS(PREC_TT),
#if LUMINARY099
    ORDERS(DAD, SET),
    ADDRESS_PLUS_ONE(TIG),
    SWITCH(SET, UPDATFLG),
    STORE(TINT), /* "TI = TIG + TF" */
#else
    ORDERS(SET, DAD),
    SWITCH(SET, UPDATFLG),
    ADDRESS_PLUS_ONE(TIG),
    STORE(TINT),    /* "TI = TIG + TF" */
    STORE(DSPTEM1), /* "FOR DISPLAY" */
#endif
    ORDER(EXIT),
};

/* After the SOI's EXIT: "DISPLAY DELTA R", verb 06 noun 57, flashed. */
void p38_deltar(void)
{
    set_A(V06N57SR);
    tc_to(DELTAR_DISPLAY + 1u, BANKCALL); /* CADR GOFLASHR */
}

/* Its returns: terminate, */
void p38_deltar_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* proceed (TCF +5; the LM's TINT to DSPTEM1 "FOR DISPLAY" first), */
void p38_deltar_proceed(void)
{
#if LUMINARY099
    dca(ADDRESS_OF(TINT)); /* EXTEND; DCA TINT */
    dxch(ADDRESS_OF(DSPTEM1));
#endif
    p38_intercept();
}

/* recycle, */
void p38_deltar_recycle(void)
{
    p38_deltar();
}

/* and "IMMEDIATE RETURN - BLANK R2, R3". */
void p38_deltar_blank(void)
{
    set_A(SIX);
    tc_to(DELTAR_DISPLAY + 7u, BLANKET);
}

void p38_deltar_end(void)
{
    continue_at(ENDOFJOB);
}

/* "DISPLAY TIME OF INTERCEPT", verb 06 noun 34 (VNDSPLY), */
static void p38_intercept(void)
{
    set_A(V06N34SR);
#if LUMINARY099
    tc_at(DELTAR_DISPLAY + 015u);
#else
    tc_at(DELTAR_DISPLAY + 012u);
#endif
    vndsply();
}

/* and to the interpreter, for JUNCTN1_CODE. */
void p38_intercept_done(void)
{
#if LUMINARY099
    tc_to(DELTAR_DISPLAY + 016u, INTPRET);
#else
    tc_to(DELTAR_DISPLAY + 013u, INTPRET);
#endif
}

/* ---- JUNCTN1 and MAINRTNE -- LM pp. 725-727, CM pp. 527-529 ---- */

/* JUNCTN1: P39/79SW clear (P38 or P78), and SELECTMU ("SELECT MU, CLEAR
 * FINALFLG, GO TO VN1645"). RECYCLE: TF again (PREC/TT); for the SOI
 * (OPTNSW set), the passive vehicle at TINT ("PRECISION UPDATE PASSIVE
 * VEHICLE TO INTERCEPT TIME", INTRPVP), and "DELTA T = (RP) DELTA R / (VP
 * X RP)" to DELTTIME, from "RP/(RP)" (RATT) and VATT, the cross product
 * normalized (NORM, its shift in X1) and the quotient shifted back (SL*
 * by X1). OPTN2, the SOR: "TI = TI + TF" from TINTSOI. JUNCTN2: TARGTIME,
 * "TT = TI - DELTA T".
 *
 * MAINRTNE: the passive vehicle at TARGTIME ("PRECISION UPDATE PASSIVE
 * VEHICLE TO TARGET TIME", INTRPVP), INTIME TIG, and from RATT,
 * S3435.25, which returns to TEST3979 (SUBEXIT). TEST3979: in P39 or P79
 * (P39/79SW set), with FINALFLG set to P39P79; else with it clear,
 * UPDATFLG set first, to the same EXIT; and in P38 or P78 to MAINRTN1. */
static const word junctn1_words[] = {
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, P39_79SW),
    ADDRESS(SELECTMU), /* "SELECT MU, CLEAR FINALFLG, GO TO VN1645" */
    /* RECYCLE */ ORDER(CALL),
    ADDRESS(PREC_TT),
    ORDERS(BOFF, DLOAD),
    SWITCH(BOF, OPTNSW),
    ADDRESS(OPTN2),
    ADDRESS_PLUS_ONE(TINT),
    STCALL(TDEC1),    /* "PRECISION UPDATE PASSIVE VEHICLE TO" */
    ADDRESS(INTRPVP), /* "INTERCEPT TIME" */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RATT), /* "RP/(RP)" */
    ORDERS(PDVL, VXV),
    ADDRESS_PLUS_ONE(VATT),
    ORDERS(ABVAL, NORM), /* "(VP X RP/(RP))" */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, DDV),
    ADDRESS_PLUS_ONE(DELTAR),
    ORDER(SL_STAR), /* "DELTA R / (VP X RP/RP)" */
    SL_X1(-7),
    STCALL(DELTTIME), /* "DELTA T = (RP) DELTA R / (VP X RP)" */
    ADDRESS(JUNCTN2),
    /* OPTN2 */ ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(TINTSOI),
    ADDRESS_PLUS_ONE(T),
    STORE(TINT), /* "TI = TI + TF" */
    /* JUNCTN2 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TINT),
    ADDRESS_PLUS_ONE(DELTTIME),
    STORE(TARGTIME),              /* "TT = TI - DELTA T" */
    /* MAINRTNE */ STCALL(TDEC1), /* "PRECISION UPDATE PASSIVE VEHICLE TO" */
    ADDRESS(INTRPVP),             /* "TARGET TIME" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TIG),
    STORE(INTIME),
    ORDERS(SSP, VLOAD),
    ADDRESS_PLUS_ONE(SUBEXIT),
    ADDRESS(TEST3979),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(CALL),
    ADDRESS(S3435_25),
    /* TEST3979 */ ORDERS(BOFF, BON),
    SWITCH(BOF, P39_79SW),
    ADDRESS(MAINRTN1),
    SWITCH(BON, FINALFLG),
    ADDRESS(P39P79),
    ORDER(SET),
    SWITCH(SET, UPDATFLG),
    /* P39P79 */ ORDER(EXIT),
};

/* P39P79's EXIT: "FOR P39 AND P79" (TC DSPLY81). */
void p39p79_exit(void)
{
    tc_at(P39P79 + 1u);
    dsply81();
}

/* MAINRTN1, P38 and P78: DELVTPI, "DELTA V", from DELVEET3; DELVTPF, "DELTA
 * V (FINAL) = V'T - VT" (VPASS4 less VTPRIME); POSTTPI, the perigee
 * altitude ("GET PERIGEE ALTITUDE", PERIAPO1 from RACT3 and VIPRIME, then
 * SHIFTR1). With FINALFLG set to DSPLY58; else with it clear, UPDATFLG set
 * first. */
static const word mainrtn1_words[] = {
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(DELVEET3),
    STOVL(DELVTPI), /* "DELTA V" */
    ADDRESS_PLUS_ONE(VPASS4),
    ORDERS(VSU, ABVAL),
    ADDRESS_PLUS_ONE(VTPRIME),
    STOVL(DELVTPF), /* "DELTA V (FINAL) = V'T - VT" */
    ADDRESS_PLUS_ONE(RACT3),
    ORDERS(PDVL, CALL),
    ADDRESS_PLUS_ONE(VIPRIME),
    ADDRESS(PERIAPO1), /* "GET PERIGEE ALTITUDE" */
    ORDER(CALL),
    ADDRESS(SHIFTR1),
    STORE(POSTTPI),
    ORDERS(BON, SET),
    SWITCH(BON, FINALFLG),
    ADDRESS(DSPLY58),
    SWITCH(SET, UPDATFLG),
    /* DSPLY58 */ ORDER(EXIT),
};

/* DSPLY58's EXIT: "DISPLAY HP, DELTA V, DELTA V (FINAL)", verb 06 noun 58
 * (VNDSPLY), */
void dsply58_exit(void)
{
    set_A(V06N58SR);
    tc_at(DSPLY58 + 2u);
    vndsply();
}

/* then DSPLY81: "DISPLAY DELTA V (LV)", verb 06 noun 81, */
void dsply81(void)
{
    set_A(V06N81SR);
    tc_at(DSPLY81 + 1u);
    vndsply();
}

/* and to the interpreter, for DSPLY81_CODE. */
void dsply81_done(void)
{
    tc_to(DSPLY81 + 2u, INTPRET);
}

/* XDELVFLG clear, and DELVEET3 to DELVSIN for VN1645 ("DISPLAY TRKMKCNT,
 * TTOGO, +MGA"); then P39 and P79 again from P39/P79B, P38 and P78 from
 * RECYCLE. */
static const word dsply81_words[] = {
    ORDERS(CLEAR, VLOAD), SWITCH(CLEAR, XDELVFLG), ADDRESS_PLUS_ONE(DELVEET3), STCALL(DELVSIN),
    ADDRESS(VN1645), /* "DISPLAY TRKMKCNT, TTOGO, +MGA" */
    ORDERS(BON, GOTO),    SWITCH(BON, P39_79SW),   ADDRESS(P39_P79B),          ADDRESS(RECYCLE),
};

/* ---- P39 and P79 -- LM pp. 728-729, CM p. 530 ---- */

/* P39: "THIS VEHICLE ACTIVE" (AVFLAGA), */
void p39(void)
{
#if LUMINARY099
    tc_to(P39, BANKCALL); /* CADR AVFLAGA */
#else
    tc_to(P39, AVFLAGA);
#endif
}

/* and its time to prepare, ATIGINC (TC P39/P79A); */
void p39_active(void)
{
    dca(ADDRESS_OF(ATIGINC)); /* EXTEND; DCA ATIGINC */
#if LUMINARY099
    tc_at(P39 + 4u);
#else
    tc_at(P39 + 3u);
#endif
    p39_p79a();
}

/* P79: "OTHER VEHICLE ACTIVE" (AVFLAGP), */
void p79(void)
{
#if LUMINARY099
    tc_to(P79, BANKCALL); /* CADR AVFLAGP */
#else
    tc_to(P79, AVFLAGP);
#endif
}

/* and PTIGINC. */
void p79_active(void)
{
    dca(ADDRESS_OF(PTIGINC)); /* EXTEND; DCA PTIGINC */
    p39_p79a();
}

/* P39/P79A: KT, the "TIME TO PREPARE FOR BURN", then "SET UPDATFLG,
 * TRACKFLG" (P20FLGON), */
static void p39_p79a(void)
{
    dxch(ADDRESS_OF(KT)); /* "TIME TO PREPARE FOR BURN" */
#if LUMINARY099
    tc_to(P39_P79A + 1u, BANKCALL); /* CADR P20FLGON */
#else
    tc_to(P39_P79A + 1u, P20FLGON);
#endif
}

/* and to the interpreter, for P39_CODE. */
void p39_p79a_done(void)
{
#if LUMINARY099
    tc_to(P39_P79A + 3u, INTPRET);
#else
    tc_to(P39_P79A + 2u, INTPRET);
#endif
}

/* P39/79SW set and SELECTMU. P39/P79B: "TIG = T (PRESENT) + PREPARATION
 * TIME" (LOADTIME plus KT), both vehicles there ("PRECISION UPDATE ACTIVE
 * AND PASSIVE VEHICLES TO TIG", PRECSET), the normal ("GET UNIT NORMAL",
 * S34/35.1), and MAINRTNE from the P38's TARGTIME.
 *
 * PREC/TT, a subroutine (its return in RTRN): both vehicles at TIG, and
 * TIMETHET's time for the passive vehicle over CENTANG ("GET TRANSFER TIME
 * BASED ON CENTANG OF PASSIVE VEHICLE") from RPASS3 and VPASS3, shifted by
 * X2 as the precision update left it, with CENTANG's cosine (CSTH) and sine
 * (SNTH) and RVSW set; then the normal, and back with T.
 *
 * INTRPVP: "PRECISION UPDATE PASSIVE VEHICLE TO TDEC1", the CSM's
 * (CSMPREC) where this vehicle is the active one (AVFLAG set), else the
 * LM's (OTHERV, LEMPREC), and back through RTRN. */
static const word p39_words[] = {
    ORDERS(SET, CALL),
    SWITCH(SET, P39_79SW),
    ADDRESS(SELECTMU), /* "SELECT MU, CLEAR FINALFLG, GO TO VN1645" */
    /* P39/P79B */ ORDERS(RTB, DAD),
    ADDRESS(LOADTIME),
    ADDRESS_PLUS_ONE(KT),
    STORE(TIG),       /* "TIG = T (PRESENT) + PREPARATION TIME" */
    STCALL(TDEC1),    /* "PRECISION UPDATE ACTIVE AND PASSIVE" */
    ADDRESS(PRECSET), /* "VEHICLES TO TIG" */
    ORDER(CALL),
    ADDRESS(S34_35_1), /* "GET UNIT NORMAL" */
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(TARGTIME),
    ADDRESS(MAINRTNE), /* "CALCULATE DELTA V AND DELTA V (LV)" */
    /* PREC/TT */ ORDERS(STQ, DLOAD),
    ADDRESS(RTRN),
    ADDRESS_PLUS_ONE(TIG),
    STCALL(TDEC1),    /* "PRECISION UPDATE ACTIVE AND PASSIVE" */
    ADDRESS(PRECSET), /* "VEHICLES TO TIG" */
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(RPASS3),
    SR_X2(0),
    STODL(RVEC),
    ADDRESS_PLUS_ONE(CENTANG),
    ORDERS(PUSH, COS),
    STODL(CSTH),
    ORDERS(SIN, SET),
    SWITCH(SET, RVSW),
    STOVL(SNTH),
    ADDRESS_PLUS_ONE(VPASS3),
    ORDER(VSR_STAR),
    SR_X2(0),
    STCALL(VVEC),      /* "GET TRANSFER TIME BASED ON CENTANG OF" */
    ADDRESS(TIMETHET), /* "PASSIVE VEHICLE" */
    ORDER(CALL),
    ADDRESS(S34_35_1), /* "GET UNIT NORMAL" */
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(T),
    ADDRESS(RTRN),
    /* INTRPVP */ ORDERS(STQ, BOFF), /* "PRECISION UPDATE PASSIVE VEHICLE TO" */
    ADDRESS(RTRN),                   /* "TDEC1" */
    SWITCH(BOF, AVFLAG),
    ADDRESS(OTHERV),
    ORDER(CALL),
    ADDRESS(CSMPREC),
    ORDER(GOTO),
    ADDRESS(RTRN),
    /* OTHERV */ ORDER(CALL),
    ADDRESS(LEMPREC),
    ORDER(GOTO),
    ADDRESS(RTRN),
};

/* ---- VNDSPLY -- LM p. 730, CM p. 532 ---- */

/* VNDSPLY, "FLASH DISPLAY": the return to RTRN (EXTEND; QXCH RTRN), the
 * verb and noun in A to VERBNOUN, */
static void vndsply_flash(void);
static void vndsply(void)
{
    qxch(ADDRESS_OF(RTRN));
    ts(ADDRESS_OF(VERBNOUN));
    vndsply_flash();
}

/* flashed (BANKCALL, CADR GOFLASH). */
static void vndsply_flash(void)
{
    ca(ADDRESS_OF(VERBNOUN));
    tc_to(VNDSPLY + 4u, BANKCALL);
}

/* The word in RTRN, a TC to the word after the caller's TC VNDSPLY, runs
 * as an instruction: Q gets the word after RTRN. (Anything else there,
 * which no program leaves, is not modelled.) */
static void tc_rtrn_word(void)
{
    word instruction = read_word(ADDRESS_OF(RTRN));

    if ((instruction & 070000u) || instruction < 010u) not_modelled();
    tc_at(ADDRESS_OF(RTRN));
    continue_at(instruction);
}

/* GOFLASH's returns: "TERMINATE" (GOTOPOOH), */
void vndsply_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* "PROCEED" (TC RTRN): the caller's return, */
void vndsply_proceed(void)
{
    tc_at(VNDSPLY + 7u);
#if LUMINARY099
    tc_rtrn_word();
#else
    continue_at(ADDRESS_OF(RTRN));
#endif
}

#if !LUMINARY099
/* (the CM's RTRN, where the TC left Z: its word), */
void rtrn(void)
{
    tc_rtrn_word();
}
#endif

/* and "RECYCLE" (TCF -5): flashed again. */
void vndsply_recycle(void)
{
    vndsply_flash();
}

const struct interpretive_code P38_CODE = {STABLE_ORBIT_BANK, OPTION_DISPLAY + 012u,
                                           COUNT(p38_words), p38_words};
const struct interpretive_code JUNCTN1_CODE = {STABLE_ORBIT_BANK, IN_BANK(JUNCTN1),
                                               COUNT(junctn1_words), junctn1_words};
const struct interpretive_code MAINRTN1_CODE = {STABLE_ORBIT_BANK, IN_BANK(MAINRTN1),
                                                COUNT(mainrtn1_words), mainrtn1_words};
const struct interpretive_code DSPLY81_CODE = {STABLE_ORBIT_BANK, IN_BANK(DSPLY81) + 3u,
                                               COUNT(dsply81_words), dsply81_words};
#if LUMINARY099
const struct interpretive_code P39_CODE = {STABLE_ORBIT_BANK, IN_BANK(P39_P79A) + 4u,
                                           COUNT(p39_words), p39_words};
#else
const struct interpretive_code P39_CODE = {STABLE_ORBIT_BANK, IN_BANK(P39_P79A) + 3u,
                                           COUNT(p39_words), p39_words};
#endif
