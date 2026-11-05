/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 808-811 (Luminary099 only):
 * the "MAIN GUIDANCE EQUATION" (QUADGUID), the thrust it commands
 * (AFCCALC1-AFCCLEND), "ERECT GUIDANCE-STABLE MEMBER TRANSFORMATION
 * MATRIX" (CGCALC) and "PREPARE TO EXIT" (EXTLOGIC). The faithful C is
 * c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- QUADGUID -- pp. 808-809 ---- */

/* QUADGUID. As the listing has it,
 *
 *     ACG = ADG + 6(VDG + VG)/TTF + 12(RDG - RG)/(TTF)(TTF)
 *
 * "AS PUBLISHED", and as programmed, with TTF/8 for TTF, VGU and RGU the
 * velocity and position in guidance coordinates,
 *
 *     ACG = 3/4 (1/4(RDG - RG)/(TTF/8) + VDG + VG)/(TTF/8) + ADG
 *
 * with each term weighted by the lag LEADTIME ("A NEGATIVE NUMBER") puts
 * on the command. The weights come from the ratio of the lag-diminished
 * time to TTF/8 ("- RATIO OF LAG-DIMINISHED TTF TO TTF", BUF): MPAC's for
 * VGU, 26D's for RDG - RGU, 28D's for VDG and 30D's for ADG, "BY FORCING
 * -TTF*LEADTIME > OR = ZERO" (the CS L, AD L pair keeps the sum's sign
 * and limits it). Then the phase's index to X1 (INTPRETX) and the
 * interpretive code.
 *
 * A TS that overflows skips the word after it and leaves +1 or -1 in A,
 * which goes on from there: TS L, MPAC, 26D, 28D and 30D can. (DV and
 * SQUARE leave no overflow, nor do +1, -1 and +0: TS BUF, BUF +1 and MODE
 * never skip.) */
void quadguid(void)
{
    cs(ADDRESS_OF(TTF_8));
    ad_at(ADDRESS_OF(LEADTIME)); /* "LEADTIME IS A NEGATIVE NUMBER" */
    ad(POSMAX);                  /* "SAFEGUARD THE COMPUTATIONS THAT FOLLOW" */
    if (!ts(L_ADDRESS)) cs(L_ADDRESS);
    ad_at(L_ADDRESS);
    zl();
    dv_at(ADDRESS_OF(TTF_8));
    ts(ADDRESS_OF(BUF)); /* "- RATIO OF LAG-DIMINISHED TTF TO TTF" */
    square();
    ts(ADDRESS_OF(BUF) + 1u);
    ad_at(ADDRESS_OF(BUF));
    xch(ADDRESS_OF(BUF) + 1u); /* "RATIO SQUARED - RATIO" */
    ad_at(ADDRESS_OF(BUF) + 1u);
    if (!ts(ADDRESS_OF(MPAC))) /* "COEFFICIENT FOR VGU TERM" */
        ad_at(ADDRESS_OF(BUF) + 1u);
    if (!ts(FIXLOC + 26u)) /* "COEFFICIENT FOR RDG-RGU TERM" */
        ad_at(ADDRESS_OF(BUF) + 1u);
    if (!ts(FIXLOC + 28u)) /* "COEFFICIENT FOR VDG TERM" */
        ad_at(ADDRESS_OF(BUF));
    ad(POSMAX);
    ad_at(ADDRESS_OF(BUF) + 1u);
    ad_at(ADDRESS_OF(BUF) + 1u);
    if (!ts(FIXLOC + 30u)) /* "COEFFICIENT FOR ADG TERM" */
        set_A(ZERO);
    ts(ADDRESS_OF(MODE));
    tc_at(QUADGUID + 037u);
    intpretx();
}

/* The interpretive code: ACG (the push-down list's terms summed, through
 * the X1 index the phase's target parameters), then AFCCALC1, "VERGUID
 * COMES HERE": CG times ACG less the gravity term GDT/2 is UNFC/2, the
 * thrust's direction ("UNFC/2 NEED NOT BE UNITIZED"), and its length
 * /AFC/, "MAGNITUDE OF AFC FOR THROTTLE". AFCCALC2: the horizontal
 * acceleration it leaves at the highest thrust, from the vertical and
 * out-of-plane ones, "AMAXHORIZ = SQRT(ATOTAL - A - A)" (squared terms),
 * or zero (ZEROVECS) if they take all of it, limits UNFC/2's third
 * component, and EXIT. */
