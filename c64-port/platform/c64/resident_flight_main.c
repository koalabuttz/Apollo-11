/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident's entry: boot, the hardware restart (GOJAM:
 * the rope's power-on code from 04000, as the AGC starts when it is
 * switched on; with an image of zeros it ends in a fresh start), then
 * resident steps until the
 * program reaches a point that neither the kernel nor the module store
 * resolves. V69's TC VERB69 spins in rk_resident_step until the AGC's TC
 * trap restarts it (rk_spin, rk_gojam; docs/FLIGHT-RESTART.md, The TC trap).
 * No other step spins: a compiled routine refuses a TC or TCF to itself
 * (LM T6JOBCHK +3 17,2032 with TIME6 negative, src/lm_t6rupt.c; Comanche055
 * TVCEXECUTIVE 16,2763, marked NOT USED, from its routine's entry, #607), and
 * a refusal (EXEC_INVALID_ACTION) stops the run, as compiled routines refuse
 * for points they do not cover. The night watchman and the rupt lock are not
 * modelled (docs/FLIGHT-RESTART.md). */
#include "resident_flight.h"
int main(void)
{
    if (!rk_boot() && !rk_gojam())
        while (rk_resident_step(AGC_TRANSFER_NONE) == EXEC_OK) {
        }
    for (;;) {
    }
}
