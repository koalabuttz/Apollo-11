/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 807-808 and 826 (Luminary099
 * only): "TTF/8 COMPUTATION" (TTF/8CL), on to QUADGUID, and the "ALARM
 * ROUTINE FOR TTF COMPUTATION" (1406P00, 1406ALM). The faithful C is
 * c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TTF/8CL: the time to go is the root of a cubic in TTF/8 whose
 * coefficients, to TABLTTF, come from the phase's targets (through X1,
 * INTPRETX: the jerk JDG, the acceleration ADG, the velocity VDG and the
 * position RDG) and the state (VGU, RGU), their components 2:
 *
 *     A(3) = 8 JDG
 *                 2
 *     A(2) = 6 ADG
 *                 2
 *     A(1) = (6 VGU  + 18 VDG )/8
 *                  2         2
 *     A(0) = -24 (RGU  - RDG )/64
 *                    2      2
 */
void ttf_8cl(void)
{
    tc_at(TTF_8CL);
    intpretx();
}

static const word ttf_8cl_words[] = {
    /* TTF/8CL +1 */ ORDER(DLOAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(JDG2TTF, 0),
    STODL_STAR_AT(TABLTTF, 6), /* "A(3) = 8 JDG  TO TABLTTF" */
    ADDRESS_PLUS_ONE_AT_X1(ADG2TTF, 0),
    STODL_AT(TABLTTF, 4), /* "A(2) = 6 ADG  TO TABLTTF" */
    ADDRESS_PLUS_ONE_AT(VGU, 4),
    ORDERS(DMP, DAD_STAR),
    ADDRESS_PLUS_ONE(3_4DP),
    ADDRESS_PLUS_ONE_AT_X1(VDG2TTF, 0),
    STODL_STAR_AT(TABLTTF, 2),
    ADDRESS_PLUS_ONE_AT_X1(RDG, 4),
    ORDERS(DSU, DMP),
    ADDRESS_PLUS_ONE_AT(RGU, 4),
    ADDRESS_PLUS_ONE(3_8DP),
    STORE(TABLTTF),
    ORDER(EXIT),
};

const struct interpretive_code TTF_8CL_CODE = {031u, TTF_8CL + 1u, COUNT(ttf_8cl_words),
                                               ttf_8cl_words};

/* After the EXIT: "FRACTIONAL PRECISION FOR TTF TO TABLE" (BIT8 to
 * TABLTTF +10), "TTF/8 (INITIAL GUESS)" to MPAC, the degree less one (two)
 * to L and the table's address (TABLTTFL) to A, and ROOTPSRS
 * (rootpsrs.c), which "YIELDS TTF/8 IN MPAC". (No TS skips: A is a
 * constant.) */
void ttf_8cl_after_exit(void)
{
    set_A(BIT8);
    ts(ADDRESS_OF(TABLTTF) + 010u); /* "FRACTIONAL PRECISION FOR TTF TO TABLE" */
    dca(ADDRESS_OF(TTF_8));
    dxch(ADDRESS_OF(MPAC)); /* "LOADS TTF/8 (INITIAL GUESS) INTO MPAC" */
    set_A(TWO);             /* "DEGREE - ONE" */
    ts(L_ADDRESS);
    set_A(TABLTTFL);
    tc_at(TTF_8CL + 031u);
    rootpsrs();
}

/* ROOTPSRS's return when the root would not converge: the phase's
 * WHATALM word. */
void ttf_8cl_bad_root(void)
{
    tcf_by_phase(WHATALM, ADDRESS_OF(WCHPHASE));
}

/* Its "GOOD RETURN": the "CORRECTED TTF/8", TDISPSET (subroutines.c),
 * and on into QUADGUID (quadguid.c), the word after. */
void ttf_8cl_good_root(void)
{
    dca(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(TTF_8)); /* "CORRECTED TTF/8" */
    tc_at(TTF_8CL + 037u);
    tdispset();
    quadguid(); /* "CONTINUE TO QUADGUID" */
}

/* 1406P00, the ignition algorithm's: alarm 1406 and the end of the
 * program (POODOO). */
void p00_1406(void)
{
    tc_to(P00_1406, POODOO); /* OCT 01406 */
}

/* 1406ALM, the braking and approach phases': alarm 1406 (ALARM), ... */
void alm1406(void)
{
    tc_to(ALM1406, ALARM); /* OCT 01406 */
}

/* ... and RATESTOP (exits.c). */
void alm1406_after_alarm(void)
{
    ratestop();
}

#endif
