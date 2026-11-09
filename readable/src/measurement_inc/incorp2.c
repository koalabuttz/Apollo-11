/* SPDX-License-Identifier: MIT */
/* MEASUREMENT_INCORPORATION.agc, INCORP2's basic code (LM pp. 1153-1157,
 * CM pp. 1256-1260): the bookkeeping of its three phases, each made safe
 * against a restart. The first phase works one column of W at a time into
 * HOLDW; the second moves HOLDW into W. Each column's indices are kept
 * twice: WIXA and ZIXA for the phase at work, WIXB and ZIXB for the next.
 * A restart in either phase goes back to FAZA (group 2's phase, OCT 04022)
 * and redoes the column from the B copies. */
#include "measurement_inc.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

/* TC at "at" to a routine outside the file (PHASCHNG, UPFLAG, POSTJUMP),
 * which reads the words after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* INCORP2 +020, after its EXIT: the "INITIAL IX 1 SETTING FOR W MATRIX",
 * 54 (54DD), to WIXA and WIXB, and the "INITIAL IX 2 SETTING FOR Z
 * COMPONENT", 0, to ZIXA and ZIXB. Then FAZA. (A CAF leaves no overflow
 * for a TS to skip on.) */
void incorp2_after_exit(void)
{
    set_A(FIFTY_FOUR_DD); /* CAF 54DD */
    if (ts(ADDRESS_OF(WIXA))) not_modelled();
    if (ts(ADDRESS_OF(WIXB))) not_modelled();
    set_A(ZERO); /* CAF ZERO */
    if (ts(ADDRESS_OF(ZIXA))) not_modelled();
    if (ts(ADDRESS_OF(ZIXB))) not_modelled();
    faza();
}

/* FAZA: group 2's phase 2.2 (PHASCHNG, OCT 04022), the restart point of
 * the first phase. */
void faza(void)
{
    tc_to(FAZA, PHASCHNG); /* OCT 04022 */
}

/* FAZA +2: REINTFLG set (UPFLAG), so a restart finishes the
 * incorporation. */
void faza_after_phaschng(void)
{
    tc_to((word)(FAZA + 2u), UPFLAG); /* ADRES REINTFLG */
}

/* FAZA1: "START FIRST PHASE OF INCORP2" "TO UPDATE 6 OR 9 DIM. W MATRIX
 * IN TEMP": the indices from the B copies (WIXB to WIXA, ZIXB to ZIXA),
 * then the interpreter. (A CA leaves no overflow.) */
void faza1(void)
{
    ca(ADDRESS_OF(WIXB));
    if (ts(ADDRESS_OF(WIXA))) not_modelled();
    ca(ADDRESS_OF(ZIXB));
    if (ts(ADDRESS_OF(ZIXA))) not_modelled();
    tc_at((word)(FAZA1 + 4u)); /* TC INTPRET */
    intpret();
}

/* FAZB1: "START 2ND PHASE OF INCORP2 TO TRANSFER" "TEMP REG TO PERM W
 * MATRIX": the next column's indices into the B copies, WIXA less 6
 * (6DD) and ZIXA less 2 (MINUS2), then the interpreter. An overflow of
 * the first sum skips the CA ZIXA, as TS does. */
void fazb1(void)
{
    ca(ADDRESS_OF(WIXA));
    ad(SIX_DD); /* AD 6DD: DEC -6 */
    if (!ts(ADDRESS_OF(WIXB))) ca(ADDRESS_OF(ZIXA));
    ad(MINUS2);
    if (ts(ADDRESS_OF(ZIXB))) not_modelled();
    tc_at((word)(FAZB1 + 6u)); /* TC INTPRET */
    intpret();
}

/* FAZAB5 +4, after its EXIT: out by POSTJUMP to INTWAKE, which lets the
 * jobs waiting for the integration go on ("EXIT"). */
void fazab5_after_exit(void)
{
    tc_to((word)(FAZAB5 + 4u), POSTJUMP); /* CADR INTWAKE */
}
