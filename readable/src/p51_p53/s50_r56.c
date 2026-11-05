/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 2 (Luminary099 pp. 934-940, bank 14): S50 and R56,
 * the stars to sight.
 *
 * S50, alias LOCSAM, "COMPUTE INPUTS FOR PICAPAR AND PLANET": the unit
 * vectors to the sun, the earth and the moon from the LM at a time
 * (TSIGHT), and the cosine of each body's half angle as the LM sees it,
 * which a star must clear (CSUN, CEARTH, CMOON in the push-down list).
 * R56, alias PICAPAR, picks the pair of stars to sight: "EACH STAR IN THE
 * CATALOG IS TESTED" against the field of view about the shaft axis (SAX)
 * and occultation by the bodies (OCCULT), each one that passes is paired
 * with every star of lower index, and of the pairs far enough apart the
 * one with the largest separation is chosen (BESTI, BESTJ). VFLAG set
 * means no pair was found: R56 returns to its caller's +2 then, its +3
 * otherwise.
 *
 * Most of it is interpretive code. The basic code: PICAPAR's entry
 * (MAKECADR keeps the caller's return in QMIN), and its two ends,
 * PICBXT, which returns through QMIN (+1 when a pair was found). */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- S50 (LOCSAM) -- pp. 934-936 ---- */

/* LOCSAM: the time in MPAC, kept in TSIGHT; the sun's and the moon's
 * directions from the earth then (LSPOS: VSUN, VMOON), and the LM's
 * position (LEMPREC, at TDEC1). Then, by the sphere of influence the LM
 * is in (S2 +0 and TIX,2: X2, which LEMPREC leaves +0 in the earth's and
 * 2 in the moon's), the vectors from the earth's centre (EARTCNTR) or
 * the moon's (MOONCNTR). The cosines (CEARTH, CSUN, CMOON) go to the
 * push-down list's 14D, 16D and 18D. OCCOS: the cosine of a body's half
 * angle plus five degrees, from its radius (MPAC) over its distance (36D,
 * where UNIT leaves the length). */
static const word locsam_words[] = {
    /* LOCSAM */ ORDER(STQ),
    ADDRESS(QMIN),
    STCALL(TSIGHT),
    ADDRESS(LSPOS),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TSIGHT),
    STCALL(TDEC1),
    ADDRESS(LEMPREC),
    ORDERS(SSP, TIX_2),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(0),
    ADDRESS(MOONCNTR),
    /* EARTCNTR */ ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(VMOON),
    ADDRESS_PLUS_ONE(RSUBEM),
    ORDERS(VSL1, VSU),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(UNIT),
    STOVL(VMOON),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, VCOMP),
    STODL(VEARTH),
    ADDRESS_PLUS_ONE(RSUBE),
    ORDER(CALL),
    ADDRESS(OCCOS),
    STODL_D(14), /* CEARTH */
    ADDRESS_PLUS_ONE(CSS5),
    STCALL_D(18), /* CMOON */
    ADDRESS(ENDSAM),
    /* MOONCNTR */ ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(VMOON),
    ADDRESS_PLUS_ONE(ROE),
    ORDERS(BVSU, UNIT),
    ADDRESS_PLUS_ONE(VSUN),
    STOVL(VSUN),
    ADDRESS_PLUS_ONE(VMOON),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(RSUBEM),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, VCOMP),
    STOVL(VEARTH),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, VCOMP),
    STODL(VMOON),
    ADDRESS_PLUS_ONE(RSUBM),
    ORDER(CALL),
    ADDRESS(OCCOS),
    STODL_D(18), /* CMOON */
    ADDRESS_PLUS_ONE(CSS5),
    STORE_D(14), /* CEARTH */
    /* ENDSAM */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(CSSUN),
    STORE_D(16), /* CSUN */
    ORDER(GOTO),
    ADDRESS(QMIN),
    /* OCCOS */ ORDERS(DDV, SR1),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(ASIN, DAD),
    ADDRESS_PLUS_ONE(5DEGREES),
    ORDERS(COS, SR1),
    ORDER(RVQ),
};

const struct interpretive_code LOCSAM_CODE = {014u, LOCSAM, COUNT(locsam_words), locsam_words};

/* ---- R56 (PICAPAR) -- pp. 937-940 ---- */

static void picbxt(void);

/* PICAPAR: the caller's return, as a CADR, to QMIN. */
void picapar(void)
{
    tc_to(PICAPAR, MAKECADR);
}

void picapar_after_makecadr(void)
{
    if (ts(ADDRESS_OF(QMIN))) not_modelled(); /* (A as MAKECADR leaves it does not overflow) */
    tc_at(PICAPAR + 2u);
    intpret();
}

/* The shaft axis in the reference frame (SAX), from the navigation base
 * axes (CDUTRIG, CALCSMSC): VFLAG set and BESTI and BESTJ zero to start.
 * X1 counts the major star down from the catalog's end (PIC1, "37 X 6 +
 * 6"), X2 the minor one from the major's (PIC3); each must lie within 50
 * degrees of SAX (CSS33), the pair more than 50 degrees apart (CSS40),
 * and neither occulted (OCCULT, CULTFLAG). STRATGY keeps the
 * first pair found (NEWPAR) and each pair after it farther apart than the
 * best so far: STRAT compares the two (VFLAG inverted, so that it runs
 * twice, swapping the pairs with XCHX between). OCCULT: whether a body
 * hides the star (MPAC), the star being within the body's half angle
 * (CULTRIX, the bodies' unit vectors; CSS, the cosines). */
