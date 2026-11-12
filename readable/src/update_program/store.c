/* SPDX-License-Identifier: MIT */
/* UPDATE_PROGRAM.agc, part 3: UPSTORE, the V73 clock increment, the job that
 * does the other updates (UPJOB), and TIMEDIDL, the clock increment V70 and
 * V73 share; LM pages 1391-1393, CM pages 1502-1504. The faithful C is
 * c64-port/src/lm_update.c and cm_update.c. */
#include "update_program.h"
#include "../interpreter/notation.h"

/* UPSTORE -- LM p. 1391, CM p. 1502: "GROUND HAS VERIFIED UPDATE. STORE
 * DATA." "INVERT VERIFLAG(BIT3 OF FLAGWRD7) TO INDICATE TO THE GROUND(VIA
 * DOWNLINK) THAT THE V33 (WHICH THE GROUND SENT TO VERIFY THE UPDATE) HAS
 * BEEN SUCCESSFULLY RECEIVED BY THE UPDATE PROGRAM", and "SET RESTART
 * GROUP 6 TO REDO THE UPDATE DATA STORE IF A RESTART OCCURS." */
void upstore(void)
{
    inhint();
    ca(ADDRESS_OF(FLAGWRD7));
    xch(L_ADDRESS);
#if LUMINARY099
    set_A(VERIFBIT);
#else
    set_A(BIT3);
#endif
    rxor_lchan();
    ts(ADDRESS_OF(FLAGWRD7));
    tc_to((word)(UPSTORE + 7u), PHASCHNG); /* OCT 04026 */
}

/* UPFNDVAC -- LM p. 1392, CM p. 1503: a VAC area "FOR 'CALL INTSTALL'",
 * at CHRPRIO ("USE EXTENDED VERB PRIORITY"), for the job UPJOB, whose
 * 2CADR also sets the EBANK "FOR 'TEPHEM' UPDATE BY V70". */
static void upfndvac(void)
{
    set_A(CHRPRIO);
    tc_to((word)(UPFNDVAC + 1u), FINDVAC); /* 2CADR UPJOB */
}

/* UPSTORE +011, where PHASCHNG returns: "(BECAUSE PHASCHNG DID A
 * RELINT)" INHINT again. "GO TO UPFNDVAC IF INSTALL IS REQUIRED. THAT IS,
 * IF IT'S A V70 - V72. GO TO UPEND73 IF IT'S A V73." UPEND73: "V73-PERFORM
 * DP OCTAL AGC CLOCK INCREMENT" (the increment in UPBUFF +8D, where
 * TIMEDIDR looks for it). */
void upstore_after_phaschng(void)
{
    inhint();
    set_A(negate(TWO));
    ad_at(ADDRESS_OF(UPVERB));
    if (A_is_zero_or_minus()) {
        upfndvac();
        return;
    }
    dca(ADDRESS_OF(UPBUFF));
    dxch(ADDRESS_OF(UPBUFF) + 8u);
    tc_at((word)(UPEND73 + 3u)); /* TC TIMEDIDL */
    timedidl();
}

/* UPEND73 +4 -- LM p. 1392, CM p. 1503, TIMEDIDL's error return: "ERROR-
 * TURN ON *OPERATOR ERROR* LIGHT". */
static void upend73_error(void)
{
    tc_to((word)(UPEND73 + 4u), FALTON);
}

/* UPEND73 +5 -- LM p. 1392, CM p. 1503, where FALTON returns, and TIMEDIDL's
 * normal return: "GO TO COMMON UPDATE PROGRAM EXIT" (UPOUT +1). */
void upend73_exit(void)
{
    tc_at((word)(UPEND73 + 5u));
    upout4();
}

/* UPFNDVAC +4 -- LM p. 1392, CM p. 1503, where FINDVAC returns: this job is
 * done; UPJOB is the new one. */
void upfndvac_after_findvac(void)
{
    tc_to((word)(UPFNDVAC + 4u), ENDOFJOB);
}

/* UPJOB -- LM p. 1392, CM p. 1503: "THIS COULD BE A STATE VECTOR
 * UPDATE--SO WAIT(PUT JOB TO SLEEP) IF ORBIT INT(OI) IS IN
 * PROGRESS--OR--GRAB OI AND RETURN TO UPWAKE IF OI IS NOT IN PROGRESS."
 * Its two interpretive words are UPJOB_CODE, below. */
void upjob(void)
{
    tc_to(UPJOB, INTPRET);
}

/* UPWAKE +1 -- LM p. 1392, CM p. 1503, after the interpretive EXIT: "RESTART
 * PROTECT(GROUP 6)". */
void upwake(void)
{
    tc_to((word)(UPWAKE + 1u), PHASCHNG); /* OCT 04026 */
}

/* UPWAKE +3 -- LM p. 1392, CM p. 1503, where PHASCHNG returns: "SET
 * INTEGRATION RESTART BIT". */
void upwake_after_phaschng(void)
{
    tc_to((word)(UPWAKE + 3u), UPFLAG); /* ADRES REINTFLG */
}

/* UPWAKE +5 -- LM p. 1392, CM p. 1503 and UPPART3, where UPFLAG returns:
 * INHINT, and INDEX UPVERB; TCF +1, "BRANCH TO THE APPROPRIATE UPDATE VERB
 * ROUTINE TO ACTUALLY PERFORM THE UPDATE": V70, V71 or V72 (UPVERB 3 lands
 * on TIMEDIDL itself, the word after the branches). Not modelled: an UPVERB
 * that makes the TCF any other. */
