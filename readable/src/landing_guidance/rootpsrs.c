/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 822-825 (Luminary099 only):
 * the "DOUBLE PRECISION ROOT FINDER SUBROUTINE (BY ALLAN KLUMPP)". The
 * guidance finds the time to go (TTF/8) as a root of a power series.
 * ROOTPSRS takes, as the listing says,
 *
 *     A     SP  LOC-3   ADRES FOR REFERENCING PWR COF TABL
 *     L     SP  N-1     N IS THE DEGREE OF THE POWER SERIES
 *     MPAC  DP  X       INITIAL GUESS FOR ROOT
 *
 * with the table A(0) .. A(N) below LOC and PRECROOT, the precision
 * wanted, after it. It forms the derivative's table (DERCOFN), and then
 * Newton's method: the root less the residual over the derivative, until
 * the step is below PRECROOT times the first guess. "THE DP RESULT IS
 * LEFT IN MPAC UPON EXIT, AND A SP COUNT OF THE ITERATIONS TO CONVERGENCE
 * IS LEFT IN MPAC+2. RETURN IS NORMALLY TO LOC(TC ROOTPSRS)+3. IF ROOTPSRS
 * FAILS TO CONVERGE TO IN 8 PASSES, RETURN IS TO LOC+1". The power series
 * are POWRSERS's (INTERPRETER.agc), and ABS and DDV/BDDV run as
 * interpretive code through USPRCADR, each a call of its own here. The
 * faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- Entry: the derivative's table -- pp. 823-824 ---- */

/* ROOTPSRS: "STORE ENTERING DATA, INITIALIZE ERASABLES". The return to
 * RETROOT, the table's address to PWRPTR ("PWR TABLE POINTER") and, with
 * N-1, to MPAC +3; DERPTR and MPAC +5 the derivative table's (DERTABLL);
 * MPAC +6 N-2 ("NO POWER SERIES DEGREE 1 OR LESS"); and "MODE USED AS
 * ITERATION COUNTER. MODE MUST BE POS SO ABS WON'T COMP MPAC+3 ETC." Then
 * "COMPUTE CRITERION TO STOP ITERATING": the guess to ROOTPS too, and
 * PRECROOT times it (SHORTMP). An A with an overflow would skip the DXCH
 * (never: the caller loads A with CA). The CCS's outcomes, from the four
 * words
 *
 *         CCS    MPAC +4
 *         TS     MPAC +6
 *         CA     ZERO
 *         TS     MODE
 *         EXTEND
 *
 * for a degree of 1 or less, which no caller has: N-1 +0 skips MPAC +6,
 * negative MODE too, and -0 runs on with MODE as it was. */
void rootpsrs(void)
{
    qxch(ADDRESS_OF(RETROOT));
    if (!ts(ADDRESS_OF(PWRPTR))) dxch(ADDRESS_OF(MPAC) + 3u);
    set_A(DERTABLL);
    ts(ADDRESS_OF(DERPTR));
    ts(ADDRESS_OF(MPAC) + 5u);
    switch (ccs_at(ADDRESS_OF(MPAC) + 4u)) { /* (A is CCS's: these TS never skip) */
    case POSITIVE:
        ts(ADDRESS_OF(MPAC) + 6u); /* N-2 */
        /* fall through */
    case IS_PLUS_ZERO:
        set_A(ZERO);
        /* fall through */
    case NEGATIVE: ts(ADDRESS_OF(MODE)); break;
    case IS_MINUS_ZERO: break;
    }
    dca(ADDRESS_OF(MPAC)); /* "FETCH ROOT GUESS, KEEPING IT IN MPAC" */
    dxch(ADDRESS_OF(ROOTPS));
    ca_indexed(ADDRESS_OF(MPAC) + 3u, 5u); /* "PRECROOT TO A" */
    tc_to(ROOTPSRS + 020u, SHORTMP);       /* "YIELDS DP PRODUCT IN MPAC" */
}

/* SHORTMP's return: "YIELDS ABVAL OF CRITERION ON DX IN MPAC" (USPRCADR,
 * CADR ABS). */
void rootpsrs_after_shortmp(void)
{
    tc_to(ROOTPSRS + 021u, USPRCADR);
}

/* DERCLOOP: PWRCNT, the loop's counter, and its I (one more) for DMPNSUB:
 * "YIELDS DERCOF = I X A(I) IN MPAC". (A, a word or CCS's, never
 * overflows: the TS does not skip.) */
static void dercloop(void)
{
    ts(ADDRESS_OF(PWRCNT));
    ad(ONE);
    tc_to(DERCLOOP + 2u, DMPNSUB);
}

/* After ABS's EXIT: DXCRIT, the "CRITERION". "SET UP DER COF TABL": A(N)
 * to MPAC, and DERCLOOP from N-1. */
void rootpsrs_after_abs(void)
{
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(DXCRIT));
    dca_indexed(ADDRESS_OF(PWRPTR), 3u);
    dxch(ADDRESS_OF(MPAC)); /* "A(N) TO MPAC" */
    ca(ADDRESS_OF(MPAC) + 4u);
    dercloop();
}

static void rootloop(void);
static void rootloop_dxch(void);

/* DMPNSUB's return: "(I-1) TO MPAC, FETCHING DERCOF", the DERCOF to its
 * table, both pointers down two words, and round again while PWRCNT
 * counts down (CCS, TCF DERCLOOP). Then ROOTLOOP, its first words
 *
 *         CCS    PWRCNT
 *         TCF    DERCLOOP
 * ROOTLOOP EXTEND
 *         DCA    ROOTPS
 *         DXCH   MPAC
 *
 * so a negative PWRCNT (never: it counts down to +0) runs the DCA's word
 * without its EXTEND, as CA ROOTPS +1, and -0 goes on at the DXCH. */
