/* SPDX-License-Identifier: MIT */
/* An RTB from an order pair, run on: the routines of RTB_OP_CODES
 * (src/rtb_op_codes/) end where the listing leaves the file, at SLOAD2,
 * NEWMODE, TPAGREE, BANKCALL and the like; an order pair that RTBs to one
 * goes on from there to its DANZIG, as the faithful port's pairs do
 * (their RTB bodies, c64-port/src/rtb_*.c, run through). Plumbing only. */
#include "binding.h"
#include "../agc/names.h"
#include "../interbank/interbank.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"

#define RTB_BANK (word)(I_SGNAGREE & 076000u) /* the file's bank */

/* The routines an RTB runs on through, at their addresses (under their
 * banks for a switched address): the interpreter's and the inter-bank
 * communication's the file leaves for, and the file's own words where
 * those return. */
static const struct {
    word z, bank;
    void (*routine)(void);
} on[] = {
    {K_SLOAD2, 0u, sload2},
    {K_SLOAD2 + 2u, 0u, sload2_plus2},
    {K_NEWMODE, 0u, newmode},
#if !LUMINARY099
    {K_VMODE, 0u, vmode},
#endif
    {TPAGREE, 0u, tpagree},
    {BANKCALL, 0u, bankcall},
    {SWRETURN, 0u, swreturn},
    {POSTJUMP, 0u, postjump},
    {VECAGREE, 0u, vecagree},
    {UNIT + 1u, 0u, unit_plus1},
    {SGNAGREE + 1u, RTB_BANK, sgnagree_after_tpagree},
    {NORMUNIT + 5u, RTB_BANK, normunit_after_vecagree},
    {VECSGNAG + 2u, RTB_BANK, vecsgnag_after_vecagree},
    {PULSEIMU + 4u, RTB_BANK, pulseimu_after_imupulse},
};

void rtb_run_on(void)
{
    unsigned i, steps;

    for (steps = 0; steps < 64u && Z != DANZIG; ++steps) {
        for (i = 0; i < sizeof on / sizeof on[0]; ++i)
            if (Z == on[i].z && (Z >= 04000u || (FBANK & 076000u) == on[i].bank)) break;
        if (i == sizeof on / sizeof on[0]) return; /* (somewhere else: an alarm) */
        on[i].routine();
    }
}
