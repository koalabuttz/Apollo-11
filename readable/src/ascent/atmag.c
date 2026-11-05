/* SPDX-License-Identifier: MIT */
/* ASCENT_GUIDANCE.agc, pp. 843-844 (Luminary099 only, bank 34): ATMAG,
 * the start of each ascent guidance cycle, where SERVICER goes after its
 * two-second update of the state (AVEGEXIT holds ATMAG's 2CADR). The
 * faithful C is c64-port/src/lm_ascent.c. */
#include "ascent.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ATMAG: restart group 5 at 5.3 (PHASCHNG, OCT 00035), ... */
void atmag(void)
{
    tc_to(ATMAG, PHASCHNG);
}

/* ... and the interpretive code (ATMAG_CODE). */
void atmag_after_phaschng(void)
{
    tc_at(ATMAG + 2u);
    intpret();
}

/* With FLRCS set (the RCS trims the burn's end), straight to ASCENT. Else
 * if the velocity gain this cycle (ABDVCONV) is less than MINABDV (the
 * engine is not yet thrusting), SURFFLAG clear and ASCTERM4: no guidance
 * this cycle. Else RENDWFLG clear, and 1/DV, BIT3H over ABDVCONV, for the
 * basic code after the EXIT. */
static const word atmag_words[] = {
    /* ATMAG +3 */ ORDER(BON),
    SWITCH(BON, FLRCS), /* the RCS trim: */
    ADDRESS(ASCENT),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(ABDVCONV), /* this cycle's velocity gain */
    ADDRESS_PLUS_ONE(MINABDV),
    ORDERS(BMN, CLEAR),
    ADDRESS(ASCTERM4), /* no guidance this cycle */
    SWITCH(CLEAR, SURFFLAG),
    ORDERS(CLEAR, SLOAD),
    SWITCH(CLEAR, RENDWFLG),
    ADDRESS_PLUS_ONE(BIT3H),
    ORDERS(DDV, EXIT), /* 1/DV */
    ADDRESS_PLUS_ONE(ABDVCONV),
};

const struct interpretive_code ATMAG_CODE = {034u, ATMAG + 3u, COUNT(atmag_words), atmag_words};

/* After the EXIT, the four latest 1/DV move along: the new one (in MPAC)
 * to 1/DV3, 1/DV3 to 1/DV2, 1/DV2 to 1/DV1 and 1/DV1 to 1/DV0 (MASS1's
 * words), the oldest dropped. Then the interpretive code again
 * (ATMAG_AFTER_EXIT_CODE). */
void atmag_after_exit(void)
{
    dxch(MPAC_AT(0u));
    dxch(ADDRESS_OF(1_DV3));
    dxch(ADDRESS_OF(1_DV2));
    dxch(ADDRESS_OF(1_DV1));
    dxch(ADDRESS_OF(MASS1)); /* 1/DV0 = MASS1 */
    tc_at(ATMAG + 026u);
    intpret();
}

/* TBUP, the burn-up time (the time to burn the whole mass at this
 * thrust), from the four 1/DV, VE (the exhaust velocity) and the two
 * seconds between cycles, with half its old value pushed in and 6
 * seconds (6SEC(18)) taken off:
 *
 *     TBUP = VE 2SEC (1/DV0 + 1/DV1 + 1/DV2 + 1/DV3) + TBUP / 2 - 6SEC
 *
 * (each term at its scale); then AT, the thrust acceleration, VE / TBUP,
 * and ASCENT. */
static const word atmag_after_exit_words[] = {
    /* ATMAG +027 */ ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(MASS1), /* 1/DV0 */
    ADDRESS_PLUS_ONE(1_DV1),
    ORDERS(DAD, DAD),
    ADDRESS_PLUS_ONE(1_DV2),
    ADDRESS_PLUS_ONE(1_DV3),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(VE),
    ADDRESS_PLUS_ONE(2SEC_9),
    ORDERS(SL3, PDDL),
    ADDRESS_PLUS_ONE(TBUP),
    ORDERS(SR1, DAD),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(6SEC_18),
    STODL(TBUP),
    ADDRESS_PLUS_ONE(VE),
    ORDERS(SR1, DDV),
    ADDRESS_PLUS_ONE(TBUP),
    STCALL(AT),
    ADDRESS(ASCENT),
};

const struct interpretive_code ATMAG_AFTER_EXIT_CODE = {
    034u, ATMAG + 027u, COUNT(atmag_after_exit_words), atmag_after_exit_words};

#endif
