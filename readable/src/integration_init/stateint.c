/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, STATEINT to NOINT (LM pp. 1209-1210, CM
 * pp. 1313-1314): the periodic integration of the state vectors in P00.
 * Every ten minutes (600SECS) a task, STATEINT, starts a job, STATINT1,
 * which brings both vectors up to the present through STATEUP (in
 * FRESH_START_AND_RESTART). Each step is a phase of restart group 2, so a
 * restart picks the cycle up again. QUITFLAG stops it ("KILL INTEGRATION
 * UNTIL NEXT P00"). The interpretive words are in the listing's notation
 * (src/interpreter/notation.h); the basic code is C. */
#include "integration_init.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

/* TC at "at" to a routine outside the file (PHASCHNG, FINDVAC, LONGCALL,
 * DOWNFLAG, ENDOFJOB, TASKOVER), which reads the words after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* STATEINT -- LM p. 1209 (CM p. 1313): the task. Group 2's phase (OCT
 * 00052), then a job with a VAC area at PRIO5 (FINDVAC) to run STATINT1,
 * and the task is over (TASKOVER). */
void stateint(void)
{
    tc_to(STATEINT, PHASCHNG); /* OCT 00052 */
}

void stateint_after_phaschng(void)
{
    set_A(PRIO5);                          /* CAF PRIO5 */
    tc_to((word)(STATEINT + 3u), FINDVAC); /* 2CADR STATINT1 */
}

void stateint_after_findvac(void)
{
    tc_to((word)(STATEINT + 6u), TASKOVER);
}

/* STATINT1: the job, into the interpreter. */
void statint1(void)
{
    tc_at(STATINT1); /* TC INTPRET */
    intpret();
}

static const word statint1_words[] = {
    /* STATINT1 +1 -- LM p. 1209 (CM p. 1313): nothing with QUITFLAG set
     * (NOINT); otherwise the present time (LOADTIME) as the time to
     * integrate to (TDEC1), the integration taken (INTSTALL), and STATEUP
     * with its flags (SETIFLGS) and NODOFLAG set (no V37 meanwhile).
     * The CM first clears V96ONFLG. */
    ORDERS(BON, RTB),
    SWITCH(BON, QUITFLAG),
#if LUMINARY099
    ADDRESS(NOINT), /* "KILL INTEGRATION UNTIL NEXT P00." */
#else
    ADDRESS(NOINT), /* "NO STATEINT IF V96" */
#endif
    ADDRESS(LOADTIME),
    STORE(TDEC1),
#if LUMINARY099
    ORDER(CALL),
#else
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, V96ONFLG),
#endif
    ADDRESS(INTSTALL),
    ORDERS(SET, CALL),
    SWITCH(SET, NODOFLAG),
    ADDRESS(SETIFLGS),
    ORDER(GOTO),
    ADDRESS(STATEUP),
    /* 600SECS */ NUMBER(000003), /* 2DEC 60000 */
    NUMBER(025140),
    /* ENDINT, where STATEUP ends: STATEFLG cleared, and back to basic
     * code. */
    /* ENDINT */ ORDERS(CLEAR, EXIT),
    SWITCH(CLEAR, STATEFLG),
};

const struct interpretive_code STATINT1_CODE = {
    CODE_BANK(I_STATINT1), (word)(CODE_Z(I_STATINT1) + 1u), COUNT(statint1_words), statint1_words};

/* ENDINT +2, after its EXIT: group 2's phase (OCT 20032), and STATEINT
 * again in ten minutes (LONGCALL with 600SECS); the job ends
 * (ENDOFJOB). */
void endint_after_exit(void)
{
    tc_to((word)(ENDINT + 2u), PHASCHNG); /* OCT 20032 */
}

void endint_after_phaschng(void)
{
    dca(SECS600_AT);                      /* EXTEND; DCA 600SECS */
    tc_to((word)(ENDINT + 6u), LONGCALL); /* 2CADR STATEINT */
}

void endint_after_longcall(void)
{
    tc_to((word)(ENDINT + 011u), ENDOFJOB);
}

static const word setiflgs_words[] = {
    /* SETIFLGS -- LM p. 1210 (CM p. 1314): the flags of a precision
     * integration of the state vector alone, kept as the permanent one:
     * STATEFLG set; INTYPFLG (conic), DIM0FLAG (the W-matrix) and
     * D6OR9FLG cleared. */
    /* SETIFLGS */ ORDERS(SET, CLEAR),
    SWITCH(SET, STATEFLG),
    SWITCH(CLEAR, INTYPFLG),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(CLEAR, D6OR9FLG),
    ORDER(RVQ),
    /* NOINT: out of the interpreter. */
    /* NOINT */ ORDER(EXIT),
};

const struct interpretive_code SETIFLGS_CODE = {CODE_BANK(I_SETIFLGS), CODE_Z(I_SETIFLGS),
                                                COUNT(setiflgs_words), setiflgs_words};

/* NOINT +1, after its EXIT: group 2's phase (OCT 00002), QUITFLAG cleared
 * (DOWNFLAG) and the job ends. */
void noint_after_exit(void)
{
    tc_to((word)(NOINT + 1u), PHASCHNG); /* OCT 00002 */
}

void noint_after_phaschng(void)
{
    tc_to((word)(NOINT + 3u), DOWNFLAG); /* ADRES QUITFLAG */
}

void noint_after_downflag(void)
{
    tc_to((word)(NOINT + 5u), ENDOFJOB);
}
