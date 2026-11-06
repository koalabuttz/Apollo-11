/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 3 (Comanche055 pp. 748-755, bank 14): S50
 * and R56, the stars to sight.
 *
 * S50, alias LOCSAM, "TO COMPUTE QUATITIGS LISTED BELOW, USED IN THE IMU
 * ALIGNMENT PROGRAMS": the unit vectors to the sun, the earth and the
 * moon from the CM at a time (TSIGHT), the cosine of each body's half
 * angle as the CM sees it, which a star must clear (CSUN, CEARTH, CMOON
 * in the push-down list), and VEL/C, the CM's velocity over the speed of
 * light, for the aberration of starlight. PICAPAR (R56) picks the pair of
 * stars to sight: "EACH STAR IN THE CATALOG IS TESTED TO DETERMINE IF IT
 * IS OCCULTED BY EITHER EARTH, SUN OR MOON", each one that is not is
 * paired with every star of lower index, and of the pairs with "GOOD
 * SEPARATION" and "IN FIELD OF VIEW" the one farthest apart is chosen
 * (BESTI, BESTJ). VFLAG set means no pair was found: PICAPAR returns to
 * its caller's +2 then, its +3 otherwise.
 *
 * Most of it is interpretive code. The basic code: PICAPAR's entry
 * (MAKECADR keeps the caller's return in QMIN), and its two ends, PICBXT,
 * which returns through QMIN (+1 when a pair was found). */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- S50 (LOCSAM) -- pp. 748-751 ---- */

/* S50: the time in MPAC, kept in TSIGHT; the sun's and the moon's
 * directions from the earth then (LSPOS: the moon's in MPAC, the sun's at
 * 2D), and the CM's state (CSMCONIC, at TDEC1: RATT, VATT). Then, by the
 * sphere of influence the CM is in (S2 +0 and TIX,2: X2, as CSMCONIC
 * leaves it), the vectors from the earth's centre (EARTCNTR) or the
 * moon's (MOONCNTR). The cosines (CEARTH, CSUN, CMOON) go to the
 * push-down list's 14D, 16D and 18D.
 * ENDSAM: VEL/C, the earth's velocity about the sun (ECLIPOL, crossed
 * with the sun's direction) and the CM's own (VATT) over the speed of
 * light (1/C). OCCOS: the cosine of a body's half angle plus five
 * degrees, from its radius (MPAC) over its distance (36D, where UNIT
 * leaves the length). */
static const word s50_words[] = {
    /* S50 */ ORDER(STQ),
    ADDRESS(QMAJ),
    STCALL(TSIGHT),
    ADDRESS(LSPOS),
    STOVL(VMOON),
    ADDRESS_PLUS_ONE_D(2),
    STODL(VSUN),
    ADDRESS_PLUS_ONE(TSIGHT),
    STCALL(TDEC1),
    ADDRESS(CSMCONIC),
    ORDERS(SSP, TIX_2),
    ADDRESS_PLUS_ONE(S2),
    NUMBER(0),
    ADDRESS(MOONCNTR),
    /* EARTCNTR */ ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(VMOON),
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
    STOVL_D(18), /* CMOON */
    ADDRESS_PLUS_ONE(VSUN),
    ORDER(UNIT),
    STCALL(VSUN),
    ADDRESS(ENDSAM),
    /* MOONCNTR */ ORDERS(VLOAD, VSR8),
    ADDRESS_PLUS_ONE(VMOON),
    ORDERS(VSR1, BVSU),
    ADDRESS_PLUS_ONE(VSUN),
    ORDER(UNIT),
    STOVL(VSUN),
    ADDRESS_PLUS_ONE(VMOON),
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE(RATT),
    ORDER(VCOMP),
    STOVL(VEARTH),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, VCOMP),
    STODL(VMOON),
    ADDRESS_PLUS_ONE(RSUBM),
    ORDER(CALL),
    ADDRESS(OCCOS),
    STODL_D(18), /* CMOON */
    ADDRESS_PLUS_ONE(CSS5),
    STOVL_D(14), /* CEARTH */
    ADDRESS_PLUS_ONE(VSUN),
    /* ENDSAM */ ORDER(VXV),
    ADDRESS_PLUS_ONE(ECLIPOL),
    STOVL(VEL_C),
    ADDRESS_PLUS_ONE(VATT),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(1_C),
    ADDRESS_PLUS_ONE(VEL_C),
    STODL(VEL_C),
    ADDRESS_PLUS_ONE(CSSUN),
    STCALL_D(16), /* CSUN */
    ADDRESS(QMAJ),
    /* OCCOS */ ORDERS(DDV, SR1),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(ASIN, DAD),
    ADDRESS_PLUS_ONE(5DEGREES),
    ORDERS(COS, SR1),
    ORDER(RVQ),
};

