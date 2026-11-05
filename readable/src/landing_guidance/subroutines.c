/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 825-826 (Luminary099 only):
 * "TRASHY LITTLE SUBROUTINES" (INTPRETX, TDISPSET) and the "SPECIALIZED 'PHASCHNG'
 * SUBROUTINE". The faithful C is c64-port/src/lm_llge.c (and, for FASTCHNG
 * as THROTTLE calls it, c64-port/src/lm_throttle.c). */
#include "landing_guidance.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"

/* INTPRETX: "SET X1 ON THE WAY TO THE INTERPRETER": X1 gets minus the
 * phase's target index (INDEX WCHPHASE; CS TARGTDEX: WCHPHASE -1 to 2,
 * IGNALG to VERTICAL), and the interpretive code after the caller's TC
 * INTPRETX runs (TCF INTPRET). Not modelled: an index that makes the CS
 * another instruction or reads outside the table's four words. (A, a
 * constant's complement, never overflows: TS X1 does not skip.) */
void intpretx(void)
{
    word instruction = indexed_instruction((word)(040000u | TARGTDEX), fetch(ADDRESS_OF(WCHPHASE)));
    word at = ADDRESS_FIELD(instruction);

    if (OPERATION_CODE(instruction) != 4u || at < TARGTDEX - 1u || at > TARGTDEX + 2u)
        not_modelled();
    cs(at);
    ts(FIXLOC + ADDRESS_OF(X1)); /* INDEX FIXLOC */
    intpret();
}

/* TDISPSET: TTFDISP, the time to go for the display (TTF/8 times
 * TSCALINV), and TREDES, the time left to redesignate: "TREDES BECOMES
 * ZERO TWO PASSES BEFORE TCGFAPPR IS REACHED". The approach phase's
 * target time TCGFAPPR is in E-bank 5 (TTF/8's E-bank back after it:
 * INCR BBANK twice, 5 to 7); the sum with TTF/8 times TREDESCL, less 103
 * and held at zero or below (an overflow past NEGMAX skips the CS L that
 * would zero A), plus 99 and held at zero or above (an overflow past
 * POSMAX skips the CS TREDES that would zero it): TREDES between 0 and
 * 99. Then TC Q, the caller's return. (A is EBANK5, a constant: TS EBANK
 * never skips.) */
void tdispset(void)
{
    ca(ADDRESS_OF(TTF_8));
    mp(TSCALINV);
    dxch(ADDRESS_OF(TTFDISP));
    set_A(EBANK5);
    ts(EBANK_ADDRESS);
    ca(ADDRESS_OF(TCGFAPPR));
    incr(BBANK_ADDRESS);
    incr(BBANK_ADDRESS);
    ad_at(ADDRESS_OF(TTF_8));
    mp(TREDESCL);
    ad(MINUS_DEC103);
    ad(NEGMAX);
    if (!ts(L_ADDRESS)) cs(L_ADDRESS);
    ad_at(L_ADDRESS);
    ad(PLUS_DEC99);
    ad(POSMAX);
    if (!ts(ADDRESS_OF(TREDES))) cs(ADDRESS_OF(TREDES));
    ads(ADDRESS_OF(TREDES));
    tc_through(Q_ADDRESS);
}

/* FASTCHNG: PHSNAME3, restart group 3's phase, gets the caller's return.
 * XCH EBANK switches in E-bank 3 and leaves the caller's EBANK in A; DXCH L
 * moves the return from Q to A and that EBANK to L; LXCH EBANK puts it
 * back, and TC A returns: A's word runs as the instruction TC <address>.
 * Not modelled: a return that is not such an address (bits 13-15 set, or
 * below 010), nor one whose sixteenth bit differs from its fifteenth: TS
 * PHSNAME3 would skip the LXCH and leave A +1 or -1. */
void fastchng(void)
{
    set_A(EBANK3);
    xch(EBANK_ADDRESS);
    dxch(L_ADDRESS);
    if (!ts(ADDRESS_OF(PHSNAME3))) lxch(EBANK_ADDRESS);
    if (A < 010u || A > 07777u) not_modelled();
    tc_through(A_ADDRESS);
}

#endif
