/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 801-805 (Luminary099 only):
 * "INITIALIZATION FOR THIS PASS" (GUILDRET), the "ROUTINES TO START NEW
 * PHASES" (P65START, STARTP64), "INCREMENT TTF/8, UPDATE LAND FOR LUNAR
 * ROTATION, DO OTHER USEFUL THINGS" (TTFINCR) and the "LANDING SITE
 * PERTURBATION EQUATIONS" (REDESIG). Each step that changes what a
 * restart would redo first calls FASTCHNG, so a restart resumes at the
 * word after it. The faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- GUILDRET -- pp. 801-802 ---- */

/* GUILDRET: RODCOUNT zero, and on. (A is +0: TS RODCOUNT never skips.) */
void guildret(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RODCOUNT));
    guildret_plus2();
}

/* GUILDRET +2: TPIPOLD gets TPIP, the last pass's time, and FASTCHNG. */
void guildret_plus2(void)
{
    dca(ADDRESS_OF(TPIP));
    dxch(ADDRESS_OF(TPIPOLD));
    tc_at(GUILDRET + 5u);
    fastchng();
}

/* FASTCHNG's return: TPIP gets this pass's (PIPTIME1), and TTF/8TMP
 * TTF/8. Then by FLPASS0 (CCS): on a pass after the phase's first,
 * TTFINCR; on the first (+0), BRSPOT1, the phase's NEWPHASE word. A
 * negative FLPASS0 skips BRSPOT1's INDEX (so BRAKQUAD's word, TTFINCR),
 * and -0 runs on into P65START. */
void guildret_after_fastchng(void)
{
    dca(ADDRESS_OF(PIPTIME1));
    dxch(ADDRESS_OF(TPIP));
    dca(ADDRESS_OF(TTF_8));
    dxch(ADDRESS_OF(TTF_8TMP));
    switch (ccs_at(ADDRESS_OF(FLPASS0))) {
    case POSITIVE: ttfincr(); return;
    case IS_PLUS_ZERO: tcf_by_phase(NEWPHASE, ADDRESS_OF(WCHPHASE)); return; /* BRSPOT1 */
    case NEGATIVE: tcf_table_word(NEWPHASE, NEWPHASE); return;
    case IS_MINUS_ZERO: p65start(); return;
    }
}

/* ---- The new phases -- p. 802 ---- */

/* P65START: NEWMODEX, the program P65 (DEC 65), ... */
void p65start(void)
{
    tc_to(P65START, NEWMODEX);
}

/* ... WCHVERT -2 (P65's guidance, in VERTGUID), and "PERMIT X-AXIS
 * OVERRIDE" (DOWNFLAG XOVINFLG). (A, a constant, never overflows: TS
 * WCHVERT does not skip.) */
void p65start_after_newmodex(void)
{
    set_A(negate(TWO));
    ts(ADDRESS_OF(WCHVERT));
    tc_to(P65START + 4u, DOWNFLAG);
}

/* DOWNFLAG's return: TTFINCR. */
void p65start_after_downflag(void)
{
    ttfincr();
}

/* STARTP64: NEWMODEX, the program P64 (DEC 64), ... */
void startp64(void)
{
    tc_to(STARTP64, NEWMODEX);
}

/* ... "AUGMENT TTF/8" (DELTTFAP to TTF/8TMP), "ENABLE RUPT10" (channel
 * 13's bit 12), and "INITIALIZE REDESIGNATION FLAG" (DOWNFLAG REDFLAG):
 * on to TTFINCR. */
void startp64_after_newmodex(void)
{
    ca(ADDRESS_OF(DELTTFAP));
    ads(ADDRESS_OF(TTF_8TMP));
    set_A(BIT12);
    wor_channel(CHAN13);
    tc_to(STARTP64 + 7u, DOWNFLAG);
}

/* ---- TTFINCR -- pp. 802-804 ---- */

/* TTFINCR. As the listing has it,
 *
 *     TTF/8 = TTF/8 + (TPIP - TPIPOLD)/8
 *     LAND = /LAND/ UNIT(LAND - LAND(TPIP - TPIPOLD) * WM)
 *     RANGEDSP = ABVAL(LAND - R)
 *
 * TTF/8 updated for the time since the last pass, the landing site for
 * the moon's rotation (WM), and the range to the site. Here the first
 * two: the interpretive code leaves the time in MPAC, the site in
 * LANDTEMP. */
void ttfincr(void)
{
    tc_at(TTFINCR);
    intpret();
}

static const word ttfincr_words[] = {
    /* TTFINCR +1 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TPIP),
    ADDRESS_PLUS_ONE(TPIPOLD),
    ORDERS(SLR, PUSH), /* "SHIFT SCALES DELTA TIME TO 2(17) CSECS" */
    SLR_COUNT(11),
    ORDERS(VXSC, VXV),
    ADDRESS_PLUS_ONE(LAND),
    ADDRESS_PLUS_ONE(WM),
    ORDERS(BVSU, RTB),
    ADDRESS_PLUS_ONE(LAND),
    ADDRESS(NORMUNIT),
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(_LAND_),
    STODL(LANDTEMP),
    ORDER(EXIT),
};

const struct interpretive_code TTFINCR_CODE = {031u, TTFINCR + 1u, COUNT(ttfincr_words),
                                               ttfincr_words};

