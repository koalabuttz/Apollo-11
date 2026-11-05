/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 811-814 (Luminary099 only):
 * "ROUTINES FOR EXITING FROM LANDING GUIDANCE". As the listing has it,
 *
 *     1. EXGSUB IS THE RETURN WHEN GUIDSUB IS CALLED BY THE IGNITION
 *        ALGORITHM.
 *     2. EXBRAK IN THE EXIT USED DURING THE BRAKING PHASE. IN THIS CASE
 *        UNIT(R) IS THE WINDOW POINTING VECTOR.
 *     3. EXNORM IS THE EXIT USED AT OTHER TIMES DURING THE BURN.
 *
 * Then the steering's outputs (STEER?, EXVERT, the throttle and the CDU
 * commands of GDUMP1) or, without them, RATESTOP. The faithful C is
 * c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- EXGSUB -- pp. 811-812 ---- */

/* EXGSUB: "COMPUTE TRIM VELOCITY CORRECTION TERM." */
void exgsub(void)
{
    tc_at(EXGSUB);
    intpret();
}

/* UNFC/2's unit vector times ZOOMTIME times TRIMACCL, to UNFC/2. */
static const word exgsub_words[] = {
    /* EXGSUB +1 */ ORDERS(VLOAD, RTB),
    ADDRESS_PLUS_ONE(UNFC_2),
    ADDRESS(NORMUNIT),
    ORDERS(VXSC, VXSC),
    ADDRESS_PLUS_ONE(ZOOMTIME),
    ADDRESS_PLUS_ONE(TRIMACCL),
    STORE(UNFC_2),
    ORDER(EXIT),
};

const struct interpretive_code EXGSUB_CODE = {031u, EXGSUB + 1u, COUNT(exgsub_words), exgsub_words};

/* EXGSUB's EXIT +1: while NGUIDSUB counts passes still to run, GUIDSUB
 * again; then, with the ignition algorithm's loop count NIGNLOOP not +0,
 * on to DDUMCALC (POSTJUMP); at +0, alarm 1412 first. The CCSs follow
 * the listing's words: NGUIDSUB negative goes on at the TCF +3, -0 at
 * the alarm. Not modelled: NIGNLOOP negative (the alarm's code would run
 * as an instruction). */
void exgsub_after_exit(void)
{
    switch (ccs_at(ADDRESS_OF(NGUIDSUB))) {
    case POSITIVE: guidsub(); return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: exgsub_postjump(); return;
    case IS_MINUS_ZERO: tc_to(EXGSUB + 015u, ALARM); return;
    }
    switch (ccs_at(ADDRESS_OF(NIGNLOOP))) {
    case POSITIVE:
    case IS_MINUS_ZERO: exgsub_postjump(); return;
    case IS_PLUS_ZERO: tc_to(EXGSUB + 015u, ALARM); return; /* OCT 01412 */
    case NEGATIVE: not_modelled();
    }
}

/* +3: on to DDUMCALC (THE_LUNAR_LANDING.agc), and ALARM's return. */
void exgsub_postjump(void)
{
    tc_to(EXGSUB + 017u, POSTJUMP);
}

/* ---- EXBRAK -- p. 812 ---- */

/* EXBRAK: "UNIT(R) IS THE WINDOW POINTING VECTOR" (UNWC/2). */
void exbrak(void)
{
    tc_at(EXBRAK);
    intpret();
}

