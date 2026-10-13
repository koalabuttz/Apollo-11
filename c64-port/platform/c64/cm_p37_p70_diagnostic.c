/* SPDX-License-Identifier: MIT */
/* cm-p37-p70 C64 diagnostic: group IRC_GROUP of the compiled
 * Comanche055 P37_P70 (one mission, the routines of its KEEP mask,
 * tools/mk/cm-p37-p70.groups) runs its cases
 * (tests/cm_p37_p70_c64_vectors.h), each digest against the yaAGC
 * oracle's. */
#include <stdint.h>
#include "interp_case.h"
#include "cm_p37_p70_c64_vectors.h"
const agc_native_program *irc_program(uint8_t cm);
/* The image lives in RAM under the KERNAL ($e000-$efff, interrupts off and
 * $01=$35) so the program's own data and BSS stay below the $c000 mailbox. */
#define s (*(apollo_overlay_state *)0xe000u)

/* Mailbox contract (tools/vice_run.py): write $c001-$c004 first, then $a5 to $c000 last. */
int main(void) {
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    unsigned failures = 0, checks = 0, i;
    __asm__("sei");
    *(volatile uint8_t *)1u = 0x35u;
    for (i = 0; i < IRC_C64_VECTORS; ++i) {
        uint16_t ie = 0;
        unsigned kind;
        irc_build(&s, &irc_c64_cases[i]);
        kind = irc_run(irc_program(irc_c64_cases[i].cm), &s, &irc_c64_cases[i], 10000, &ie);
        ++checks;
        if (kind != IRC_STOPPED || irc_digest(kind, &s, ie) != irc_c64_expected[i]) ++failures;
    }
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
