/* SPDX-License-Identifier: MIT */
/* Single precision sine and cosine: SINGLE_PRECISION_SUBROUTINES.agc of
 * Luminary099 (LM, listing page 1102) and Comanche055 (CM, page 1207),
 * the same code in both. The faithful C is c64-port/src/sp_routines.c,
 * with lm_single_trig.c and cm_quictrig.c.
 *
 * The method: bring the angle into -90 to +90 degrees, where sine has the
 * same values, then sum a short polynomial in it. Along the way TS's skip
 * on overflow does the testing: an overflow is an angle past the range,
 * and the +1 or -1 TS leaves in A says which side. */
#include "single_precision.h"
#include "../agc/names.h"

static void spt(void);
static void polley(void);
static void arg90(void);

/* The words around LIMITS (NEG1/2) in the constant pool, read with INDEX
 * by +1 or -1: LIMITS -1 is POSMAX, LIMITS +1 is BIT15. */
#define LIMITS_ADDRESS ((unsigned)K_LIMITS)

/* SPCOS -- LM p. 1102, CM p. 1207.
 * "ARGUMENTS SCALED AT PI": cos x is the sine of x plus a quarter turn
 * (HALF, at this scale). */
void spcos(void)
{
    ad(HALF);
    spsin();
}

/* SPSIN -- LM p. 1102, CM p. 1207.
 * TS TEMK overflows only after SPCOS's AD HALF, for an angle past a half
 * turn. TS keeps the angle less a half turn; CS TEMK makes that a half
 * turn less the angle, which has the same sine (sin(180 - x) = sin x). */
void spsin(void)
{
    if (ts(ADDRESS_OF(TEMK))) cs(ADDRESS_OF(TEMK));
    spt();
}

/* SPT -- LM p. 1102, CM p. 1207.
 * DOUBLE: the angle scaled at pi/2, a quarter turn now 1. Within a quarter
 * turn either way, on to the polynomial. Past one, TS overflows, keeping
 * the angle less a quarter turn and leaving +1 or -1 in A; the steps after
 * reflect it about 90 (or -90) degrees, to 180 less the angle:
 *     XCH   TEMK        A: the angle less a quarter turn, TEMK: +1 or -1
 *     INDEX TEMK
 *     AD    LIMITS      +1: AD BIT15, -1: AD POSMAX (a quarter turn, less
 *                       its last bit, the other way)
 *     COM               the reflection
 *     AD    TEMK        the last bit back
 *     TS    TEMK
 * Exactly 90 degrees overflows once more: ARG90. */
static void spt(void)
{
    double_A();
    if (!ts(ADDRESS_OF(TEMK))) {
        polley();
        return;
    }
    xch(ADDRESS_OF(TEMK));
    ad_at(indexed(LIMITS_ADDRESS, TEMK));
    com();
    ad(TEMK);
    if (!ts(ADDRESS_OF(TEMK))) {
        polley();
        return;
    }
    arg90();
}

/* POLLEY -- LM p. 1102, CM p. 1207.
 * The sine of the angle t in TEMK (scaled at pi/2) as
 *     t (C1 + t^2 (C3 + t^2 C5))
 * with the coefficients halved, C1/2 being pi/4, so that they fit a word;
 * DDOUBL puts the factor 2 back. A result of 1 or more (near 90 degrees)
 * overflows TS TEMK, which then skips the TC Q into ARG90. */
static void polley(void)
{
    mp(TEMK);
    ts(ADDRESS_OF(SQ));
    mp(C5_2);
    ad(C3_2);
    mp(SQ);
    ad(C1_2);
    mp(TEMK);
    ddoubl();
    if (ts(ADDRESS_OF(TEMK))) {
        arg90();
        return;
    }
    tc_through(Q_ADDRESS); /* TC Q */
}

/* ARG90 -- LM p. 1102, CM p. 1207.
 * Sine of plus or minus 90 degrees: with +1 in A, CS LIMITS +1 (BIT15) is
 * POSMAX; with -1, CS LIMITS -1 (POSMAX) is its negative. "RESULT SCALED
 * AT 1". */
static void arg90(void)
{
    cs(indexed(LIMITS_ADDRESS, a_word()));
    tc_through(Q_ADDRESS); /* TC Q */
}