static const word exbrak_words[] = {
    /* EXBRAK +1 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNIT_R_),
    STORE(UNWC_2),
    ORDER(EXIT),
};

const struct interpretive_code EXBRAK_CODE = {031u, EXBRAK + 1u, COUNT(exbrak_words), exbrak_words};

/* ---- EXNORM -- pp. 812-813 ---- */

/* EXNORM: "UNIT(LAND - R) IS TENTATIVE CHOICE" of the window pointing
 * vector; PROJ, its projection, is "IN MPAC 1/8 REAL SIZE". */
void exnorm(void)
{
    tc_at(EXNORM);
    intpret();
}

static const word exnorm_words[] = {
    /* EXNORM +1 */ ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(LAND),
    ADDRESS_PLUS_ONE(R),
    ORDER(RTB),
    ADDRESS(NORMUNIT),
    STORE(UNWC_2), /* "UNIT(LAND - R) IS TENTATIVE CHOICE" */
    ORDERS(VXV, DOT),
    ADDRESS_PLUS_ONE(XNBPIP),
    ADDRESS_PLUS_ONE_AT(CG, 6),
    ORDER(EXIT), /* "WITH PROJ IN MPAC 1/8 REAL SIZE" */
};

const struct interpretive_code EXNORM_CODE = {031u, EXNORM + 1u, COUNT(exnorm_words), exnorm_words};

/* EXNORM's EXIT +1: the window vector UNWC/2 blends CG +14 in, by two
 * weights: BUF, PROJMAX less PROJ ("RESULT IS 0 IF PROJMAX - PROJ
 * NEGATIVE"), on CG +14, and BUF +1, PROJ less PROJMIN ("RESULT IS 0 IF
 * PROJ - PROJMIN NEGATIVE"), on UNIT(LAND - R). Each difference plus
 * POSMAX overflows if it is positive: TS then stores one less, skips the
 * CS and leaves +1 in A, which ADS adds back. Else the CS, ADS pair zeroes
 * the weight. Then UNWCLOOP. */
void exnorm_after_exit(void)
{
    cs(ADDRESS_OF(MPAC)); /* "GET COEFFICIENT FOR CG +14" */
    ad(PROJMAX);
    ad(POSMAX);
    if (!ts(ADDRESS_OF(BUF))) cs(ADDRESS_OF(BUF));
    ads(ADDRESS_OF(BUF)); /* "RESULT IS 0 IF PROJMAX - PROJ NEGATIVE" */

    set_A(negate(PROJMIN)); /* "GET COEFFICIENT FOR UNIT(LAND - R)" */
    ad_at(ADDRESS_OF(MPAC));
    ad(POSMAX);
    if (!ts(ADDRESS_OF(BUF) + 1u)) cs(ADDRESS_OF(BUF) + 1u);
    ads(ADDRESS_OF(BUF) + 1u); /* "RESULT IS 0 IF PROJ - PROJMIN NEGATIVE" */

    set_A(FOUR);
    unwcloop();
}

/* UNWCLOOP: for each component, Q 4, 2 and 0 (MASK SIX of the count),
 * BUF times CG +14's (in E-bank 5, CG's) and BUF +1 times UNWC/2's (in
 * E-bank 6, UNWC/2's) to UNWC/2. Then E-bank 7 (PIF's) and STEER?. (A
 * masked count and EBANK5 never overflow: neither TS skips. Q is never
 * negative.) */
void unwcloop(void)
{
    for (;;) {
        mask(SIX);
        ts(Q_ADDRESS);
        set_A(EBANK5);
        ts(EBANK_ADDRESS);
        ca(ADDRESS_OF(BUF));
        mp_indexed(Q_ADDRESS, ADDRESS_OF(CG) + 014u);
        incr(BBANK_ADDRESS);
        dxch_indexed(Q_ADDRESS, ADDRESS_OF(UNWC_2));
        mp_at(ADDRESS_OF(BUF) + 1u);
        das_indexed(Q_ADDRESS, ADDRESS_OF(UNWC_2));
        switch (ccs_at(Q_ADDRESS)) {
        case POSITIVE: continue;
        case IS_PLUS_ZERO:
            incr(BBANK_ADDRESS);
            steer();
            return;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled();
        }
    }
}

/* ---- STEER? to RATESTOP -- pp. 813-814 ---- */

/* STEER?: "IF STEERSW DOWN NO OUTPUTS" (RATESTOP). */
void steer(void)
{
    ca(ADDRESS_OF(FLAGWRD2));
    mask(STEERBIT);
    if (A_is_zero())
        ratestop();
    else
        exvert();
}

/* EXVERT: "IF OVERFLOW ANYWHERE IN GUIDANCE DON'T CALL THROTTLE OR
 * FINDCDUW": EXOVFLOW sounds the alarm ("NON-ABORTIVELY") and ALARM
 * returns to RATESTOP. Else GDUMP1. */
void exvert(void)
{
    ca(ADDRESS_OF(OVFIND));
    if (A_is_zero())
        gdump1();
    else
        tc_to(EXOVFLOW, ALARM); /* OCT 01410 */
}

/* RATESTOP: "ARE WE IN ATTITUDE-HOLD?" (channel 31's bit 13 clear). If
 * not, "DO A STOPRATE" (BANKCALL), and both on to DISPEXIT. */
void ratestop(void)
{
    set_A(BIT13);
    rand_channel(CHAN31);
    if (A_is_zero())
        dispexit();
    else
        tc_to(RATESTOP + 5u, BANKCALL); /* CADR STOPRATE */
}

/* 1406ALM's ALARM return (p. 826; alarm 1406, from the time to go's
 * computation): TCF RATESTOP. */
void alm1406_after_alarm(void)
{
    ratestop();
}

/* GDUMP1: THROTTLE, then the CDU commands' interpretive code. */
void gdump1(void)
{
    tc_to(GDUMP1, THROTTLE);
}

/* THROTTLE's return: TC INTPRET. */
void gdump1_after_throttle(void)
{
    tc_at(GDUMP1 + 1u);
    intpret();
}

/* CALL FINDCDUW -2 (the attitude's CDU commands, FINDCDUW--GUIDAP_INTERFACE.agc),
 * and EXIT on to DISPEXIT. */
static const word gdump1_words[] = {
    /* GDUMP1 +2 */ ORDER(CALL),
    ADDRESS_AT(FINDCDUW, -2),
    ORDER(EXIT),
};

const struct interpretive_code GDUMP1_CODE = {031u, GDUMP1 + 2u, COUNT(gdump1_words), gdump1_words};

#endif
