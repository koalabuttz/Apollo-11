/* SPDX-License-Identifier: MIT */
/* TPI_SEARCH.agc of the CM, part 2 (Comanche055 pp. 558-559, bank 07):
 * TRANSANG, the "CENTRAL ANGLE SUBROUTINE".
 *
 * "THIS SUBROUTINE COMPUTES THE CENTRAL ANGLE OF TRAVEL OF THE PASSIVE
 * VEHICLE DURING THE TRANSFER": from its state at TPF (RPASS4, VPASS4),
 * the semimajor axis R/(2 - R V V/MU), and from it the mean motion,
 * SQRT(MU/A**3), times the transfer time TF, over 2 PI (2PISC):
 * "CENTANG = (SQRT(MU/ASUP***)TF)", "IN REVOLUTIONS B-0". X1 and X2 (from
 * XRS) pick the scaling: MUTABLE's MU and its square root by X1, the
 * vectors shifted by X2. It returns through SUBEXIT. */
#include "tpi_search.h"
#include "../agc/names.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- TRANSANG -- pp. 558-559 ---- */

/* The list as it goes, as the listing's comments give it (with each
 * value's scale): "SQRT MU (+18 OR +15)" at 00D, the "MAGNITUDE OF R
 * (+29 OR +27)" at 02D, "1/MU (+34 OR +28)" at 04D; then R V squared over
 * MU, 2 less that, the semimajor axis R over it at 02D, and "ASUBP***" at
 * 00D. */
static const word transang_words[] = {
    /* TRANSANG */ ORDERS(STQ, SETPD),
    ADDRESS(SUBEXIT),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(LXA_1, LXA_2),
    ADDRESS(XRS),
    ADDRESS_AT(XRS, 1),
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(VPASS4),
    SR_X2(0),
    STODL_STAR_AT(VVEC, 0),
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, 2),
    ORDERS(PDVL, VSR_STAR), /* "SQRT MU (+18 OR +15)" 00D */
    ADDRESS_PLUS_ONE(RPASS4),
    SR_X2(0),
    ORDERS(ABVAL, PDDL_STAR), /* "MAGNITUDE OF R (+29 OR +27)" 02D */
    ADDRESS_PLUS_ONE_AT_X1(MUTABLE, 0),
    ORDERS(PDVL, VSQ), /* "1/MU (+34 OR +28)" 04D */
    ADDRESS_PLUS_ONE(VVEC),
    ORDERS(NORM, DMPR), /* "PUSH LIST AT 02D" */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE_D(2),
    SRR_X1(-3),
    ORDER(BDSU), /* R V squared over MU (+6) */
    ADDRESS_PLUS_ONE(D1_32),
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X1),  /* 2 less that (+6-N) */
    ORDERS(SR1R, DDV),     /* "MAGNITUDE OF R (+30 OR +28)" */
    ORDERS(SL_STAR, PUSH), /* R over that (+29 OR +27), 02D */
    SL_X1(-5),
    ORDERS(SR1, SQRT),
    ORDER(DMP),
    ORDERS(NORM, PDDL), /* "ASUBP***" 00D */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(SR1, DDV),
    ORDERS(DMP, SL_STAR),
    ADDRESS_PLUS_ONE(TF),
    SL_X1(0),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE(2PISC),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, DDV),
    ORDER(SL_STAR),
    SL_X1(-3),       /* "CENTANG = (SQRT(MU/ASUP***)TF)" */
    STCALL(CENTANG), /* "IN REVOLUTIONS B-0" */
    ADDRESS(SUBEXIT),
};

const struct interpretive_code TRANSANG_CODE = {07u, TRANSANG, COUNT(transang_words),
                                                transang_words};

#endif
