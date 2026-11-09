/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, NEWSTATE and LAMENTER (LM p. 1184, CM p. 1287):
 * the state at the end of a conic, in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs.
 *
 * After DELTIME has found T for an X (in TIME-THETA, TIME-RADIUS and
 * LAMBERT), the position and velocity there from the initial ones (UR1, the
 * unit position, R1 its length, and VVEC), the way KEPCONVG makes KEPLER's:
 * R2VEC and V2VEC. LAMBERT has its own R2 already, and enters at
 * LAMENTER for the velocity alone. */
#include "conics.h"
#include "../agc/names.h"

static const word newstate_words[] = {
    /* NEWSTATE -- LM p. 1184 (CM p. 1287): the position, from the f and
     * g functions of X: R2VEC = (R1 - X squared C(XI)) UR1 + (T - X cubed
     * S(XI)/ROOTMU) VVEC, pushed, and its length. */
    /* NEWSTATE */ ORDERS(DLOAD, SR4R),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(DSU, VXSC),
    ADDRESS_PLUS_ONE(XSQC_XI),
    ADDRESS_PLUS_ONE(UR1),
    ORDERS(VSL1, PDDL), /* "0D=(R1-XSQC(XI))UR1 (+33 OR 31) PL AT 6" */
    ADDRESS_PLUS_ONE(X),
    ORDERS(DSQ, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DMPR, DMPR),
    ADDRESS_PLUS_ONE(1_ROOTMU),
    ADDRESS_PLUS_ONE(X),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE(S_XI),
    SRR_X1(-7),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(T),
    ORDERS(SL1, VXSC),
    ADDRESS_PLUS_ONE(VVEC),
    ORDERS(VSL1, VAD), /* "PL AT 0" */
    ORDERS(VSL4, PUSH),
    ORDER(ABVAL),
    /* LAMENTER: LAMBERT comes in here with its own R2 in MPAC. The
     * velocity: V2VEC = UR1 (XI S(XI) - 1) X ROOTMU/R2 + (1 - X squared
     * C(XI)/R2) VVEC, in MPAC, R2VEC under it on the push-down list, and
     * RVQ returns to the caller. */
    /* LAMENTER */ ORDER(NORM),
    ADDRESS_PLUS_ONE(X1),
    STODL(R2),
    ADDRESS_PLUS_ONE(XI),
    ORDERS(DMP, DSU),
    ADDRESS_PLUS_ONE(S_XI),
    ADDRESS_PLUS_ONE(D1_128),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(ROOTMU),
    ORDERS(DMP, SLR_STAR),
    ADDRESS_PLUS_ONE(X),
    SLR_X1(-3),
    ORDERS(DDV, VXSC),
    ADDRESS_PLUS_ONE(R2),
    ADDRESS_PLUS_ONE(UR1),
    ORDERS(VSL1, PDDL), /* "6D=V2VEC PART (+15 OR 13)" */
    ADDRESS_PLUS_ONE(XSQC_XI),
    ORDERS(SLR_STAR, DDV),
    SLR_X1(-4),
    ADDRESS_PLUS_ONE(R2),
    ORDER(BDSU),
    ADDRESS_PLUS_ONE(D1_256),
    ORDERS(VXSC, VAD), /* "PL AT 6" */
    ADDRESS_PLUS_ONE(VVEC),
    ORDERS(VSL8, RVQ),
};

const struct interpretive_code NEWSTATE_CODE = {I_NEWSTATE >> 10, K_NEWSTATE, COUNT(newstate_words),
                                                newstate_words};