const struct interpretive_code S50_CODE = {014u, LOCSAM, COUNT(s50_words), s50_words};

/* ---- PICAPAR (R56) -- pp. 752-755 ---- */

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
 * 6"), X2 the minor one from the major's (PIC3); neither may be occulted
 * (OCCULT, CULTFLAG); the pair's separation "LESS THAN 66 DEG" (CSS66)
 * and "MORE THAN 40 DEG" (CSS6640), and both within 33 degrees of SAX
 * (CSS33). STRATGY keeps the first pair found (NEWPAR) and each pair
 * after it farther apart than the best so far: STRAT compares the two
 * (VFLAG inverted, so that it runs twice, swapping the pairs with XCHX
 * between). OCCULT: whether a body hides the star (MPAC), the star being
 * within the body's half angle (CULTRIX, the bodies' unit vectors; CSS,
 * the cosines). */
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
    ADDRESS_PLUS_ONE(SIN33),
    ADDRESS_PLUS_ONE(ZNB),
    ORDERS(AXT_1, VXSC),
    NUMBER(228), /* "X1 = 37 X 6 + 6" */
    ADDRESS_PLUS_ONE(COS33),
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
    /* PIC2 */ ORDERS(VLOAD_STAR, CALL),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS(OCCULT),
    ORDERS(BON, LXA_2),
    SWITCH(BON, CULTFLAG),
    ADDRESS(PIC1),
    ADDRESS(X1),
    /* PIC3 */ ORDERS(TIX_2, GOTO),
    ADDRESS(PIC4),
    ADDRESS(PIC1),
    /* PIC4 */ ORDERS(VLOAD_STAR, CALL),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ADDRESS(OCCULT),
    ORDERS(BON, VLOAD_STAR),
    SWITCH(BON, CULTFLAG),
    ADDRESS(PIC3),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ORDERS(DOT_STAR, DSU),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ADDRESS_PLUS_ONE(CSS66), /* "SEPARATION LESS THAN 66 DEG." */
    ORDERS(BMN, DAD),
    ADDRESS(PIC3),
    ADDRESS_PLUS_ONE(CSS6640), /* "SEPARATION MORE THAN 40 DEG." */
    ORDER(BPL),
    ADDRESS(PIC3),
    ORDERS(VLOAD_STAR, DOT),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS_PLUS_ONE(SAX),
    ORDERS(DSU, BMN), /* "MAJOR STAR IN CONE" */
    ADDRESS_PLUS_ONE(CSS33),
    ADDRESS(PIC1),
    ORDERS(VLOAD_STAR, DOT),
    ADDRESS_PLUS_ONE_AT_X2(CATLOG, 0),
    ADDRESS_PLUS_ONE(SAX),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(CSS33),
    ADDRESS(STRATGY),
    ORDER(GOTO),
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

/* PICGXT: a pair found. */
static const word picgxt_words[] = {
    /* PICGXT */ ORDER(EXIT),
};

const struct interpretive_code PICGXT_CODE = {014u, PICGXT, COUNT(picgxt_words), picgxt_words};

void picend_after_exit(void)
{
    tc_at(PICEND + 3u); /* TC PICBXT */
    picbxt();
}

/* PICGXT +1: the caller's +3, the normal return. */
void picgxt_after_exit(void)
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