void upwake_after_upflag(void)
{
    word fan = (word)(UPPART3 + 2u); /* where the TCF +1 goes */
    word to;

    inhint();
    to = indexed_instruction(TCF_TO(fan), fetch(ADDRESS_OF(UPVERB)));
    if (to == TCF_TO(fan))
        upend70();
    else if (to == TCF_TO(fan + 1u))
        upend71();
    else if (to == TCF_TO(fan + 2u))
        upend72();
    else if (to == TCF_TO(TIMEDIDL))
        timedidl();
    else
        not_modelled();
}

/* TIMEDIDL -- LM p. 1392, CM p. 1504: the "ROUTINE TO INCREMENT
 * CLOCK(TIME2,TIME1) WITH CONTENTS OF DP WORD AT UPBUFF." (UPBUFF +8D by
 * now). "SAVE Q FOR RETURN" in UPTEMP; "ZERO AND SAVE TIME2,TIME1" ("STORE
 * IN CASE OF OVERFLOW", in UPBUFF +18D); and "DO A QUICK PHASCHNG": group
 * 6 to TIMEDIDR (-PHASE6, PHASE6 = -5, 5). */
void timedidl(void)
{
    qxch(ADDRESS_OF(UPTEMP));
    set_A(ZERO);
    zl();
    dxch(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(UPBUFF) + 18u);
    set_A(UPDTPHAS);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE6));
    timedidr();
}

/* TIMEDIDR -- LM p. 1393, CM p. 1504, and the restart point: "PICK UP
 * INCREMENTER(AND ZERO IT IN CASE OF RESTARTS) AND STORE IT INTO MPAC FOR
 * TPAGREE." "FORM SUM IN MPAC" with the old clock, and "TEST FOR
 * OVERFLOW". On an overflow, "RESTORE OLD VALUE OF CLOCK AND TURN ON
 * OPERATOR ERROR": "RESTART PROTECT(GROUP 6)", then the error return.
 * Otherwise DELTATOK. */
void timedidr(void)
{
    inhint();
    set_A(ZERO);
    zl();
    ts(ADDRESS_OF(MPAC) + 2u);
    dxch(ADDRESS_OF(UPBUFF) + 8u);
    dxch(ADDRESS_OF(MPAC));
    dca(ADDRESS_OF(UPBUFF) + 18u);
    das(ADDRESS_OF(MPAC));
    if (A_is_zero()) {
        tc_to(DELTATOK, TPAGREE); /* "FORCE SIGN AGREEMENT" */
        return;
    }
    set_A(ZERO);
    dxch(ADDRESS_OF(UPBUFF) + 18u);
    das(ADDRESS_OF(TIME2));
    tc_to((word)(TIMEDIDR + 016u), PHASCHNG); /* OCT 04026 */
}

/* TIMEDIDR +020 -- LM p. 1393, CM p. 1504, where PHASCHNG returns: TC
 * UPTEMP, "GO TO ERROR EXIT": the AGC runs the word TIMEDIDL saved there,
 * its caller's return, as TC to it. That is UPEND73 +4 or UPEND70 +4; not
 * modelled, any other word. */
void timedidr_after_phaschng(void)
{
    word to = fetch(ADDRESS_OF(UPTEMP));

    if (to != UPEND73 + 4u && to != UPEND70 + 4u) not_modelled();
    tc_through(ADDRESS_OF(UPTEMP));
    if (to == UPEND73 + 4u)
        upend73_error();
    else
        upend70_clock_error();
}

/* DELTATOK +1 -- LM p. 1393, where TPAGREE returns: "INCREMENT TIME2,TIME1",
 * and "RESTART PROTECT(GROUP 6)". */
void deltatok_after_tpagree(void)
{
    dxch(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(TIME2));
    tc_to((word)(DELTATOK + 3u), PHASCHNG); /* OCT 04026 */
}

/* DELTATOK +5 -- LM p. 1393, where PHASCHNG returns: INHINT, and INDEX
 * UPTEMP; TC 1, the "NORMAL RETURN" to the word after the error return
 * ("CODED THIS WAY FOR RESTART PROTECTION"): UPEND73 +5 or UPEND70 +5. Not
 * modelled: an UPTEMP that makes it any other. */
void deltatok_after_phaschng(void)
{
    word to;

    inhint();
    to = indexed_instruction(1u, fetch(ADDRESS_OF(UPTEMP))); /* TC 1 */
    if (to != UPEND73 + 5u && to != UPEND70 + 5u) not_modelled();
    tc_at((word)(DELTATOK + 7u));
    if (to == UPEND73 + 5u)
        upend73_exit();
    else
        upend70_after_timedidl();
}

/* UPJOB +1 -- CALL INTSTALL: "WAIT(PUT JOB TO SLEEP) IF ORBIT INT(OI) IS
 * IN PROGRESS--OR--GRAB OI AND RETURN TO UPWAKE IF OI IS NOT IN
 * PROGRESS.", and UPWAKE: EXIT, to the basic code after it (upwake()). */
static const word upjob_words[] = {
    ORDER(CALL),
    ADDRESS(INTSTALL),
    /* UPWAKE */ ORDER(EXIT),
};

#if LUMINARY099
#define UPDATE_BANK 04u
#else
#define UPDATE_BANK 027u
#endif

const struct interpretive_code UPJOB_CODE = {
    UPDATE_BANK, (word)(UPJOB + 1u), sizeof upjob_words / sizeof upjob_words[0], upjob_words};
