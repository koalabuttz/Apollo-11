/* SPDX-License-Identifier: MIT */
/* CONIC_SUBROUTINES.agc, APSIDES and ABTCONIC (LM pp. 1200-1201, CM pp.
 * 1303-1304), in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs, and ABTCONIC's basic code.
 *
 * APSIDES: "GIVEN AN INITIAL STATE VECTOR CALCULATES THE RADIUS OF
 * PERICENTER AND OF APOCENTER AND THE ECCENTRICITY OF THE RESULTING CONIC
 * TRAJECTORY" (LM p. 1172, CM p. 1275). In: RVEC and VVEC, the state; X1,
 * the central body. Out: ECC; the apocenter in MPAC (the largest position,
 * LDPOSMAX, when there is none: a hyperbola, or one past the scaling) and
 * the pericenter at 0D.
 *
 * ABTCONIC: the conics' abort, alarm 607, through POODOO. */
#include "conics.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

static const word apsides_words[] = {
    /* APSIDES -- LM p. 1200 (CM p. 1303): the return to RTNAPSE, SETPD 0
     * and BOV +1 (which clears the overflow indicator), and PARAM for the
     * state. BOV GETECC clears the indicator again. */
    /* APSIDES */ ORDERS(STQ, SETPD), /* "PL AT 0" */
    ADDRESS(RTNAPSE),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(BOV),
    ADDRESS_AT(APSIDES, 5), /* (+1) */
    ORDERS(VLOAD, PDVL),    /* "PL AT 6" */
    ADDRESS_PLUS_ONE(RVEC),
    ADDRESS_PLUS_ONE(VVEC),
    ORDER(CALL),
    ADDRESS(PARAM),
    ORDER(BOV), /* "PL AT 0" */
    ADDRESS(GETECC),
    /* GETECC: ECC, the root of 1 - P R1A. The pericenter, P R1 over 1 +
     * ECC, pushed ("0D=RP"); the apocenter, twice R1 over R1A less it. An
     * overflow, or a negative apocenter, is INFINAPO; otherwise RVQ's
     * return through RTNAPSE. */
    /* GETECC */ ORDERS(DMP, SL4),
    ADDRESS_PLUS_ONE(R1A),
    ORDERS(BDSU, SQRT),
    ADDRESS_PLUS_ONE(D1_64),
    STORE(ECC),
    ORDERS(DAD, PDDL), /* "PL AT 2" */
    ADDRESS_PLUS_ONE(D1_8),
    ADDRESS_PLUS_ONE(R1),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE(P),
    ORDER(DDV),         /* "PL AT 0" */
    ORDERS(PDDL, NORM), /* "0D=RP (+29 OR +27) PL AT 2" */
    ADDRESS_PLUS_ONE(R1A),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(PDDL, SL_STAR), /* "PL AT 4" */
    ADDRESS_PLUS_ONE(R1),
    SL_X1(-5),
    ORDERS(DDV, DSU), /* "PL AT 2,0" */
    ORDERS(BOV, BMN),
    ADDRESS(INFINAPO),
    ADDRESS(INFINAPO),
    ORDER(GOTO),
    ADDRESS(RTNAPSE),
    /* INFINAPO: the apocenter is LDPOSMAX, and back. */
    /* INFINAPO */ ORDERS(DLOAD, GOTO), /* "RETURNS WITH APOAPSIS IN MPAC" */
    ADDRESS_PLUS_ONE(LDPOSMAX),
    ADDRESS(RTNAPSE),
    /* ABTCONIC -- LM p. 1201 (CM p. 1304): to the basic code. */
    /* ABTCONIC */ ORDER(EXIT),
};

const struct interpretive_code APSIDES_CODE = {I_APSIDES >> 10, K_APSIDES, COUNT(apsides_words),
                                               apsides_words};

/* After ABTCONIC's EXIT: TC POODOO, with the alarm code after it (OCT
 * 00607). POODOO does not return. */
void abtconic_after_exit(void)
{
    tc_to(ABTCONIC + 1u, POODOO); /* OCT 00607 */
}
