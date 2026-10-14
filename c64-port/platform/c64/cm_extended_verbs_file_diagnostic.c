/* SPDX-License-Identifier: MIT */
/* cm-extended-verbs-file C64 diagnostic: group IRC_GROUP of the compiled
 * EXTENDED_VERBS (the extended verbs) code (the routines of its KEEP mask,
 * tools/mk/cm-extended-verbs-file.groups) runs its cases
 * (tests/cm_extended_verbs_file_c64_vectors.h), each digest against the yaAGC
 * oracle's. */
#include <stdint.h>
#include "interp_case.h"
#include "cm_extended_verbs_file_c64_vectors.h"
const agc_native_program *irc_program(uint8_t cm);
/* The 4 KB image lives in the RAM under the KERNAL ROM ($e000), so the
 * compiled routines and their interpreter orders have the room below $c000;
 * main banks the KERNAL out with interrupts off before touching it. */
#define S ((apollo_overlay_state *)0xe000)
_Static_assert(sizeof(apollo_overlay_state) <= 0x1ff0, "image fits under the KERNAL");

/* Mailbox contract (tools/vice_run.py): write $c001-$c004 first, then $a5 to $c000 last. */
int main(void) {
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    unsigned failures = 0, checks = 0, i;
    __asm__ volatile("sei");
    *(volatile uint8_t *)1 = 0x35;
    for (i = 0; i < IRC_C64_VECTORS; ++i) {
        uint16_t ie = 0;
        unsigned kind;
        irc_build(S, &irc_c64_cases[i]);
        kind = irc_run(irc_program(irc_c64_cases[i].cm), S, &irc_c64_cases[i], 200, &ie);
        ++checks;
        if (kind != IRC_STOPPED || irc_digest(kind, S, ie) != irc_c64_expected[i]) ++failures;
    }
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
