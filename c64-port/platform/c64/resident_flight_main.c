/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident's entry: boot, then resident steps until the
 * program reaches a point that neither the kernel nor the module store
 * resolves. Where the AGC's TC trap would restart it, the resident does
 * too (rk_gojam): a TCF to itself, which a compiled routine refuses
 * (EXEC_INVALID_ACTION), and a TC to itself (V69's TC VERB69), a step that
 * leaves Z and FB as they were with Q = Z + 1. The night watchman and the
 * rupt lock are not modelled (docs/FLIGHT-RESTART.md). */
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
            if ((s != EXEC_OK && s != EXEC_INVALID_ACTION) || rk_gojam()) break;
        }
    for (;;) {
    }
}
