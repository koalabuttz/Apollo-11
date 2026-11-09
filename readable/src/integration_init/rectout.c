/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, the end of an integration: A-PCHK,
 * PHEXIT, RECTOUT and INTEXIT (LM pp. 1216-1217, CM pp. 1320-1321). The
 * interpretive words are in the listing's notation
 * (src/interpreter/notation.h); the basic code between them is C.
 *
 * A-PCHK is where the integration has reached TDEC. With STATEFLG set
 * the answer is also the new permanent state vector: a phase change of
 * restart group 2, REINTFLG set, and the transfer (ATOPCSM or ATOPLEM),
 * which comes back to PHEXIT. RECTOUT then gives the caller the state:
 * RATT, VATT, TAT and the central body's MU on the push-down list, X1
 * for the conics. INTEXIT clears the flags and goes back to the caller by
 * INTWAKE (part 3), with the return in QPRET. The CM first checks, near
 * the moon's sphere (MIDFLAG), whether the origin should change
 * (CHKSWTCH, ORIGCHNG). */
#include "integration_init.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

/* The EXIT out of A-PCHK's interpretive words: the LM's A-PCHK BOF EXIT,
 * the CM's ANDOUT BOFCLR EXIT. The basic code is the next word on. */
#if LUMINARY099
#define BOF_EXIT       A_PCHK
#define BOF_EXIT_LABEL I_A_PCHK
#else
#define BOF_EXIT       ANDOUT
#define BOF_EXIT_LABEL I_ANDOUT
#endif

static const word a_pchk_words[] = {
#if COMANCHE055
    /* A-PCHK -- CM p. 1320: with MIDFLAG set, CHKSWTCH, and ORIGCHNG when
     * it finds the vehicle has crossed into the other sphere ("MAKE THE
     * SWITCH"). */
    /* A-PCHK */ ORDERS(BOF, CALL),
    SWITCH(BOF, MIDFLAG),
    ADDRESS(ANDOUT), /* "DONT MAKE ORIGIN CHANGE CHECK" */
    ADDRESS(CHKSWTCH),
    ORDERS(BPL, CALL),
    ADDRESS(ANDOUT),   /* "NO ORIGIN CHANGE" */
    ADDRESS(ORIGCHNG), /* "MAKE THE SWITCH" */
                       /* ANDOUT: no permanent vector (STATEFLG clear), RECTOUT; otherwise
     * STATEFLG cleared and out of the interpreter. */
    /* ANDOUT */ ORDERS(BOFCLR, EXIT),
    SWITCH(BOFCLR, STATEFLG),
    ADDRESS(RECTOUT),
#else
    /* A-PCHK -- LM p. 1216: no permanent vector (STATEFLG clear),
     * RECTOUT; otherwise out of the interpreter. */
    /* A-PCHK */ ORDERS(BOF, EXIT),
    SWITCH(BOF, STATEFLG),
    ADDRESS(RECTOUT),
#endif
};

const struct interpretive_code A_PCHK_CODE = {CODE_BANK(I_A_PCHK), CODE_Z(I_A_PCHK),
                                              COUNT(a_pchk_words), a_pchk_words};

/* TC at "at" to a routine outside the file (PHASCHNG, UPFLAG), which
 * reads the word after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* After the EXIT: group 2's phase (OCT 04022), REINTFLG set ("PHASE
 * CHANGE HAS OCCURRED BETWEEN" INTSTALL and INTWAKE), and back into the
 * interpreter. */
void a_pchk_after_exit(void)
{
    tc_to((word)(BOF_EXIT + 3u), PHASCHNG); /* OCT 04022 */
}

void a_pchk_after_phaschng(void)
{
    tc_to((word)(BOF_EXIT + 5u), UPFLAG); /* ADRES REINTFLG */
}

void a_pchk_after_upflag(void)
{
    tc_at((word)(BOF_EXIT + 7u)); /* TC INTPRET */
    intpret();
}

static const word phexit_words[] = {
    /* The transfer to the permanent vector, ATOPCSM for the CSM (VINTFLAG
     * set), ATOPLEM for the LM, its return (QPRET) PHEXIT. PHEXIT: group
     * 2's phase again (GRP2PC). */
    ORDER(SSP),
    ADDRESS_PLUS_ONE(QPRET),
    ADDRESS(PHEXIT),
    ORDERS(BON, GOTO),
    SWITCH(BON, VINTFLAG),
    ADDRESS(ATOPCSM),
    ADDRESS(ATOPLEM),
    /* PHEXIT */ ORDER(CALL),
    ADDRESS(GRP2PC),
    /* RECTOUT -- LM p. 1216 (CM p. 1321): RECTIFY, and the state for the
     * caller, scaled by X2 for the central body: "RATT TO PD0", "VATT TO
     * PD6", "TAT TO PD12"; then RRECT, VRECT and the body's MU pushed too
     * (MUEARTH,2: an index is subtracted, so the moon's MUM for X2 2). X1
     * -10 for the earth, -2 for the moon. */
    /* RECTOUT */ ORDERS(SETPD, CALL),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(RECTIFY),
    ORDERS(VLOAD, VSL_STAR),
    ADDRESS_PLUS_ONE(RRECT),
    VSL_X2(0),
    ORDERS(PDVL, VSL_STAR), /* "RATT TO PD0" */
    ADDRESS_PLUS_ONE(VRECT),
    VSL_X2(0),
    ORDERS(PDDL, PDVL), /* "VATT TO PD6", "TAT TO PD12" */
    ADDRESS_PLUS_ONE(TET),
    ADDRESS_PLUS_ONE(RRECT),
    ORDERS(PDVL, PDDL_STAR),
    ADDRESS_PLUS_ONE(VRECT),
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, 0),
    ORDERS(PUSH, AXT_1),
    MINUS_NUMBER(10),
    ORDERS(BON, AXT_1),
    SWITCH(BON, MOONFLAG),
    ADDRESS(INTEXIT), /* (+2) */
    MINUS_NUMBER(2),
    /* INTEXIT -- LM p. 1217 (CM p. 1321): the push-down list emptied, the
     * overflow indicator cleared (BOV +1), AVEMIDSW cleared ("ALLOW UPDATE
     * OF DOWNLINK STATE VECTOR") and PRECIFLG; the CM's MIDAVFLG too, and
     * the LM's STATEFLG. The return (IRETURN) loaded, and out of the
     * interpreter. */
    /* INTEXIT */ ORDERS(SETPD, BOV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_AT(INTEXIT, 3), /* (+1) */
#if COMANCHE055
    ORDER(CLEAR),
    SWITCH(CLEAR, MIDAVFLG),
#endif
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, AVEMIDSW), /* "ALLOW UPDATE OF DOWNLINK STATE VECTOR" */
    SWITCH(CLEAR, PRECIFLG),
#if LUMINARY099
    ORDER(CLEAR),
    SWITCH(CLEAR, STATEFLG),
#endif
    ORDERS(SLOAD, EXIT),
    ADDRESS_PLUS_ONE(IRETURN),
};

/* The words after A-PCHK's TC INTPRET. */
const struct interpretive_code PHEXIT_CODE = {CODE_BANK(BOF_EXIT_LABEL),
                                              (word)(CODE_Z(BOF_EXIT_LABEL) + 010u),
                                              COUNT(phexit_words), phexit_words};
