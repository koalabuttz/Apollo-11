/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, SMCDURES (CM p. 1372), bank 23: the
 * CM's alone.
 *
 * "THIS SUBROUTINE COMPUTES INCREMENTAL CHANGES IN CDU(GIMBAL) ANGLES FROM
 * INCREMENTAL CHANGES ABOUT SM AXES. IT REQUIRES SM INCREMENTS AS A DP
 * VECTOR SCALED AT ONE REVOLUTION(DTHETASM,+2,+4)." The listing's matrix,
 * with the inner (IGA) and middle (MGA) gimbal angles, takes the stable
 * member's increments to the CDUs':
 *
 *     DCDU     = (COS(IGA) DTHETASM - SIN(IGA) DTHETASM +4) / COS(MGA)
 *     DCDU +2  = DTHETASM +2 - SIN(MGA) DCDU
 *     DCDU +4  = SIN(IGA) DTHETASM + COS(IGA) DTHETASM +4
 *
 * the sines and cosines at SINCDU and COSCDU (Y, Z, X: IGA the Y angle,
 * MGA the Z), "SCALED TO ONE HALF", so the products are shifted back up
 * one place (SL1). The faithful C is c64-port/src/cm_powered_subs.c. */
#include "powered_flight.h"
#include "../agc/names.h"

#if COMANCHE055

static const word smcdures_words[] = {
    /* SMCDURES */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(DTHETASM),
    ADDRESS_PLUS_ONE(COSCDUY),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE_AT(DTHETASM, 4),
    ADDRESS_PLUS_ONE(SINCDUY),
    ORDER(BDSU),
    ORDER(DDV),
    ADDRESS_PLUS_ONE(COSCDUZ),
    STORE(DCDU),
    ORDERS(DMP, SL1), /* "SCALE" */
    ADDRESS_PLUS_ONE(SINCDUZ),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE_AT(DTHETASM, 2),
    STODL_AT(DCDU, 2),
    ADDRESS_PLUS_ONE(DTHETASM),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE(SINCDUY),
    ADDRESS_PLUS_ONE_AT(DTHETASM, 4),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(COSCDUY),
    ORDER(SL1),
    STORE_AT(DCDU, 4),
    ORDER(RVQ),
};

const struct interpretive_code SMCDURES_CODE = {CODE_BANK(I_SMCDURES), CODE_Z(I_SMCDURES),
                                                COUNT(smcdures_words), smcdures_words};

#endif
