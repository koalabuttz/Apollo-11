/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 2 (Luminary099 pp. 773-776, bank 27): S40.13,
 * "TIMEBURN", and S40.9, their basic code and their interpretive code in
 * the listing's notation. The faithful C is c64-port/src/lm_p40_p47.c.
 *
 * S40.13, a job (FINDVAC) of P40, P41 and P42, decides "WHETHER A GIVEN
 * COMBINATION OF VELOCITY TO", "BE GAINED AND ENGINE CHOICE RESULT IN A
 * BURN TIME", "SUFFICIENT TO ALLOW STEERING AT THE VEHICLE DURING THE",
 * "BURN"; for a short burn it sets IMPULSW and works out TGO, the time to
 * burn. NOTHROTL set inhibits throttling. S40.9, a job UPDATEVG starts,
 * calls Lambert's guess (HAVEGUES) for a burn to an aim point.
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC. */
#include "p40_p47.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- S40.13 -- pp. 773-775 ---- */

/* S40.13: to the interpreter, for S40_13_CODE. */
void s40_13(void)
{
    tc_at(S40_13);
    intpret();
}

/* S40_13_CODE: "ASSUME NO STEERING UNTIL FOUND OTHERWISE" (IMPULSW
 * clear). In 00D the magnitude of VGTIG less what "4 SECS OF 2 JET
 * ULLAGE" give (4SEC(17), FRCS2, WEIGHT/G). For the descent engine
 * (APSFLAG clear) on at S40.13D. For the ascent engine NOTHROTL set: with
 * the delta V less than K1VAL over the mass, "TGO LESS THAN 100 CS", on
 * at S40.131; else TEMP1 (02D) from the mass flow (MDOT) over 3.5
 * seconds, TEMP2 (04D) from the thrust over 5 seconds, and with TEMP1 at
 * least TEMP2 on at S40.13D; else the time from the two (5SECS), plus a
 * second (1SEC2D), to S40.132. S40.131: the time from the mass (K2VAL, K3VAL).
 * S40.132: IMPULSW set, to basic code. */
static const word s40_13_words[] = {
    ORDERS(SETPD, CLEAR),
    ADDRESS_PLUS_ONE_D(0),
    SWITCH(CLEAR, IMPULSW), /* "ASSUME NO STEERING UNTIL FOUND OTHERWISE" */
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(VGTIG),   /* "VELOCITY TO BE GAINED AT +7" */
    ORDERS(PDDL, DMP),         /* "00D = MAG OF VGTIG AT +7" */
    ADDRESS_PLUS_ONE(4SEC_17), /* "CORRECT VG FOR 4 SECS OF 2 JET ULLAGE" */
    ADDRESS_PLUS_ONE(FRCS2),
    ORDERS(DDV, SL1), /* "SCALE" */
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(BDSU, PUSH),
    ORDERS(BOF, SET),
    SWITCH(BOF, APSFLAG),
    ADDRESS(S40_13D), /* "FOR DPS ENGINE" */
    SWITCH(SET, NOTHROTL),
    ORDERS(DLOAD, DDV),      /* "00D = MAG OF VGTIG CORRECTED" */
    ADDRESS_PLUS_ONE(K1VAL), /* "M.NEWTONS-CS AT +24" */
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(S40_131),  /* "TGO LESS THAN 100 CS" */
    ORDERS(PDDL, DMP), /* "02D = TEMP1 AT +7" */
    ADDRESS_PLUS_ONE(MDOT),
    ADDRESS_PLUS_ONE(3_5SEC), /* "350 CS AT +14" */
    ORDERS(BDSU, PDDL),
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ADDRESS_PLUS_ONE(F),
    ORDERS(DMP, SR2), /* "SCALE" */
    ADDRESS_PLUS_ONE(5SECS),
    ORDERS(DDV, PUSH), /* "04D = TEMP2" */
    ORDERS(BDSU, BPL),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS(S40_13D),
    ORDERS(DLOAD, BDDV),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(5SECS),
    ADDRESS_PLUS_ONE(1SEC2D), /* "100 CS AT +14" */
    ORDER(GOTO),
    ADDRESS(S40_132),
    /* S40.131 */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(SR1, PUSH),
    ORDERS(DAD, DDV),
    ADDRESS_PLUS_ONE(K2VAL), /* "M.NEWTON CS AT +24" */
    ADDRESS_PLUS_ONE(K3VAL), /* "M.NEWTON CS AT +10" */
    /* S40.132 */
    ORDERS(SET, EXIT),
    SWITCH(SET, IMPULSW),
};

/* S40.132*: the time to burn in MPAC, its two words made to agree in sign
 * (TPAGREE), */
void s40_132_star(void)
{
    tc_to(S40_132_STAR, TPAGREE);
}

/* to TGO (CA MPAC, XCH L, CA ZERO, DXCH TGO: the low word, as a DP
 * number); on at S40.134 (TCF). */
void s40_132_star_after_tpagree(void)
{
    ca(ADDRESS_OF(MPAC));
    xch(L_ADDRESS);
    set_A(ZERO);
    dxch(ADDRESS_OF(TGO));
    s40_134();
}

/* S40_13D_CODE: "FOR DPS ENGINE": the delta V times the mass; for the
 * ascent engine (APSFLAG set) on at APSTGO. Over S40.136, NOTHROTL
 * clear; on overflow at S40.130V. S40.137:
 * with 6 seconds or more, on at S40.138; less, the time (6SEC added
 * back) to S40.132. S40.133: to basic code. */
