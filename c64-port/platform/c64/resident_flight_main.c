/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident's entry: boot, then resident steps until the
 * program reaches a point that neither the kernel nor the module store
 * resolves. Where the AGC's TC trap would restart it, the resident does
 * too (rk_gojam): a TC to itself (V69's TC VERB69), a step that leaves Z
 * and FB as they were with Q = Z + 1. A refusal (EXEC_INVALID_ACTION)
 * stops the run: compiled routines refuse for points they do not cover,
 * and the one compiled TCF to itself (Comanche055 TVCEXECUTIVE 16,2763,
 * marked NOT USED) refuses from its routine's entry, with the routine's
 * earlier writes kept and Z not at the trap (#607). The night watchman and
 * the rupt lock are not modelled (docs/FLIGHT-RESTART.md). */
#include "resident_flight.h"
int main(void)
{
    agc_word *const w = ((apollo_overlay_state *)APOLLO_SHARED_ADDRESS)->words;
    enum exec_status s;
    agc_word z, fb;
    if (!rk_boot())
        for (;;) {
            z = w[5];
            fb = w[4];
            s = rk_resident_step(AGC_TRANSFER_NONE);
            if (s == EXEC_OK && (w[5] != z || w[4] != fb || w[2] != ((z + 1u) & 07777u))) continue;
            if (s != EXEC_OK || rk_gojam()) break;
        }
    for (;;) {
    }
}