static const word quadguid_words[] = {
    /* QUADGUID +040 */ ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(VGU),
    ADDRESS_PLUS_ONE_D(28),
    ORDERS(VXSC_STAR, PDVL_STAR),
    ADDRESS_PLUS_ONE_AT_X1(VDG, 0),
    ADDRESS_PLUS_ONE_AT_X1(RDG, 0),
    ORDERS(VSU, V_SC),
    ADDRESS_PLUS_ONE(RGU),
    ADDRESS_PLUS_ONE(TTF_8),
    ORDERS(VSR2, VXSC),
    ADDRESS_PLUS_ONE_D(26),
    ORDERS(VAD, VAD),
    ORDERS(V_SC, VXSC),
    ADDRESS_PLUS_ONE(TTF_8),
    ADDRESS_PLUS_ONE(3_4DP),
    ORDERS(PDDL, VXSC_STAR),
    ADDRESS_PLUS_ONE_D(30),
    ADDRESS_PLUS_ONE_AT_X1(ADG, 0),
    ORDER(VAD),
    /* AFCCALC1 */ ORDERS(VXM, VSL1), /* "VERGUID COMES HERE" */
    ADDRESS_PLUS_ONE(CG),
    ORDERS(PDVL, V_SC),
    ADDRESS_PLUS_ONE(GDT_2),
    ADDRESS_PLUS_ONE(GSCALE),
    ORDERS(BVSU, STADR),
    AFTER_STADR(STORE(UNFC_2)), /* "UNFC/2 NEED NOT BE UNITIZED" */
    ORDER(ABVAL),
    /* AFCCALC2 */ STODL(_AFC_), /* "MAGNITUDE OF AFC FOR THROTTLE" */
    ADDRESS_PLUS_ONE(UNFC_2),    /* "VERTICAL COMPONENT" */
    ORDERS(DSQ, PDDL),
    ADDRESS_PLUS_ONE_AT(UNFC_2, 2), /* "OUT-OF-PLANE" */
    ORDERS(DSQ, PDDL),
    ADDRESS_PLUS_ONE(HIGHESTF),
    ORDERS(DDV, DSQ),
    ADDRESS_PLUS_ONE(MASS),
    ORDERS(DSU, DSU), /* "AMAXHORIZ = SQRT(ATOTAL - A - A)" */
    ORDERS(BPL, DLOAD),
    ADDRESS(AFCCALC3),
    ADDRESS_PLUS_ONE(ZEROVECS),
    /* AFCCALC3 */ ORDERS(SQRT, DAD),
    ADDRESS_PLUS_ONE_AT(UNFC_2, 4),
    ORDERS(BPL, BDSU),
    ADDRESS(AFCCLEND),
    ADDRESS_PLUS_ONE_AT(UNFC_2, 4),
    STORE_AT(UNFC_2, 4),
    /* AFCCLEND */ ORDER(EXIT),
};

const struct interpretive_code QUADGUID_CODE = {031u, QUADGUID + 040u, COUNT(quadguid_words),
                                                quadguid_words};

/* AFCCLEND's EXIT +1: TC FASTCHNG, restart group 3 to resume at the word
 * after. */
void afcclend_after_exit(void)
{
    tc_at(AFCCLEND + 1u);
    fastchng();
}

/* FASTCHNG's return: "PREPARE FOR PHASE SWITCHING LOGIC" (WCHPHOLD gets
 * WCHPHASE) and "INCREMENT PASS COUNTER" (FLPASS0), then BRSPOT4. */
void afcclend_after_fastchng(void)
{
    ca(ADDRESS_OF(WCHPHASE));
    ts(ADDRESS_OF(WCHPHOLD));
    incr(ADDRESS_OF(FLPASS0));
    brspot4();
}

/* BRSPOT4: the phase's AFTRGUID word. */
void brspot4(void)
{
    tcf_by_phase(AFTRGUID, ADDRESS_OF(WCHPHASE));
}

/* ---- CGCALC -- pp. 810-811 ---- */

/* CGCALC: the phase's target time (EXTEND; INDEX WCHPHASE; INDEX
 * TARGTDEX; DCA TCGFBRAK: TCGFBRAK's double word, or TCGFAPPR's) in
 * E-bank 5, then TTF/8's E-bank back (INCR BBANK twice: 5 to 7). With A
 * its high word plus TTF/8 and L its low word, CCS A and CCS L: CG wants
 * erecting only while the sum is negative. Then INTPRETX and the
 * interpretive code. The phase is -1, 0 or 1 here (AFTRGUID's words
 * for the others are not CGCALC). Not modelled: -0, which AFTRGUID takes
 * as BRAKQUAD. (A is EBANK5, a constant: TS EBANK never skips.) */
