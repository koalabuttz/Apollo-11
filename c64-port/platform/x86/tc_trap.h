/* SPDX-License-Identifier: MIT */
#ifndef TC_TRAP_H
#define TC_TRAP_H
/* The TC trap (docs/FLIGHT-RESTART.md), as the native x86_64 flight
 * program takes it (platform/x86/flight_dispatch.c fd_run). V69's VERB69
 * (43,2037 in both ropes: bank 043 is FB 033 with the superbank's bit
 * 0100) is a TC to itself. There, outside an interrupt, the AGC runs
 * nothing but TCs, so the step spins as the idle loop does, to the end of
 * the clock period, with Q TC VERB69's (02040). The trap's GOJAM comes at
 * the next step when the spin held the period's T3 step and no interrupt
 * came in between, as yaAGC's trap fires at scaler phase 000 after only
 * TCs since phase 020. The C64's rk_resident_step and the host
 * differential's verb69 and spin make the same test.
 * platform/x86/test_tc_trap.c checks it. */
#include "agc_word.h"
#include "resident_rupt.h"

enum { TC_TRAP_NONE, TC_TRAP_SPIN, TC_TRAP_GOJAM };

/* What the step at erasable w (Z at w[5], FB at w[4]) does: a spin, the
 * trap's GOJAM, or neither. spin carries the last step's spin from one
 * call to the next: 2 when it held a T3 step (phase, the next clock step,
 * was T4 or T3), 1 when not. */
static __attribute__((unused)) int tc_trap_step(const agc_word *w, agc_word superbank,
                                                int in_interrupt, uint8_t phase, uint8_t *spin)
{
    uint8_t last = *spin;
    *spin = 0;
    if (in_interrupt) return TC_TRAP_NONE;
    if (last == 2u) return TC_TRAP_GOJAM;
    if ((w[5] & 07777u) != 02037u || w[4] != 066000u || !(superbank & 0100u)) return TC_TRAP_NONE;
    *spin = phase == RK_PHASE_T5 ? 1u : 2u;
    return TC_TRAP_SPIN;
}
#endif
