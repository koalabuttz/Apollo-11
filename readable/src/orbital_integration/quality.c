/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, QUALITY1 and QUALITY2 (LM pp. 1247-1248), in
 * the listing's notation (src/interpreter/notation.h). The LM's only: the
 * CM keeps them in RT8_OP_CODES.
 *
 * COMTERM calls QUALITY1 when it has added the oblateness to FV. For the
 * earth there is nothing more: on to NBRANCH. For the moon, QUALITY2
 * starts the term of its equator not being round (J22): from the
 * position in the moon's frame (URPV), 5(Y**2-X**2) times the position,
 * twice its X added to the X and twice its Y taken from the Y, times
 * E3J22R2M, into 2D, and back to QUALITY3 (RVQ) with the position's Z in
 * MPAC. */
#include "orbital_integration.h"
#include "../agc/names.h"

#if LUMINARY099
static const word quality_words[] = {
    /* QUALITY1 -- LM p. 1247: the earth's, NBRANCH. */
    /* QUALITY1 */ ORDERS(BOF, DLOAD),
    SWITCH(BOF, MOONFLAG),
    ADDRESS(NBRANCH),
    ADDRESS_PLUS_ONE(URPV),
    ORDER(DSQ),
    /* QUALITY2 */ ORDERS(PDDL, DSQ), /* "SQUARE INTO 2D, B2" */
    ADDRESS_PLUS_ONE_AT(URPV, 2),     /* "Y COMPONENT, B1" */
    ORDER(DSU),
    ORDERS(DMP, VXSC),     /* "5(Y**2-X**2)UR" */
    ADDRESS_PLUS_ONE(5_8), /* "CONSTANT, 5B3" */
    ADDRESS_PLUS_ONE(URPV),
    /* LM p. 1248 */
    ORDERS(VSL3, PDDL),     /* "STORE SCALED B3 IN 2D, 4D, 6D FOR XYZ" */
    ADDRESS_PLUS_ONE(URPV), /* "X COMPONENT, B1" */
    ORDERS(SR1, DAD),       /* "2 X X COMPONENT FOR B3 SCALING" */
    ADDRESS_PLUS_ONE_D(2),
    STODL_D(2),
    ADDRESS_PLUS_ONE_AT(URPV, 2), /* "Y COMPONENT, B1" */
    ORDERS(SR1, BDSU),            /* "2 X Y COMPONENT FOR B3 SCALING" */
    ADDRESS_PLUS_ONE_D(4),
    STORE_D(4), /* "2D HAS VECTOR, B3." */
    ORDERS(SLOAD, VXSC),
    ADDRESS_PLUS_ONE(E3J22R2M),
    ORDERS(PDDL, RVQ),          /* "J22 TERM X R**4 IN 2D, SCALED B61" */
    ADDRESS_PLUS_ONE(COSPHI_2), /* "SAME AS URPV +4  Z COMPONENT" */
};

const struct interpretive_code QUALITY1_CODE = {CODE_BANK(I_QUALITY1), CODE_Z(I_QUALITY1),
                                                COUNT(quality_words), quality_words};
#endif