static const word s40_13d_words[] = {
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(PUSH, BON),
    SWITCH(BON, APSFLAG),
    ADDRESS(APSTGO),
    ORDERS(DDV, CLEAR),
    ADDRESS_PLUS_ONE(S40_136),
    SWITCH(CLEAR, NOTHROTL),
    ORDERS(BOV, PUSH),
    ADDRESS(S40_130V),
    /* S40.137 */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(6SEC), /* "600.0 CS AT +14" */
    ADDRESS(S40_138),
    ORDERS(DAD, GOTO),
    ADDRESS_PLUS_ONE(6SEC),
    ADDRESS(S40_132),
    /* S40.133 */
    ORDER(EXIT),
};

/* S40.134 (S40.133 +1, after the EXIT): the restart group 3 at its phase
 * (PHASCHNG, OCT 00003), */
void s40_134(void)
{
    tc_to(S40_134, PHASCHNG);
}

/* and the job ends (TC ENDOFJOB). */
void s40_134_after_phaschng(void)
{
    tc_to(S40_134 + 2u, ENDOFJOB);
}

/* S40_130V_CODE: "RECOMPUTED TGO IN TIMER UNITS" (S40.136 shifted left
 * 10, S40.136_), into TGO; to basic code. */
static const word s40_130v_words[] = {
    ORDERS(DLOAD, SR4),                             /* "RECOMPUTED TGO IN TIMER UNITS" */
    ORDER(DDV),         ADDRESS_PLUS_ONE(S40_136_), /* "S40.136 SHIFTED LEFT 10" */
    STORE(TGO),         ORDER(EXIT),
};

/* S40.130V +5, after the EXIT: "REJOIN COMMON CODING FOR RESTART PROTECT"
 * (TCF S40.134). */
void s40_130v_after_exit(void)
{
    s40_134();
}

/* S40_138_CODE (S40.138): with less than 89 seconds more (89SECS),
 * NOTHROTL set.
 * STORETGO: "LOAD TGO AT 2(14)"; to basic code. APSTGO: the time over the
 * ascent engine's thrust (FAPS), on at STORETGO +1. */
static const word s40_138_words[] = {
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(89SECS),
    ADDRESS(STORETGO),
    ORDER(SET),
    SWITCH(SET, NOTHROTL),
    /* STORETGO */
    ORDER(DLOAD), /* "LOAD TGO AT 2(14)" */
    ORDER(EXIT),
};

/* STORETGO +2, after the EXIT: on at S40.132* (TCF). */
void storetgo_after_exit(void)
{
    s40_132_star();
}

static const word apstgo_words[] = {
    ORDERS(DDV, SL2),
    ADDRESS_PLUS_ONE(FAPS),
    ORDER(GOTO),
    ADDRESS_AT(STORETGO, 1),
};

/* ---- S40.9 -- p. 776 ---- */

/* S40.9: to the interpreter, for S40_9_CODE. */
void s40_9(void)
{
    tc_at(S40_9);
    intpret();
}

/* S40_9_CODE: "SET AVFLAG FOR LEM ACTIVE", no iterations (HI6ZEROS) and
 * the cone, "EPSILON4 = 10 OR 45 DEGREES." (EPS1, or EPS2 more with
 * NORMSW set); EPSSMALL: HAVEGUES. ENDS40.9: to basic code. */
static const word s40_9_words[] = {
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SET, DLOAD),
    SWITCH(SET, AVFLAG), /* "SET AVFLAG FOR LEM ACTIVE" */
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDER(PDDL),
    ADDRESS_PLUS_ONE(EPS1),
    ORDERS(BOF, DAD), /* "EPSILON4 = 10 OR 45 DEGREES." */
    SWITCH(BOF, NORMSW),
    ADDRESS(EPSSMALL),
    ADDRESS_PLUS_ONE(EPS2),
    /* EPSSMALL */
    ORDERS(PUSH, CALL),
    ADDRESS(HAVEGUES),
    /* ENDS40.9 */
    ORDER(EXIT),
};

/* ENDS40.9 +1, after the EXIT: the restart group 2 at its phase
 * (PHASCHNG, OCT 2), */
void ends40_9_after_exit(void)
{
    tc_to(ENDS40_9 + 1u, PHASCHNG);
}

/* and the job ends (TCF ENDOFJOB). */
void ends40_9_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* ---- The interpretive code ---- */

const struct interpretive_code S40_13_CODE = {027u, S40_13 + 1u, COUNT(s40_13_words), s40_13_words};
const struct interpretive_code S40_13D_CODE = {027u, S40_132_STAR + 6u, COUNT(s40_13d_words),
                                               s40_13d_words};
const struct interpretive_code S40_130V_CODE = {027u, S40_130V, COUNT(s40_130v_words),
                                                s40_130v_words};
const struct interpretive_code S40_138_CODE = {027u, S40_130V + 6u, COUNT(s40_138_words),
                                               s40_138_words};
const struct interpretive_code APSTGO_CODE = {027u, STORETGO + 3u, COUNT(apstgo_words),
                                              apstgo_words};
const struct interpretive_code S40_9_CODE = {027u, S40_9 + 1u, COUNT(s40_9_words), s40_9_words};

#endif
