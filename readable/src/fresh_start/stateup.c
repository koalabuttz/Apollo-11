/* SPDX-License-Identifier: MIT */
/* FRESH_START_AND_RESTART.agc's interpretive code, in the listing's
 * notation (src/interpreter/notation.h): STATEUP, in bank 13, and R00's
 * wait for the integration, in bank 04 (the CM calls it ROC). The
 * readable interpreter runs these words (src/binding/stateup_routines.c),
 * as the AGC's interpreter ran the listing's. */
#include "../agc/agc.h"
#include "../agc/names.h"
#include "../interpreter/notation.h"
#include "stateup.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099
/* STATEUP -- LM pp. 236-237.
 * "THIS ROUTINE DOES THE P00 INTEGRATION": P00 brings the two state
 * vectors up to the present now and then (INTEGRATION_INITIALIZATION's
 * CALL STATEUP). First the CM's, VINTFLAG set ("CM STATE VECTOR"), with
 * the W-matrix (DIM0FLAG) when the LM is on the moon's surface and the
 * W-matrix is valid; then the LM's (unless it is on the surface, where
 * it does not move: NO-INT), with the W-matrix when it is valid for
 * rendezvous, which may need all 9 dimensions (D6OR9FLG). PRECIFLG chooses
 * INTEGRV's way of stepping. NO-INT clears NODOFLAG ("V37 PERMITTED") and
 * ends. THISVINT, for MIDTOAV1 and 2, just clears VINTFLAG ("LM STATE
 * VECTOR") and returns. */
static const word bank13[] = {
    /* STATEUP */ ORDERS(SET, BOF), /* "EXTRAPOLATE CM STATE VECTOR" */
    SWITCH(SET, VINTFLAG),
    SWITCH(BOF, SURFFLAG), /* "ALSO 6X6 W-MATRIX IF LM ON LUNAR" */
    ADDRESS(DOINT),        /* "SURFACE AND W-MATRIX VALID" */
    ORDERS(BOF, SET),      /* "FOR RENDEZVOUS NAVIGATION." */
    SWITCH(BOF, RENDWFLG),
    ADDRESS(DOINT),
    SWITCH(SET, DIM0FLAG),
    /* DOINT */ ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, PRECIFLG), /* "ENGAGES 4-TIME STEP LOGIC IN INTEGRATION" */
    ADDRESS(INTEGRV),        /* "WHEN MODREG = 0" */
    ORDERS(BON, DLOAD),
    SWITCH(BON, SURFFLAG),
    ADDRESS(NO_INT),
    ADDRESS_PLUS_ONE(TETCSM),
    STCALL(TDEC1),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, CALL), /* "EXTRAPOLATE LM STATE VECTOR" */
    SWITCH(CLEAR, VINTFLAG),
    ADDRESS(SETIFLGS),
    ORDER(BOF), /* "ALSO 9X9 W-MATRIX IF W IS VALID" */
    SWITCH(BOF, RENDWFLG),
    ADDRESS(DOINT2),
    ORDERS(SET, SET),
    SWITCH(SET, DIM0FLAG),
    SWITCH(SET, D6OR9FLG),
    /* DOINT2 */ ORDERS(SET, CALL),
    SWITCH(SET, PRECIFLG), /* "DISENGAGE 4 TIME STEP LOGIC IN INTEG." */
    ADDRESS(INTEGRV),
    /* NO-INT */ ORDER(CLRGO),
    SWITCH(CLRGO, NODOFLAG),
    ADDRESS(ENDINT),
    /* THISVINT ("IS CALLED BY MIDTOAV1 AND 2") */ ORDERS(CLEAR, RVQ),
    SWITCH(CLEAR, VINTFLAG),
};
#define STATEUP_FIRST 02026u

/* R00 -- LM p. 229. After its TC INTPRET: "WAIT FOR INTEGRATION TO
 * FINISH" (INTSTALL returns when no integration is running), then leave
 * the interpreter at DUMMYAD for the basic code after it (r00_after_intpret
 * in v37.c). */
static const word bank04[] = {
    ORDER(CALL), /* "WAIT FOR INTEGRATION TO FINISH" */
    ADDRESS(INTSTALL),
    /* DUMMYAD */ ORDER(EXIT),
};
#define R00_FIRST 02144u
#else
/* STATEUP -- CM pp. 209-210.
 * "THIS ROUTINE DOES THE P00 INTEGRATION": P00 brings the two state
 * vectors up to the present now and then (INTEGRATION_INITIALIZATION's
 * CALL STATEUP). First the CM's, VINTFLAG set ("CM STATE VECTOR"), with
 * the W-matrix (DIM0FLAG) when it is valid for orbital navigation; then
 * the LM's (unless it is on the moon's surface, where it does not move:
 * STATEND), with the W-matrix when it is valid for rendezvous. PRECIFLG
 * chooses INTEGRV's way of stepping. STATEND clears NODOFLAG ("V37
 * PERMITTED") and ends. THISVINT, for MIDTOAV1 and 2, just sets VINTFLAG
 * and returns. */
static const word bank13[] = {
    /* STATEUP */ ORDERS(SET, BOF), /* "EXTRAPOLATE CM STATE VECTOR" */
    SWITCH(SET, VINTFLAG),
    SWITCH(BOF, ORBWFLAG),  /* "ALSO 6X6 W-MATRIX IF VALID" */
    ADDRESS_AT(STATEUP, 6), /* +3: "FOR ORBITAL NAVIGATION" */
    ORDER(SET),
    SWITCH(SET, DIM0FLAG),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, PRECIFLG),
    ADDRESS(INTEGRV),
    ORDERS(BON, DLOAD),
    SWITCH(BON, SURFFLAG),
    ADDRESS(STATEND),
    ADDRESS_PLUS_ONE(TETCSM),
    STCALL(TDEC1),
    ADDRESS(INTSTALL),
    ORDERS(CLEAR, CALL), /* "EXTRAPOLATE LM STATE VECTOR" */
    SWITCH(CLEAR, VINTFLAG),
    ADDRESS(SETIFLGS), /* "AND 6X6 W-MATRIX IF VALID" */
    ORDERS(BOF, SET),
    SWITCH(BOF, RENDWFLG),    /* "FOR RENDEZVOUS NAVIGATION" */
    ADDRESS_AT(STATEUP, 026), /* +2 */
    SWITCH(SET, DIM0FLAG),
    ORDERS(SET, CALL),
    SWITCH(SET, PRECIFLG),
    ADDRESS(INTEGRV),
    /* STATEND */ ORDER(CLRGO),
    SWITCH(CLRGO, NODOFLAG),
    ADDRESS(ENDINT),
    /* THISVINT ("IS CALLED BY MIDTOAV1 AND2") */ ORDERS(SET, RVQ),
    SWITCH(SET, VINTFLAG),
};
#define STATEUP_FIRST 02000u

/* ROC -- CM p. 199. After its TC INTPRET: "WAIT FOR INTEGRATION TO
 * FINISH" (INTSTALL returns when no integration is running), then leave
 * the interpreter at DUMMYAD for the basic code after it (roc_after_intpret
 * in v37.c). */
static const word bank04[] = {
    ORDER(CALL), /* "WAIT FOR INTEGRATION TO FINISH" */
    ADDRESS(INTSTALL),
    /* DUMMYAD */ ORDER(EXIT),
};
#define R00_FIRST     02131u
#endif

const struct interpretive_code STATEUP_CODE = {013u, STATEUP_FIRST, COUNT(bank13), bank13};
const struct interpretive_code R00_CODE = {04u, R00_FIRST, COUNT(bank04), bank04};
