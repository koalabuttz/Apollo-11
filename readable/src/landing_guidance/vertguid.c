/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 815-816 (Luminary099 only):
 * "GUIDANCE FOR P65" and the vertical phase's other programs: WCHVERT
 * picks P65's (an acceleration from the velocity error), P66's (the rate
 * of descent mode, p66.c) or P67's (manual: the velocity only). The
 * faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* VERTGUID: WCHVERT "POSITIVE NON-ZERO ---> P67", +0 P66, negative P65.
 * Not modelled: WCHVERT -0 (P65's interpretive code would run as basic
 * code). */
void vertguid(void)
{
    switch (ccs_at(ADDRESS_OF(WCHVERT))) {
    case POSITIVE: p67vert(); return;
    case IS_PLUS_ZERO: p66vert(); return;
    case NEGATIVE: p65vert(); return;
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* P65VERT. As the listing has it,
 *
 *     ACG = (V2FG - VGU) / TAUVERT
 *
 * on to AFCCALC1, the thrust's computation. */
void p65vert(void)
{
    tc_at(P65VERT);
    intpret();
}

static const word p65vert_words[] = {
    /* P65VERT +1 */ ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(V2FG),
    ADDRESS_PLUS_ONE(VGU),
    ORDERS(V_SC, GOTO),
    ADDRESS_PLUS_ONE(TAUVERT),
    ADDRESS(AFCCALC1),
};

const struct interpretive_code P65VERT_CODE = {031u, P65VERT + 1u, COUNT(p65vert_words),
                                               p65vert_words};

/* P66VERT: "GUIDANCE FOR P66", P66VERTA in bank 32 (POSTJUMP). */
void p66vert(void)
{
    tc_to(P66VERT, POSTJUMP);
}

/* P67VERT: "TERMINATE GROUP 3." */
void p67vert(void)
{
    tc_to(P67VERT, PHASCHNG);
}

/* PHASCHNG's return: TC INTPRET. */
void p67vert_after_phaschng(void)
{
    tc_at(P67VERT + 2u);
    intpret();
}

/* The velocity, on to VHORCOMP (bank 32, p66.c). */
static const word p67vert_words[] = {
    /* P67VERT +3 */ ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE(V),
    ADDRESS(VHORCOMP),
};

const struct interpretive_code P67VERT_CODE = {031u, P67VERT + 3u, COUNT(p67vert_words),
                                               p67vert_words};

#endif
