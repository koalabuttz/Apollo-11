/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, TIMERAD (LM pp. 1198-1199, CM pp. 1301-1302):
 * TIME-RADIUS, in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs.
 *
 * "GIVEN AN INITIAL STATE VECTOR AND A DESIRED RADIUS TO WHICH THE STATE
 * IS TO BE UPDATED ALONG A CONIC TRAJECTORY, CALCULATES THE CORRESPONDING
 * TIME-OF-FLIGHT" (LM p. 1169, CM p. 1272), and the new state as well
 * unless RVSW is set. In: RVEC and VVEC, the state; RDESIRED, the radius;
 * SGNRDOT, the sign of the radial velocity wanted there; X1, the central
 * body. Past an apsis (RDESIRED beyond the apocenter or below the
 * pericenter) APSESW is set and the apsis is the answer.
 *
 * PARAM gives P, R1A and COGA; from them the eccentricity vector, and the
 * cosine of the true anomaly at RDESIRED (COSF). The direction there,
 * from the conic's plane (UN), gives the angle to it, SNTH and CSTH; then
 * GETX and TIME-THETA's own COMMNOUT, DELTIME and NEWSTATE. */
#include "conics.h"
#include "../agc/names.h"

static const word timerad_words[] = {
    /* TIMERAD -- LM p. 1198 (CM p. 1301): the return to RTNTR, SETPD 0
     * and BOV +1 (which clears the overflow indicator), and PARAM for the
     * state, as TIME-THETA starts (an overflow is COGAOVFL). The
     * eccentricity vector from R1A, P, COGA and the unit vectors of the
     * position and velocity (UR1, U2): its unit vector pushed, its length
     * at 36D. With none (a circle), CIRCULAR. */
    /* TIMERAD */ ORDERS(STQ, SETPD), /* "PL AT 0" */
    ADDRESS(RTNTR),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(BOV),
    ADDRESS_AT(TIMERAD, 5), /* (+1) */
    ORDERS(VLOAD, PDVL),    /* "PL AT 6" */
    ADDRESS_PLUS_ONE(RVEC),
    ADDRESS_PLUS_ONE(VVEC),
    ORDER(CALL),
    ADDRESS(PARAM),
#if LUMINARY099
    ORDERS(BOV, DLOAD), /* "PL AT 0" */
    ADDRESS(COGAOVFL),
#else
    ORDERS(BOV, DLOAD),
    ADDRESS(COGAOVFL), /* "PL AT 0" */
#endif
    ADDRESS_PLUS_ONE(D1_32),
    ORDERS(DSU, DMP),
    ADDRESS_PLUS_ONE(R1A),
    ADDRESS_PLUS_ONE(P),
    ORDERS(SQRT, DMP),
    ADDRESS_PLUS_ONE(COGA),
    ORDERS(SL4, VXSC),
    ADDRESS_PLUS_ONE(U2),
    ORDERS(PDDL, DSU), /* "PL AT 6" */
    ADDRESS_PLUS_ONE(D1_64),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(VXSC, VSU), /* "PL AT 0" */
    ADDRESS_PLUS_ONE(UR1),
    ORDERS(VSL4, UNIT),
    ORDER(BOV),
    ADDRESS(CIRCULAR),
    /* COSF, the cosine of the true anomaly at RDESIRED: (P R1/RDESIRED
     * - 1) over the eccentricity. An overflow, or a cosine past one, is
     * BADR2. Otherwise the sine, with SGNRDOT's sign; APSESW clear. */
    ORDERS(PDDL, NORM),         /* "0D=UNIT(ECC) (+3) PL AT 6" */
    ADDRESS_PLUS_ONE(RDESIRED), /* "35D=ECC (+3)" */
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, DMP), /* "PL AT 8" */
    ADDRESS_PLUS_ONE(R1),
    ADDRESS_PLUS_ONE(P),
    ORDERS(SL_STAR, DDV), /* "PL AT 6" */
    SL_X1(0),
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE(D1_16),
    ADDRESS_PLUS_ONE_D(36), /* "36D=ECC (+3)" */
    STORE(COSF),
    ORDERS(BOV, DSQ),
    ADDRESS(BADR2),
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE(D1_4),
    ADDRESS(BADR2),
    ORDERS(SQRT, SIGN),
    ADDRESS_PLUS_ONE(SGNRDOT),
    ORDER(CLEAR),
    SWITCH(CLEAR, APSESW),
    /* TERMNVEC -- LM p. 1199 (CM p. 1302): the unit vector to the point
     * at RDESIRED, from the eccentricity's direction, the sine and COSF,
     * pushed ("0D=U2"). CSTH, its cosine with UR1 (limited by the DDV and
     * shifted back, BOV +1 clearing the overflow indicator), and SNTH, the
     * sine along UN; with P, GETX, and SOLNSW cleared on to COMMNOUT. */
    /* TERMNVEC */ ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(UN),
    ORDERS(VXV, PDVL), /* "VXCH WITH 0D PL AT 0,6" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VXSC, VAD), /* "PL AT 0" */
    ADDRESS_PLUS_ONE(COSF),
    ORDERS(VSL1, PUSH), /* "0D=U2 PL AT 6" */
    ORDERS(DOT, DDV),   /* "LIMITS RESULT TO POSMAX OR NEGMAX" */
    ADDRESS_PLUS_ONE(UR1),
    ADDRESS_PLUS_ONE(DP1_4),
    ORDERS(SR1, BOV),          /* "SCALE BACK DOWN TO NORMAL" */
    ADDRESS_AT(TERMNVEC, 014), /* (+1): "CLEAR OVFIND IF SET" */
    STOVL(CSTH),               /* "CSTH (+1)" */
    ADDRESS_PLUS_ONE(UR1),
    ORDERS(VXV, VSL1),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UN),
    STODL(SNTH), /* "SNTH (+1)" */
    ADDRESS_PLUS_ONE(P),
    ORDER(CALL),
    ADDRESS(GETX),
    ORDER(CLRGO),
    SWITCH(CLRGO, SOLNSW),
    ADDRESS(COMMNOUT),
    /* CIRCULAR: no eccentricity, no answer: SOLNSW set, and ABTCONIC. */
    /* CIRCULAR */ ORDERS(SETPD, SETGO),
    ADDRESS_PLUS_ONE_D(0),
    SWITCH(SETGO, SOLNSW),
    ADDRESS(ABTCONIC),
    /* BADR2: RDESIRED is past an apsis. COSF is one with its own sign
     * (LODPHALF), the sine zero (KEPZERO), APSESW set, and TERMNVEC. */
    /* BADR2 */ ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE(LODPHALF),
    ADDRESS_PLUS_ONE(COSF),
    STODL(COSF),
    ADDRESS_PLUS_ONE(KEPZERO),
    ORDER(SETGO),
    SWITCH(SETGO, APSESW),
    ADDRESS(TERMNVEC),
};

const struct interpretive_code TIMERAD_CODE = {I_TIMERAD >> 10, K_TIMERAD, COUNT(timerad_words),
                                               timerad_words};
