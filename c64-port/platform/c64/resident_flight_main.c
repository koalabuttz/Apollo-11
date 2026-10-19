/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident's entry: boot, the hardware restart (GOJAM:
 * the rope's power-on code from 04000, as the AGC starts when it is
 * switched on; with an image of zeros it ends in a fresh start), then
 * resident steps until the
 * program reaches a point that neither the kernel nor the module store
 * resolves. Where the AGC's TC trap would restart it, the resident does
 * too (rk_gojam): Z at VERB69 (43,2037 in both ropes, V69's TC VERB69, the
 * one TC to itself in either) outside an interrupt, before the step that
 * would spin there, the point tools/flight_host_run.c tc_trap takes it at.
 * A refusal (EXEC_INVALID_ACTION) stops the run: compiled routines refuse
 * for points they do not cover, and the one compiled TCF to itself
 * (Comanche055 TVCEXECUTIVE 16,2763, marked NOT USED) refuses from its
 * routine's entry, with the routine's earlier writes kept (#607). The night
 * watchman and the rupt lock are not modelled (docs/FLIGHT-RESTART.md). */
#include "resident_flight.h"
int main(void)
{
    if (!rk_boot() && !rk_gojam())
        for (;;) {
            if (rk_tc_trap()) {
                if (rk_gojam()) break;
            } else if (rk_resident_step(AGC_TRANSFER_NONE) != EXEC_OK)
                break;
        }
    for (;;) {
    }
}
