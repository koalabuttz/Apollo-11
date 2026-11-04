/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 1 (Luminary099 pp. 761-763, bank 36): UPDATEVG,
 * STEERING, NSTEER and GETDT. Their basic code, and their interpretive
 * code in the listing's notation. The faithful C is
 * c64-port/src/lm_p40_p47.c.
 *
 * STEERING is SERVICER's exit during a P40 or P42 burn (AVEGEXIT, from
 * item 7 of BURNBABY's P40TABLE and P42TABLE): UPDATEVG brings the velocity
 * to be gained up to date ("X-PRODUCT STEERING", S40.8) and, for a burn to
 * an aim point (XDELVFLG clear), starts S40.9, "LAMBERT VTOGAIN", as a
 * job. NSTEER then checks the impulse switch: a burn about to end has the
 * engine off task (ENGOFTSK) set for its time to go.
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

/* TC Q: the routine returns to the address in Q (bits 1-12), and Q
 * becomes 3. Not modelled: a Q that is not such an address (bits 13-15
 * set, or below 010). */
static void tc_q(void)
{
    word at = (word)(Q & 07777u);

    if ((Q & 070000u) || Q < 010u) not_modelled();
    Q = 3u;
    Z = at;
}

/* ---- UPDATEVG -- p. 761 ---- */

/* UPDATEVG_CODE: the return kept in QTEMP1 (STQ), and S40.8, "X-PRODUCT
 * STEERING". With XDELVFLG set (an external delta V burn), back through
 * QTEMP1; with NORMSW set, on at 180SETUP. Otherwise, a Lambert burn:
 * when PIPTIME less TIGSAVE less TNEWA is negative, on at GETRANS;
 * otherwise TIGSAVEP is TIGSAVE plus TNEWA. 180SETUP: to basic code. */
static const word updatevg_words[] = {
    ORDERS(STQ, CALL),
    ADDRESS(QTEMP1),
    ADDRESS(S40_8),
    ORDERS(BON, BON),
    SWITCH(BON, XDELVFLG),
    ADDRESS(QTEMP1),
    SWITCH(BON, NORMSW),
    ADDRESS(180SETUP),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(PIPTIME),
    ADDRESS_PLUS_ONE(TIGSAVE),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(TNEWA),
    ADDRESS(GETRANS),
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(TIGSAVE),
    ADDRESS_PLUS_ONE(TNEWA),
    STORE(TIGSAVEP),
    /* 180SETUP */
    ORDER(EXIT),
};

/* 180SETUP +1, after the EXIT: with restart group 2 active (PHASE2
 * positive), straight on at NO.9. Otherwise S40.9, "LAMBERT VTOGAIN", as
 * a job of priority 10 (FINDVAC). A negative PHASE2 comes into the INHINT
 * with A |PHASE2| - 1, and -0 into the TC FINDVAC, as CCS goes on. */
void updatevg_after_exit(void)
{
    switch (ccs_at(ADDRESS_OF(PHASE2))) {
    case POSITIVE:
        tc_at(NO_9); /* TCF NO.9: its TC INTPRET */
        intpret();
        return;
    case IS_PLUS_ZERO: set_A(PRIO10); /* fall through */
    case NEGATIVE: inhint();          /* fall through */
    case IS_MINUS_ZERO: break;
    }
    tc_to(ONE_EIGHTY_SETUP + 5u, FINDVAC); /* 2CADR S40.9 */
}

/* 180SETUP +010: "2.17SPOT FOR S40.9", and "HERE AND REREADAC AFTER
 * RESTART" (2PHSCHNG, OCT 00172, OCT 10035). */
void updatevg_after_findvac(void)
{
    tc_to(ONE_EIGHTY_SETUP + 010u, TWO_PHSCHNG);
}

/* ENDSTEER: to the interpreter, for ENDSTEER_CODE. */
void endsteer(void)
{
    tc_at(ENDSTEER);
    intpret();
}

/* ENDSTEER_CODE: TIGSAVE from TIGSAVEP, and the state vector (RN, VN) to
 * RINIT and VINIT. GETRANS: DELLT4, TPASS4 less PIPTIME; back through
 * QTEMP1 (STCALL). */
static const word endsteer_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TIGSAVEP),
    STOVL(TIGSAVE),
    ADDRESS_PLUS_ONE(RN),
    STOVL(RINIT),
    ADDRESS_PLUS_ONE(VN),
    STORE(VINIT),
    /* GETRANS */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TPASS4),
    ADDRESS_PLUS_ONE(PIPTIME),
    STCALL(DELLT4),
    ADDRESS(QTEMP1),
};

/* NO_9_CODE (NO.9 +1): back through QTEMP1. */
static const word no_9_words[] = {
    ORDER(GOTO),
    ADDRESS(QTEMP1),
};