static const word picapar_words[] = {
    /* PICAPAR +3 */ ORDER(CALL),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(CALCSMSC),
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SET, DLOAD), /* "VFLAG = 1" */
    SWITCH(SET, VFLAG),
    ADDRESS_PLUS_ONE(DPZERO),
    STOVL(BESTI),
    ADDRESS_PLUS_ONE(XNB),
    ORDERS(VXSC, PDVL),
    ADDRESS_PLUS_ONE(HALFDP),
    ADDRESS_PLUS_ONE(ZNB),
    ORDERS(AXT_1, VXSC),
    NUMBER(228), /* "X1 = 37 X 6 + 6" */
    ADDRESS_PLUS_ONE(HALFDP),
    ORDER(VAD),
    ORDERS(VXM, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(SAX),       /* "SAX = SHAFT AXIS" */
    ORDERS(SSP, SSP), /* "S1 = S2 = 6" */
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(6),
    /* PIC1 */ ORDERS(TIX_1, GOTO), /* "MAJOR STAR" */
    ADDRESS(PIC2),
    ADDRESS(PICEND),
    /* PIC2 */ ORDERS(VLOAD_STAR, DOT),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS_PLUS_ONE(SAX),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(CSS33),
    ADDRESS(PIC1),
    ORDER(LXA_2),
    ADDRESS(X1),
    /* PIC3 */ ORDERS(TIX_2, GOTO),
    ADDRESS(PIC4),
    ADDRESS(PIC1),
    /* PIC4 */ ORDERS(VLOAD_STAR, DOT),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ADDRESS_PLUS_ONE(SAX),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(CSS33),
    ADDRESS(PIC3),
    ORDERS(VLOAD_STAR, DOT_STAR),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(CSS40),
    ADDRESS(PIC3),
    ORDERS(VLOAD_STAR, CALL),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS(OCCULT),
    ORDER(BON),
    SWITCH(BON, CULTFLAG),
    ADDRESS(PIC1),
    ORDERS(VLOAD_STAR, CALL),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ADDRESS(OCCULT),
    ORDER(BON),
    SWITCH(BON, CULTFLAG),
    ADDRESS(PIC3),
    /* STRATGY */ ORDER(BONCLR),
    SWITCH(BONCLR, VFLAG),
    ADDRESS(NEWPAR),
    ORDERS(XCHX_1, XCHX_2),
    ADDRESS(BESTI),
    ADDRESS(BESTJ),
    /* STRAT */ ORDERS(VLOAD_STAR, DOT_STAR),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ORDERS(PUSH, BOFINV),
    SWITCH(BOFINV, VFLAG),
    ADDRESS_AT(STRAT, -3),
    ORDERS(DLOAD, DSU),
    ORDER(BPL),
    ADDRESS(PIC3),
    /* NEWPAR */ ORDERS(SXA_1, SXA_2),
    ADDRESS(BESTI),
    ADDRESS(BESTJ),
    ORDER(GOTO),
    ADDRESS(PIC3),
    /* OCCULT */ ORDERS(MXV, BVSU),
    ADDRESS_PLUS_ONE(CULTRIX),
    ADDRESS_PLUS_ONE_D(14), /* CSS, CEARTH's 14D */
    ORDER(BZE),
    ADDRESS(CULTED),
    ORDERS(BMN, SIGN),
    ADDRESS(CULTED),
    ADDRESS_PLUS_ONE_AT(MPAC, 3),
    ORDERS(BMN, SIGN),
    ADDRESS(CULTED),
    ADDRESS_PLUS_ONE_AT(MPAC, 5),
    ORDERS(BMN, CLRGO),
    ADDRESS(CULTED),
    SWITCH(CLRGO, CULTFLAG),
    ADDRESS(QPRET),
    /* CULTED */ ORDER(SETGO),
    SWITCH(SETGO, CULTFLAG),
    ADDRESS(QPRET),
};

const struct interpretive_code PICAPAR_CODE = {014u, PICAPAR + 3u, COUNT(picapar_words),
                                               picapar_words};

/* PICEND: every major star tried. With VFLAG still set no pair was
 * found: back to the caller's +2 (PICBXT). */
static const word picend_words[] = {
    /* PICEND */ ORDERS(BOFF, EXIT),
    SWITCH(BOFF, VFLAG),
    ADDRESS(PICGXT),
};

const struct interpretive_code PICEND_CODE = {014u, PICEND, COUNT(picend_words), picend_words};

void picend_after_exit(void)
{
    tc_at(PICEND + 3u); /* TC PICBXT */
    picbxt();
}

/* PICGXT: a pair found. The star nearer the shaft axis becomes BESTJ
 * (SXA,1 and SXA,2 swap them unless BESTJ's is already the nearer). */
static const word picgxt_words[] = {
    /* PICGXT */ ORDERS(LXA_1, LXA_2),
    ADDRESS(BESTI),
    ADDRESS(BESTJ),
    ORDERS(VLOAD, DOT_STAR),
    ADDRESS_PLUS_ONE(SAX),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ORDERS(PDVL, DOT_STAR),
    ADDRESS_PLUS_ONE(SAX),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ORDER(DSU),
    ORDERS(BPL, SXA_1),
    ADDRESS(PICNSWP),
    ADDRESS(BESTJ),
    ORDER(SXA_2),
    ADDRESS(BESTI),
    /* PICNSWP */ ORDER(EXIT),
};

const struct interpretive_code PICGXT_CODE = {014u, PICGXT, COUNT(picgxt_words), picgxt_words};

/* PICNSWP +1: the caller's +3, the normal return. */
void picnswp_after_exit(void)
{
    incr(ADDRESS_OF(QMIN));
    picbxt();
}

/* PICBXT: back to the caller through its CADR in QMIN (SWCALL). */
static void picbxt(void)
{
    ca(ADDRESS_OF(QMIN));
    tc_to(PICBXT + 1u, SWCALL);
}

#endif
