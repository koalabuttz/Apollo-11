/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 867-868 (Luminary099 only, banks 23 and 33): NORMLIZE,
 * average G's first job, which puts the state vector in the units and
 * frame SERVICER works in and computes its gravity; and COPYCYC, which
 * copies each cycle's new state over the old. The faithful C is
 * c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* ---- NORMLIZE -- p. 867 (bank 23) ---- */

/* NORMLIZE: to the interpreter, for NORMLIZE_CODE. */
void normlize(void)
{
    tc_at(NORMLIZE);
    intpret();
}

/* With MUNFLAG clear, the gravity of the position RN1 (NORMLIZ1). With it
 * set, the landing's: R, RN1 in the stable member's frame (REFSMMAT), its
 * gravity (MUNGRAV); V, VN1 so too; then V(CSM) loaded and UHYP, the unit
 * normal to the CSM's orbit, from R(CSM) cross V(CSM). */
static const word normlize_words[] = {
    /* NORMLIZE +1 */ ORDERS(VLOAD, BOFF),
    ADDRESS_PLUS_ONE(RN1),
    SWITCH(BOFF, MUNFLAG),
    ADDRESS(NORMLIZ1),
    ORDERS(VSL6, MXV),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STCALL(R),
    ADDRESS(MUNGRAV),
    ORDERS(VLOAD, VSL1),
    ADDRESS_PLUS_ONE(VN1),
    ORDER(MXV),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(V),
    ADDRESS_PLUS_ONE(V_CSM),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(R_CSM),
    STORE(UHYP),
    /* ASCSPOT */ ORDER(EXIT),
};

const struct interpretive_code NORMLIZE_CODE = {023u, NORMLIZE + 1u, COUNT(normlize_words),
                                                normlize_words};

/* After ASCSPOT's EXIT: "MAKE SURE GROUP 2 IS OFF", and NORMLIZ2 in bank
 * 33 (POSTJUMP). */
void ascspot_after_exit(void)
{
    group_off(ADDRESS_OF(MINUS_PHASE2));
    tc_to(ASCSPOT + 4u, POSTJUMP); /* CADR NORMLIZ2 */
}

/* ---- NORMLIZ1, NORMLIZ2, COPYCYC -- pp. 867-868 (bank 33) ---- */

/* NORMLIZ1: the gravity (CALCGRAV), and EXIT to NORMLIZ2. */
static const word normliz1_words[] = {
    /* NORMLIZ1 */ ORDER(CALL),
    ADDRESS(CALCGRAV),
    ORDER(EXIT),
};

const struct interpretive_code NORMLIZ1_CODE = {033u, NORMLIZ1, COUNT(normliz1_words),
                                                normliz1_words};

/* NORMLIZ2: the state copied into the old one, 18 words: "DO NOT COPY MASS
 * IN NORMLIZE" (TC COPYCYC +1); ... */
void normliz2(void)
{
    set_A(EIGHTEEN);
    tc_to(NORMLIZ2 + 1u, COPYCYC + 1u);
}

/* ... and the job ends. */
void normliz2_after_copycyc(void)
{
    tc_to(NORMLIZ2 + 2u, ENDOFJOB);
}

/* COPYCYC +1: the new state (RN1 on, 20 words with the mass) over the
 * old (RN on), two words at a time from the end, interrupts held: the
 * count in A "REDUCE BY 1 IF ODD" (MASK NEG1) into ITEMP1, the words at
 * ITEMP1 copied (INDEX ITEMP1; DCA RN1; INDEX ITEMP1; DXCH RN), and again
 * while CCS ITEMP1 is positive, with the count less one. At +0 back,
 * "RETURN UNDER INHINT". (The counts the program gives, 20 and 18, are
 * even: ITEMP1 is never negative.) */
void copycyc_plus_1(void)
{
    inhint();
    do {
        mask(NEG1);
        ts(ADDRESS_OF(ITEMP1));
        dca(indexed(ADDRESS_OF(RN1), fetch(ADDRESS_OF(ITEMP1))));
        dxch(indexed(ADDRESS_OF(RN), fetch(ADDRESS_OF(ITEMP1))));
    } while (ccs_at(ADDRESS_OF(ITEMP1)) == POSITIVE);
    return_to_q();
}

void copycyc(void)
{
    set_A(OCT24); /* "DEC 20" */
    copycyc_plus_1();
}

#endif