/* After the EXIT: the time to TTF/8TMP ("NOW HAVE INCREMENTED TTF/8 IN
 * TTF/8TMP"), and FASTCHNG. */
void ttfincr_after_exit(void)
{
    dxch(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(TTF_8TMP));
    tc_at(TTFINCR + 022u);
    fastchng();
}

/* LANDTEMP's three double words to LAND. */
static void land_from_landtemp(void)
{
    unsigned i;

    for (i = 0; i < 6u; i += 2u) {
        dca(ADDRESS_OF(LANDTEMP) + i);
        dxch(ADDRESS_OF(LAND) + i);
    }
}

/* FASTCHNG's return: TTF/8 and LAND get the new ones, TDISPSET the
 * displays' (subroutines.c), and on: ... */
void ttfincr_after_fastchng(void)
{
    dca(ADDRESS_OF(TTF_8TMP));
    dxch(ADDRESS_OF(TTF_8));
    land_from_landtemp();
    tc_at(TTFINCR + 037u);
    tdispset();
    ttfincr_after_tdispset();
}

/* ... FASTCHNG, "SINCE REDESIG MAY CHANGE LANDTEMP", ... */
void ttfincr_after_tdispset(void)
{
    tc_at(TTFINCR + 040u);
    fastchng();
}

/* ... and BRSPOT2: the phase's PREGUIDE word. */
void brspot2(void)
{
    tcf_by_phase(PREGUIDE, ADDRESS_OF(WCHPHASE));
}

/* ---- REDESIG -- pp. 804-805 ---- */

/* REDESIG: "IS REDFLAG SET?" and "HAS TREDES REACHED ZERO?": if not set,
 * or if zero, "SKIP REDESIGNATION LOGIC" (RGVGCALC). If set and not yet
 * zero, with interrupts inhibited, ELINCR and AZINCR get the increments
 * the redesignator counted (ELINCR1, AZINCR1), and FASTCHNG. (A is a
 * word's, which never overflows: no TS skips.) */
void redesig(void)
{
    ca(ADDRESS_OF(FLAGWRD6));
    mask(REDFLBIT);
    if (A_is_zero()) {
        rgvgcalc();
        return;
    }
    ca(ADDRESS_OF(TREDES));
    if (A_is_zero()) {
        rgvgcalc();
        return;
    }
    inhint();
    ca(ADDRESS_OF(ELINCR1));
    ts(ADDRESS_OF(ELINCR));
    ca(ADDRESS_OF(AZINCR1));
    ts(ADDRESS_OF(AZINCR));
    tc_at(REDESIG + 014u);
    fastchng();
}

/* FASTCHNG's return: the counts zero (ELINCR1, AZINCR1) and the
 * increments' low words, the push-down list at its start ("SET PD TO
 * 0"), and the interpretive code. (No TS skips: A is +0, then FIXLOC.) */
void redesig_after_fastchng(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(ELINCR1));
    ts(ADDRESS_OF(AZINCR1));
    ts(ADDRESS_OF(ELINCR) + 1u);
    ts(ADDRESS_OF(AZINCR) + 1u);
    ca(ADDRESS_OF(FIXLOC));
    ts(ADDRESS_OF(PUSHLOC)); /* "SET PD TO 0" */
    tc_at(REDESIG + 024u);
    intpret();
}

/* The interpretive code: the line of sight, unit(LAND - R), and the
 * increments along the axes YNBPIP and its cross product with the line
 * of sight ("RESULTING VECTOR IS 1/2 REAL SIZE"), the site moved by
 * them, "MAKE SURE REDESIGNATION IS NOT TOO CLOSE TO THE HORIZON"
 * (DEPRCRIT), and REDES1: the new site, at LAND's length (/LAND/), to
 * LANDTEMP. */
static const word redesig_words[] = {
    /* REDESIG +025 */ ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(LAND),
    ADDRESS_PLUS_ONE(R),
    ORDERS(RTB, PUSH), /* "PUSH DOWN UNIT (LAND - R)" */
    ADDRESS(NORMUNIT),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(YNBPIP),
    ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(ELINCR),
    ADDRESS_PLUS_ONE(AZINCR),
    ORDERS(VXSC, VSU),
    ADDRESS_PLUS_ONE(YNBPIP),
    ORDERS(VAD, PUSH), /* "RESULTING VECTOR IS 1/2 REAL SIZE" */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(DEPRCRIT),
    ORDERS(BMN, DLOAD),
    ADDRESS(REDES1),
    ADDRESS_PLUS_ONE(DEPRCRIT),
    STORE_D(0),
    /* REDES1 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(LAND),
    ADDRESS_PLUS_ONE(R),
    ORDERS(DDV, VXSC),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE(R),
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(_LAND_),
    STORE(LANDTEMP),
    ORDER(EXIT), /* "LOOKANGL WILL BE COMPUTED AT RGVGCALC" */
};

const struct interpretive_code REDESIG_CODE = {031u, REDESIG + 025u, COUNT(redesig_words),
                                               redesig_words};

/* After the EXIT: FASTCHNG, ... */
void redes1_after_exit(void)
{
    tc_at(REDESIG + 064u);
    fastchng();
}

/* ... then LAND gets the new site, and RGVGCALC. */
void redes1_after_fastchng(void)
{
    land_from_landtemp();
    rgvgcalc();
}

#endif