void cgcalc(void)
{
    word index, instruction;

    set_A(EBANK5);
    ts(EBANK_ADDRESS);
    if (fetch(ADDRESS_OF(WCHPHASE)) == MINUS_ZERO) not_modelled();
    index = indexed_instruction((word)(050000u | TARGTDEX), fetch(ADDRESS_OF(WCHPHASE)));
    instruction = indexed_instruction((word)(030000u + ADDRESS_OF(TCGFBRAK) + 1u),
                                      read_word(ADDRESS_FIELD(index)));
    if (OPERATION_CODE(instruction) != 3u || ADDRESS_FIELD(instruction) == 0u ||
        ADDRESS_FIELD(instruction) > 02000u)
        not_modelled();
    dca(ADDRESS_FIELD(instruction) - 1u);
    incr(BBANK_ADDRESS);
    incr(BBANK_ADDRESS);
    ad_at(ADDRESS_OF(TTF_8));
    xch(L_ADDRESS);
    ad_at(ADDRESS_OF(TTF_8));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        switch (ccs_at(L_ADDRESS)) {
        case POSITIVE:
        case IS_PLUS_ZERO: extlogic(); return;
        case NEGATIVE:
        case IS_MINUS_ZERO: break;
        }
        break;
    case IS_PLUS_ZERO:
    case NEGATIVE: extlogic(); return;
    case IS_MINUS_ZERO: break; /* NOOP */
    }
    tc_at(CGCALC + 020u);
    intpretx();
}

/* The interpretive code: CG's rows. The first, the unit vector to the
 * landing site (LAND); the second, unit(the site less R, offset by
 * GAINBRAK (the phase's: "NUMERO MYSTERIOSO") times ANGTERM) crossed with
 * LAND, "SECOND ROW"; and the third, the first crossed with the second. */
static const word cgcalc_words[] = {
    /* CGCALC +021 */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(LAND),
    STODL(CG),
    ADDRESS_PLUS_ONE(TTF_8),
    ORDERS(DMP_STAR, VXSC),
    ADDRESS_PLUS_ONE_AT_X1(GAINBRAK, 0), /* "NUMERO MYSTERIOSO" */
    ADDRESS_PLUS_ONE(ANGTERM),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(LAND),
    ORDERS(VSU, RTB),
    ADDRESS_PLUS_ONE(R),
    ADDRESS(NORMUNIT),
    ORDERS(VXV, RTB),
    ADDRESS_PLUS_ONE(LAND),
    ADDRESS(NORMUNIT),
    STOVL_AT(CG, 6), /* "SECOND ROW" */
    ADDRESS_PLUS_ONE(CG),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE_AT(CG, 6),
    STORE_AT(CG, 014),
    ORDER(EXIT),
};

const struct interpretive_code CGCALC_CODE = {031u, CGCALC + 021u, COUNT(cgcalc_words),
                                              cgcalc_words};

/* ---- EXTLOGIC -- p. 811 ---- */

/* EXTLOGIC: "DECIDE (1) HOW TO EXIT, AND (2) WHETHER TO SWITCH PHASES":
 * the phase's end time (TENDBRAK, or TENDAPPR: "WCHPHASE = 1 APPRQUAD")
 * plus TTF/8, then EXSPOT1. */
void extlogic(void)
{
    ca_indexed(ADDRESS_OF(WCHPHASE), ADDRESS_OF(TENDBRAK));
    ad_at(ADDRESS_OF(TTF_8));
    exspot1();
}

/* EXSPOT1: while the time is not yet past the phase's end (zero or less)
 * the phase's WHATEXIT word; else FASTCHNG and on to the next phase. The
 * AGC adds the index to the whole BZMF; not modelled: an index that makes
 * it another instruction. */
void exspot1(void)
{
    word instruction = indexed_instruction((word)(060000u | WHATEXIT), fetch(ADDRESS_OF(WCHPHASE)));

    if (OPERATION_CODE(instruction) != 6u) not_modelled();
    if (A_is_zero_or_minus()) {
        tcf_table_word(WHATEXIT, ADDRESS_FIELD(instruction));
        return;
    }
    tc_at(EXSPOT1 + 3u);
    fastchng();
}

/* FASTCHNG's return: the next phase (WCHPHOLD +1 to WCHPHASE), "RESET
 * FLPASS0", and the old phase's WHATEXIT word. A TS WCHPHASE that
 * overflows skips the CA ZERO and leaves +1 in A, which FLPASS0 gets.
 * (So TS FLPASS0 never skips.) */
void exspot1_after_fastchng(void)
{
    ca(ADDRESS_OF(WCHPHOLD));
    ad(ONE);
    if (!ts(ADDRESS_OF(WCHPHASE))) set_A(ZERO);
    ts(ADDRESS_OF(FLPASS0)); /* "RESET FLPASS0" */
    tcf_by_phase(WHATEXIT, ADDRESS_OF(WCHPHOLD));
}

#endif
