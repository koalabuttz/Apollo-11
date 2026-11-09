/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, DOSWITCH, ORIGCHNG, RECTIFY and MINIRECT (LM
 * pp. 1239-1240, CM pp. 1345-1347), in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs.
 *
 * DOSWITCH changes the center of the integration when the vehicle has
 * crossed the moon's sphere of influence, and goes on with the step
 * (INTGRATE). ORIGCHNG first puts the deviation into the conic (RECTIFY),
 * then moves the position and the velocity to the other center: less the
 * other body's position (RPQV) and velocity (LUNVEL; negated from the
 * moon, VCOMP), each shifted to its scale (by X2, the central body), into
 * RRECT, RCV, VRECT and VCV; and goes back by the return kept in ORIGEX
 * (through QPRET) to CLRMOON or SETMOON, which change MOONFLAG and PBODY.
 *
 * "THE RECTIFY SUBROUTINE IS CALLED BY THE INTEGRATION PROGRAM AND
 * OCCASIONALLY BY THE MEASUREMENT INCORPORATION" routines, "TO ESTABLISH
 * A NEW CONIC.": the conic plus the deviation (TDELTAV, TNUV, shifted by
 * the central body) is the new conic's start, in RRECT and RCV, VRECT and
 * VCV; the deviations, TC and XKEP are zeroed. MINIRECT is its second
 * half, for a caller with the position stored and the velocity in MPAC. */
#include "orbital_integration.h"
#include "../agc/names.h"

static const word doswitch_words[] = {
    /* DOSWITCH -- LM p. 1239 (CM p. 1345) */
    /* DOSWITCH */ ORDER(CALL),
    ADDRESS(ORIGCHNG),
    ORDER(GOTO),
    ADDRESS(INTGRATE),
    /* ORIGCHNG -- LM p. 1239 (CM p. 1346): the return in ORIGEX; RECTIFY;
     * the position less the other body's. */
    /* ORIGCHNG */ ORDERS(STQ, CALL),
    ADDRESS(ORIGEX),
    ADDRESS(RECTIFY),
    ORDERS(VLOAD, VSL_STAR),
    ADDRESS_PLUS_ONE(RCV),
    VSL_X2(0),
    ORDERS(VSU, VSL_STAR),
    ADDRESS_PLUS_ONE(RPQV),
    VSL_X2(2),
    STORE(RRECT),
    STODL(RCV),
    ADDRESS_PLUS_ONE(TET),
    /* The velocity less the moon's (LUNVEL, at TET; from the moon, its
     * negative). */
    ORDER(CALL),
    ADDRESS(LUNVEL),
    ORDERS(BOF, VCOMP),
    SWITCH(BOF, MOONFLAG),
    ADDRESS_AT(ORIGCHNG, 021), /* (+1) */
    ORDERS(PDVL, VSL_STAR),
    ADDRESS_PLUS_ONE(VCV),
    VSL_X2(0),
    ORDER(VSU),
    ORDER(VSL_STAR),
    VSL_X2(2),
    STORE(VRECT),
    STORE(VCV),
    /* The return into QPRET; on to CLRMOON (from the moon) or SETMOON. */
    ORDERS(LXA_2, SXA_2),
    ADDRESS(ORIGEX),
    ADDRESS(QPRET),
    ORDERS(BON, GOTO),
    SWITCH(BON, MOONFLAG),
    ADDRESS(CLRMOON),
    ADDRESS(SETMOON),
    /* RECTIFY -- LM p. 1240 (CM p. 1347): RRECT and RCV the conic's
     * position plus the deviation; then MINIRECT with the velocity. */
    /* RECTIFY */ ORDERS(LXA_2, VLOAD),
    ADDRESS(PBODY),
    ADDRESS_PLUS_ONE(TDELTAV),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-7),
    ADDRESS_PLUS_ONE(RCV),
    STORE(RRECT),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(TNUV),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-4),
    ADDRESS_PLUS_ONE(VCV),
    /* MINIRECT: VRECT and VCV; the deviations, TC and XKEP zero. */
    /* MINIRECT */ STORE(VRECT),
    STOVL(VCV),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(TDELTAV),
    STODL(TNUV),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(TC),
    STORE(XKEP),
    ORDER(RVQ),
};

const struct interpretive_code DOSWITCH_CODE = {CODE_BANK(I_DOSWITCH), CODE_Z(I_DOSWITCH),
                                                COUNT(doswitch_words), doswitch_words};