/* ---- STEERING and NSTEER -- p. 762 ---- */

/* STEERING: to the interpreter, for STEERING_CODE. */
void steering(void)
{
    tc_at(STEERING);
    intpret();
}

/* STEERING_CODE: UPDATEVG. */
static const word steering_words[] = {
    ORDER(CALL),
    ADDRESS(UPDATEVG),
    ORDER(EXIT),
};

/* NSTEER, after the EXIT: "CHECK IMPULSE SWITCH. IT IS SET EITHER BY
 * S40.13 IF TBURN<6 SECS OR BY S40.8 IF STEERING IS ALMOST DONE." With
 * IMPULSW clear, back to SERVICER (POSTJUMP, CADR SERVEXIT). With it set,
 * the idle flag: "DVMON OFF --> IMPULSW ON VIA S40.13 --> EXIT"; "DVMON
 * ON --> THRUSTING --> IMPULSW VIA S40.8": the rates stopped (STOPRATE).
 * MASK leaves A positive or +0: those are the two ways each CCS goes on
 * here. */
void nsteer(void)
{
    inhint();
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    cs(ADDRESS_OF(FLAGWRD2));
    mask(IMPULBIT);
    if (ccs_register(A_ADDRESS) != POSITIVE) { /* IMPULSW set */
        cs(ADDRESS_OF(FLAGWRD7));
        mask(IDLEFBIT);
        if (ccs_register(A_ADDRESS) == POSITIVE) { /* IDLEFLAG clear: TCF +3 */
            tc_to(NSTEER + 015u, IBNKCALL);        /* CADR STOPRATE */
            return;
        }
    }
    tc_to(NSTEER + 013u, POSTJUMP); /* CADR SERVEXIT */
}

/* NSTEER +017: "TURN OFF IMPULSW", */
void nsteer_after_stoprate(void)
{
    tc_to(NSTEER + 017u, DOWNFLAG); /* ADRES IMPULSW */
}

/* "TURN OFF DVMON" (IDLEFLAG set), */
void nsteer_after_impulsw(void)
{
    tc_to(NSTEER + 021u, UPFLAG); /* ADRES IDLEFLAG */
}

/* the time to ignition from now (TIG less TIME2, in MPAC; TPAGREE makes
 * its two words agree in sign), */
void nsteer_after_idleflag(void)
{
    inhint();
    dca(ADDRESS_OF(TIG)); /* EXTEND; DCA */
    dxch(ADDRESS_OF(MPAC));
    dcs(ADDRESS_OF(TIME2)); /* EXTEND; DCS */
    das(ADDRESS_OF(MPAC));
    tc_to(NSTEER + 032u, TPAGREE);
}

/* as a delay of at least a centisecond (GETDT), */
void nsteer_after_tpagree(void)
{
    ca(ADDRESS_OF(MPAC) + 1u); /* CAE */
    tc_at(NSTEER + 034u);
    getdt();
    nsteer_after_getdt();
}

/* for ENGOFTSK (TWIDDLE); */
void nsteer_after_getdt(void)
{
    tc_to(NSTEER + 035u, TWIDDLE); /* ADRES ENGOFTSK */
}

/* "ENGOFTSK (ENGINOFF)", "SERVICER -- REREADAC" (2PHSCHNG, OCT 40114,
 * OCT 00035), */
void nsteer_after_twiddle(void)
{
    tc_to(NSTEER + 037u, TWO_PHSCHNG);
}

/* and the job ends. */
void nsteer_after_2phschng(void)
{
    continue_at(ENDOFJOB);
}

/* ---- GETDT -- p. 763 ---- */

/* GETDT: the time in A (centiseconds) as a delay of at least one: A if
 * positive, 1 for +0, -0 or a negative A. TGO gets it as a DP number,
 * and A again; back to the caller (TC Q). */
void getdt(void)
{
    if (ccs_register(A_ADDRESS) == NEGATIVE) set_A(ZERO); /* CCS A; TCF +3; TCF +2; CAF ZERO */
    ad(ONE);
    xch(L_ADDRESS);
    set_A(ZERO);
    dxch(ADDRESS_OF(TGO));
    ca(ADDRESS_OF(TGO) + 1u);
    tc_q();
}

/* ---- The interpretive code ---- */

const struct interpretive_code UPDATEVG_CODE = {036u, UPDATEVG, COUNT(updatevg_words),
                                                updatevg_words};
const struct interpretive_code ENDSTEER_CODE = {036u, ENDSTEER + 1u, COUNT(endsteer_words),
                                                endsteer_words};
const struct interpretive_code NO_9_CODE = {036u, NO_9 + 1u, COUNT(no_9_words), no_9_words};
const struct interpretive_code STEERING_CODE = {036u, STEERING + 1u, COUNT(steering_words),
                                                steering_words};

#endif
