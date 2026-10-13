/* SPDX-License-Identifier: MIT */
/* kalcmanu-steering C64 diagnostic: group IRC_GROUP of the compiled
 * KALCMANU_STEERING (Luminary099 and Comanche055) (one mission, the routines of its KEEP mask,
 * tools/mk/kalcmanu-steering.groups) runs its cases
 * (tests/kalcmanu_steering_c64_vectors.h), each digest against the yaAGC
 * oracle's. */
#include <stdint.h>
#include "interp_case.h"
#include "kalcmanu_steering_c64_vectors.h"
const agc_native_program *irc_program(uint8_t cm);
/* The image lives in the RAM under KERNAL ($01=$35, interrupts off). */
#define S ((apollo_overlay_state *)0xe000u)

/* Mailbox contract (tools/vice_run.py): write $c001-$c004 first, then $a5 to $c000 last. */
int main(void) {
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    unsigned failures = 0, checks = 0, i;
    __asm__("sei");
    *(volatile uint8_t *)1u = 0x35u;
    for (i = 0; i < IRC_C64_VECTORS; ++i) {
        uint16_t ie = 0;
        unsigned kind;
        irc_build(S, &irc_c64_cases[i]);
        kind = irc_run(irc_program(irc_c64_cases[i].cm), S, &irc_c64_cases[i], 1000, &ie);  /* (the spec's max_steps) */
        ++checks;
        if (kind != IRC_STOPPED || irc_digest(kind, S, ie) != irc_c64_expected[i]) ++failures;
    }
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
