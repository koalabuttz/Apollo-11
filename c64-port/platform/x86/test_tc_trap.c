/* SPDX-License-Identifier: MIT */
/* Checks platform/x86/tc_trap.h, the x86 program's TC trap: where it spins,
 * when it takes the GOJAM, and when it does neither (make x86-flight-check
 * builds and runs it). */
#include <stdio.h>
#include <string.h>
#include "tc_trap.h"

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

static agc_word w[2048];

/* Z at VERB69 (43,2037: FB 033's 066000 with the superbank's bit 0100). */
static void at_verb69(void)
{
    memset(w, 0, sizeof w);
    w[5] = 02037u;
    w[4] = 066000u;
}

int main(void)
{
    uint8_t spin = 0;
    /* A spin through a T3 step, then the GOJAM. */
    at_verb69();
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T4, &spin) == TC_TRAP_SPIN && spin == 2u,
          "VERB69 from the T4 step spins and holds the T3 step");
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T4, &spin) == TC_TRAP_GOJAM && spin == 0u,
          "after a spin that held the T3 step, the GOJAM");
    spin = 0;
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T3, &spin) == TC_TRAP_SPIN && spin == 2u,
          "VERB69 from the T3 step holds it");
    /* A spin from the T5 step missed the T3 step: no GOJAM, a new spin. */
    spin = 0;
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T5, &spin) == TC_TRAP_SPIN && spin == 1u,
          "VERB69 from the T5 step spins without the T3 step");
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T4, &spin) == TC_TRAP_SPIN && spin == 2u,
          "after a spin without the T3 step, another spin");
    /* An interrupt between: no GOJAM, and the spin starts over. */
    check(tc_trap_step(w, 0100u, 1, RK_PHASE_T4, &spin) == TC_TRAP_NONE && spin == 0u,
          "in an interrupt, neither, and the spin is cleared");
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T4, &spin) == TC_TRAP_SPIN,
          "after the interrupt, a new spin");
    /* Not VERB69: neither. */
    spin = 0;
    check(tc_trap_step(w, 0u, 0, RK_PHASE_T4, &spin) == TC_TRAP_NONE && spin == 0u,
          "FB 033 without the superbank bit (bank 033) is not VERB69");
    w[4] = 064000u;
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T4, &spin) == TC_TRAP_NONE,
          "another bank is not VERB69");
    at_verb69();
    w[5] = 02040u;
    check(tc_trap_step(w, 0100u, 0, RK_PHASE_T4, &spin) == TC_TRAP_NONE,
          "another address is not VERB69");
    printf("TC trap: %s\n", failures ? "checks failed" : "all checks pass");
    return failures != 0;
}
