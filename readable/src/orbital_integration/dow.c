/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, from WMATEND's TC INTPRET to DOW..1 (LM pp.
 * 1244-1246, CM pp. 1351-1353), and the LM's SETITCTR, in the listing's
 * notation (src/interpreter/notation.h), which the readable interpreter
 * runs.
 *
 * "ORBITAL ROUTINE FOR EXTRAPOLATION OF THE W MATRIX. IT COMPUTES THE
 * SECOND DERIVATIVE OF EACH COLUMN POSITION" vector of the W-matrix, from
 * the vehicle's positions at the stage (VECTAB, by X1) that the state
 * vector's step left: DOW..1 gives the gravity gradient of a body (its MU
 * in BETAM) times the column's position, ALPHAV: three times its
 * projection on the vehicle's unit position less the column itself, over
 * the cube of the distance (normalized by S2 and 34D, shifted back at the
 * end). In midcourse (MIDFLAG) the other body's term is added (MUEARTH
 * -2,2, the other body's MU; from the moon, shifted to its scale), with
 * the positions six words on (INCR,1 DEC -6). Then NBRANCH.
 *
 * SETITCTR, the LM's only: ITERCTR 20 for LAMBERT, or 5 with AVEGFLAG
 * set. Its comments say it belongs in INITVEL, and is here so that only
 * one module had to be made again. */
#include "orbital_integration.h"
#include "../agc/names.h"

static const word dow_words[] = {
    ORDER(GOTO),
    ADDRESS(TESTLOOP), /* "FINISH INTEGRATING STATE VECTOR" */

    /* DOW.. -- LM p. 1245 (CM p. 1352): the central body's term into FV;
     * out of midcourse, NBRANCH. */
    /* DOW.. */ ORDERS(LXA_2, DLOAD_STAR),
    ADDRESS(PBODY),
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, 0),
    STCALL(BETAM),
    ADDRESS(DOW__1),
    STORE(FV),
    ORDERS(BOF, INCR_1),
    SWITCH(BOF, MIDFLAG),
    ADDRESS(NBRANCH),
    MINUS_NUMBER(6),
    /* The other body's term (from the moon, VSR6 to the moon's scale),
     * added to FV. */
    ORDERS(LXC_2, DLOAD_STAR),
    ADDRESS(PBODY),
    ADDRESS_PLUS_ONE_AT_X2(MUEARTH, -2),
    STCALL(BETAM),
    ADDRESS(DOW__1),
    ORDERS(BON, VSR6),
    SWITCH(BON, MOONFLAG),
    ADDRESS_AT(DOW__, 022), /* (+1) */
    ORDER(VAD),
    ADDRESS_PLUS_ONE(FV),
    STCALL(FV),
    ADDRESS(NBRANCH),
    /* DOW..1: 3/4 of ALPHAV's projection on the vehicle's unit position
     * (the unit at 1/2), less ALPHAV/16, times BETAM over the cube of the
     * distance. */
    /* DOW..1 */ ORDERS(VLOAD, VSR4),
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(PDVL_STAR, UNIT),
    ADDRESS_PLUS_ONE_AT_X1(VECTAB, 0),
    ORDERS(PDVL, VPROJ),
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(VXSC, VSU),
    ADDRESS_PLUS_ONE(3_4),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE_D(36),
    ADDRESS_PLUS_ONE(S2),
    ORDERS(PUSH, DSQ),
    ORDER(DMP),
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE_D(34),
    ADDRESS_PLUS_ONE(BETAM),
    ORDERS(SR1, DDV),
    ORDER(VXSC),
    /* The shifts of the normalizing (S2 three times and 34D) in X2, and
     * the vector shifted back by them. */
    ORDERS(LXA_2, XAD_2),
    ADDRESS(S2),
    ADDRESS(S2),
    ORDERS(XAD_2, XAD_2),
    ADDRESS(S2),
    ADDRESS_D(34),
    ORDERS(VSL_STAR, RVQ),
    /* LM p. 1246 (CM p. 1353) */
    VSL_X2(-8),
#if LUMINARY099
    /* SETITCTR: "SET ITERCTR FOR LAMBERT CALLS." */
    /* SETITCTR */ ORDERS(SSP, BOFF),
    ADDRESS_PLUS_ONE(ITERCTR),
    NUMBER(20),
    SWITCH(BOFF, AVEGFLAG),
    ADDRESS(LAMBERT),
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(ITERCTR),
    NUMBER(5),
    ADDRESS(LAMBERT),
#endif
};

/* The words after WMATEND's TC INTPRET. */
const struct interpretive_code DOW_CODE = {CODE_BANK(I_WMATEND), (word)(CODE_Z(I_WMATEND) + 012u),
                                           COUNT(dow_words), dow_words};
