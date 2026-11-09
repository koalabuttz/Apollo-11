/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, ITERATOR (LM pp. 1182-1183, CM pp. 1285-1286):
 * the search for the root that LAMBERT and TIME-RADIUS share, in the
 * listing's notation (src/interpreter/notation.h), which the readable
 * interpreter runs.
 *
 * Given the dependent variable now (DEP) and last time (DEPREV), the
 * change still wanted in it (DELDEP), the last step in the independent
 * variable INDEP (DELINDEP) and its bounds MIN and MAX, it works out the
 * next step, to DELINDEP: by the secant through the last two points, kept
 * between the bounds, which close in on the root. Its words are in the work area of
 * the job (MIN 8D, DELINDEP 12D, MAX 14D, DEP 30D, TWEEKIT 40D). */
#include "conics.h"
#include "../agc/names.h"

static const word iterator_words[] = {
    /* ITERATOR -- LM p. 1182 (CM p. 1285): the step. The first time
     * (SLOPESW, which BONCLR clears) there is no slope yet: FIRSTIME. After
     * it, the secant: the last step DELINDEP times DELDEP over the change
     * it made, DEP - DEPREV, each normalized (the counts in X1 and X2) and
     * shifted back by their difference (in X1). With ORDERSW (a second
     * order search) the step takes DELDEP's sign whatever the slope's. */
    /* ITERATOR */ ORDERS(BONCLR, DLOAD),
    SWITCH(BONCLR, SLOPESW),
    ADDRESS(FIRSTIME),
    ADDRESS_PLUS_ONE(DEP),
    ORDERS(DSU, NORM),
    ADDRESS_PLUS_ONE(DEPREV),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE(DELINDEP),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(XSU_1, DMP),
    ADDRESS(X2),
    ADDRESS_PLUS_ONE(DELDEP),
    ORDERS(SLR_STAR, DDV), /* "PL UP 2" */
    SLR_X1(1),
    ORDERS(SR1, BOFF),
    SWITCH(BOFF, ORDERSW),
    ADDRESS(SGNCHECK),
    ORDERS(ABS, SIGN),        /* "IN CASE 2ND DERIV. CHANGED SIGN, MUST" */
    ADDRESS_PLUS_ONE(DELDEP), /* "DISREGARD IT TO FIND MIN." */
                              /* SGNCHECK: the trial step on the push-down list, and the bound on
     * the side it moves away from moved in to INDEP, but in a second order
     * search. MINCHECK: a step that would take INDEP past MIN is cut to 9/10
     * of the way to it (MODNGDEL); POSDEL and MAXCHECK are the same for a
     * positive step and MAX (MODPSDEL). DELOK: the step stands. NEWDEL: it
     * becomes DELINDEP, and RVQ returns to the caller, who adds it to
     * INDEP. */
    /* SGNCHECK */ ORDERS(PUSH, BPL), /* "TRIAL DELINDEP" */
    ADDRESS(POSDEL),
    ORDERS(DLOAD, BON),
    ADDRESS_PLUS_ONE(INDEP),
    SWITCH(BON, ORDERSW),
    ADDRESS(MINCHECK),
    STORE(MAX), /* "IF NOT 2ND ORDER, CAN MOVE MAX BOUND IN." */
    /* MINCHECK */ ORDERS(BDSU, DSU),
    ADDRESS_PLUS_ONE(MIN),
    ORDERS(BOV, BPL),
    ADDRESS(MODNGDEL),
    ADDRESS(MODNGDEL),
    ORDER(GOTO),
    ADDRESS(DELOK),
    /* MODNGDEL */ ORDERS(DLOAD, DSU), /* "TRIAL DELINDEP WOULD EXCEED MIN BOUND" */
    ADDRESS_PLUS_ONE(MIN),
    ADDRESS_PLUS_ONE(INDEP),
    ORDERS(DMP, GOTO),
    ADDRESS_PLUS_ONE(DP9_10),
    ADDRESS(NEWDEL),
    /* FIRSTIME: TWEEKIT, a fraction, of the span from MIN to MAX, with
     * DELDEP's sign. */
    /* FIRSTIME */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(MIN),
    ADDRESS_PLUS_ONE(TWEEKIT), /* "DLOAD TWEEKIT(40D) SENSITIVE TO CHANGE." */
    ORDERS(PDDL, DMP),         /* "S2(41D) SHOULDNT CONTAIN HI ORDER ONES" */
    ADDRESS_PLUS_ONE(MAX),
    ADDRESS_PLUS_ONE(TWEEKIT),
    ORDER(DSU),
    ORDERS(SIGN, GOTO),
    ADDRESS_PLUS_ONE(DELDEP),
    ADDRESS(SGNCHECK),
    /* POSDEL */ ORDERS(DLOAD, BON),
    ADDRESS_PLUS_ONE(INDEP),
    SWITCH(BON, ORDERSW),
    ADDRESS(MAXCHECK),
    STORE(MIN), /* "IF NOT 2ND ORDER, CAN MOVE MIN BOUND IN." */
    /* MAXCHECK */ ORDERS(BDSU, DSU),
    ADDRESS_PLUS_ONE(MAX),
    ORDERS(BOV, BMN),
    ADDRESS(MODPSDEL),
    ADDRESS(MODPSDEL),
    /* DELOK */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(0),
    /* NEWDEL */ STORE(DELINDEP),
    ORDER(RVQ),
    /* MODPSDEL */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(MAX),
    ADDRESS_PLUS_ONE(INDEP),
    ORDERS(DMP, GOTO),
    ADDRESS_PLUS_ONE(DP9_10),
    ADDRESS(NEWDEL),
};

const struct interpretive_code ITERATOR_CODE = {I_ITERATOR >> 10, K_ITERATOR, COUNT(iterator_words),
                                                iterator_words};