void dercloop_after_dmpnsub(void)
{
    dca_indexed(ADDRESS_OF(PWRPTR), 1u);
    dxch(ADDRESS_OF(MPAC));
    dxch_indexed(ADDRESS_OF(DERPTR), 3u); /* "DERCOF TO DER TABLE" */
    set_A(negate(TWO));
    ads(ADDRESS_OF(PWRPTR)); /* "DECREMENT PWR POINTER" */
    set_A(negate(TWO));
    ads(ADDRESS_OF(DERPTR)); /* "DECREMENT DER POINTER" */
    switch (ccs_at(ADDRESS_OF(PWRCNT))) {
    case POSITIVE: dercloop(); return;
    case IS_PLUS_ZERO: rootloop(); return;
    case NEGATIVE: ca(ADDRESS_OF(ROOTPS) + 1u); break;
    case IS_MINUS_ZERO: break;
    }
    rootloop_dxch();
}

/* ---- ROOTLOOP: "CONVERGE ON ROOT" -- pp. 824-825 ---- */

/* "FETCH CURRENT ROOT", "LEAVE IN MPAC", and the derivative's series
 * (POWRSERS) from "DER TABL ADRES, N-2". */
static void rootloop(void)
{
    dca(ADDRESS_OF(ROOTPS));
    rootloop_dxch();
}

static void rootloop_dxch(void)
{
    dxch(ADDRESS_OF(MPAC));
    dca(ADDRESS_OF(MPAC) + 5u);
    tc_to(ROOTLOOP + 5u, POWRSERS); /* "YIELDS DERIVATIVE IN MPAC" */
}

/* POWRSERS's return: "CURRENT ROOT TO MPAC, FETCHING DERIVATIVE", "LEAVE
 * DERIVATIVE IN BUF AS DIVISOR", and the series itself from "PWR TABL
 * ADRES, N-1": "YIELDS RESIDUAL IN MPAC". */
void rootloop_after_derivative(void)
{
    dca(ADDRESS_OF(ROOTPS));
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(BUF));
    dca(ADDRESS_OF(MPAC) + 3u);
    tc_to(ROOTLOOP + 014u, POWRSERS);
}

/* "YIELDS -DX IN MPAC" (USPRCADR, CADR DDV/BDDV), */
void rootloop_after_residual(void)
{
    tc_to(ROOTLOOP + 015u, USPRCADR);
}

/* and after its EXIT: "FETCH DX, LEAVING -DX IN MPAC", "CORRECTED ROOT
 * NOW IN ROOTPS"; "YIELDS ABS(DX) IN MPAC" (USPRCADR, CADR ABS). */
void rootloop_after_ddv(void)
{
    dcs(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(ROOTPS));
    tc_to(ROOTLOOP + 022u, USPRCADR);
}

/* TC through an erasable word that holds a return: the word runs as the
 * instruction TC <address>. Not modelled: a word that is not such an
 * address (bits 13-15 set, or below 010). */
static void tc_through_return(unsigned address)
{
    word to = fetch(address);

    if (to < 010u || to > 07777u) not_modelled();
    tc_through(address);
}

/* ROOTSTOR: the root to MPAC, the count to MPAC +2 ("STORE SP ITERATION
 * COUNT IN MPAC+2"; MODE, a word, never overflows), and INDEX RETROOT;
 * TCF 2: the caller's return +2. Not modelled: an index that makes the
 * TCF another instruction or sends it to erasable memory. */
static void rootstor(void)
{
    word instruction;

    dxch(ADDRESS_OF(ROOTPS));
    dxch(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(MODE));
    ts(ADDRESS_OF(MPAC) + 2u);
    instruction = indexed_instruction(010002u, fetch(ADDRESS_OF(RETROOT)));
    if (OPERATION_CODE(instruction) != 1u || ADDRESS_FIELD(instruction) < 02000u) not_modelled();
    continue_at(ADDRESS_FIELD(instruction));
}

/* After ABS's EXIT: "ABS(DX)-ABS(DXCRIT) IN MPAC". "KLUMPP SAYS GIVE UP
 * AFTER EIGHT PASSES" (MODE bit 4): BADROOT, TC RETROOT, the caller's
 * return +0. Else one pass more; a step still above the criterion goes
 * round again ("TEST HI ORDER DX", "TEST LO ORDER DX"), any other is the
 * root (ROOTSTOR). The four-word tables:
 *
 *         CCS    MPAC              TESTLODX CCS    MPAC +1
 *         TCF    ROOTLOOP                   TCF    ROOTLOOP
 *         TCF    TESTLODX                   TCF    ROOTSTOR
 *         TCF    ROOTSTOR                   TCF    ROOTSTOR
 * TESTLODX                         ROOTSTOR
 *
 * (CCS MPAC's -0 is TESTLODX itself.) */
void rootloop_after_abs(void)
{
    dcs(ADDRESS_OF(DXCRIT));
    das(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(MODE));
    mask(BIT4);
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* BADROOT */
        tc_through_return(ADDRESS_OF(RETROOT));
        return;
    }
    incr(ADDRESS_OF(MODE)); /* "INCREMENT ITERATION COUNTER" */
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: rootloop(); return;
    case NEGATIVE: rootstor(); return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    if (ccs_at(ADDRESS_OF(MPAC) + 1u) == POSITIVE)
        rootloop();
    else
        rootstor();
}

#endif
