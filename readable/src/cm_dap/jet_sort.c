/* SPDX-License-Identifier: MIT */
/* JET_SELECTION_LOGIC.agc (Comanche055 pp. 1059-1060, bank 17): "SORT THE JET
 * ON-TIMES". "AT THIS POINT ALL THE CHANNEL COMMANDS AND JET ON-TIMES HAVE
 * BEEN DETERMINED": RWORD1 and RWORD2 with BLAST, PWORD1 and PWORD2 with
 * BLAST1, YWORD1 and YWORD2 with BLAST2. "THE JET ON-TIMES ARE SORTED AND
 * THE SEQUENCE OF T6 INTERRUPTS IS DETERMINED": BLAST, BLAST1 and BLAST2
 * are double words, each with "A BRANCH INDEX ASSOCIATED WITH THE
 * ROTATION AXIS" in its lower word, and are swapped into increasing
 * order; the second and third then become the times after the one before
 * (the T6 interrupts come one after another). Phase 3 ends: T5LOC back to
 * RCSATT for phase 1, and the T6 program told to start over. The
 * faithful C is c64-port/src/cm_jet_select.c, whose runs stop
 * at the keys of its parts: the functions here start at them. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void endjets(void);

/* T6SETUP: the branch indexes, 0 for roll, 4 for pitch, 11 for yaw; then
 * the first two on-times in order (DXCHT12 if BLAST1 is no longer than
 * BLAST: "T1 GR T2"). */
void t6setup(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(BLAST) + 1u);
    set_A(FOUR);
    ts(ADDRESS_OF(BLAST1) + 1u);
    set_A(ELEVEN);
    ts(ADDRESS_OF(BLAST2) + 1u);
    cs(ADDRESS_OF(BLAST));
    ad_at(ADDRESS_OF(BLAST1));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DXCHT12 */
        continue_at(DXCHT12);
        return;
    }
    checkt23();
}

/* CHECKT23: the second and third (DXCHT23 if BLAST2 is no longer than
 * BLAST1). */
void checkt23(void)
{
    cs(ADDRESS_OF(BLAST1));
    ad_at(ADDRESS_OF(BLAST2));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DXCHT23 */
        continue_at(DXCHT23);
        return;
    }
    calcdt6();
}

/* CALCDT6: in order, the on-times become the times between the T6
 * interrupts: BLAST2 less BLAST1, at CALCDT6 +1. */
void calcdt6(void)
{
    cs(ADDRESS_OF(BLAST1));
    continue_at(CALCDT6 + 1u);
}

/* CALCDT6 +1: BLAST2 less BLAST1, then BLAST1 less BLAST at CALCDT6 +3. */
void calcdt6_plus1(void)
{
    ads(ADDRESS_OF(BLAST2));
    cs(ADDRESS_OF(BLAST));
    calcdt6_plus3();
}

/* CALCDT6 +3: BLAST1 less BLAST, "END OF SORTING PROCEDURE". "RESET T5LOC
 * TO BEGIN PHASE1" (RCS2CADR, the 2CADR of RCSATT), and ENDJETS. */
void calcdt6_plus3(void)
{
    ads(ADDRESS_OF(BLAST1));
    dca(ADDRESS_OF(RCS2CADR));
    dxch(ADDRESS_OF(T5LOC));
    endjets();
}

/* ENDJETS: "RESET BIT1 FOR INITIALIZATION OF T6 PROGRAM" (RCSFLAGS),
 * "RESET T5PHASE FOR PHASE1" (-0), and "RESUME INTERRUPTED PROGRAM". */
static void endjets(void)
{
    set_A(negate(BIT1));
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    set_A(negate(ZERO));
    ts(ADDRESS_OF(T5PHASE));
    continue_at(RESUME);
}

/* ---- The exchanges -- p. 1060 ---- */

/* DXCHT12: BLAST and BLAST1 swapped (A,L as they were), on to CHECKT23.
 * The swap's second word is a key of its own. */
void dxcht12(void)
{
    dxch(ADDRESS_OF(BLAST));
    dxcht12_plus1();
}

/* DXCHT12 +1. */
void dxcht12_plus1(void)
{
    dxch(ADDRESS_OF(BLAST1));
    dxch(ADDRESS_OF(BLAST));
    continue_at(CHECKT23);
}

/* DXCHT23: BLAST1 and BLAST2 swapped; then BLAST and the new BLAST1 swapped
 * too if BLAST1 is no longer than BLAST; on to CALCDT6. */
void dxcht23(void)
{
    dxch(ADDRESS_OF(BLAST1));
    dxch(ADDRESS_OF(BLAST2));
    dxch(ADDRESS_OF(BLAST1));
    cs(ADDRESS_OF(BLAST));
    ad_at(ADDRESS_OF(BLAST1));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        dxch(ADDRESS_OF(BLAST));
        dxch(ADDRESS_OF(BLAST1));
        dxch(ADDRESS_OF(BLAST));
    }
    continue_at(CALCDT6);
}

#endif
