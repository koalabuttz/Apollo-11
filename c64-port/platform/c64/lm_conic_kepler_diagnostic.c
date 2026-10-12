/* SPDX-License-Identifier: MIT */
/* One C64 phase diagnostic group (LM_CONIC_KEPLER_GROUP 1..4): the whole routine
 * with its interpretive bodies is about 64 KB of 6502 code, so each PRG links
 * only its group's phases. */
#include <stdint.h>
#include "lm_conic_kepler_phase_target.h"

/* Mailbox contract (tools/vice_run.py): write $c001-$c004 first, then $a5 to $c000 last. */
int main(void) {
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    unsigned completed, failures;
    int status;
    /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
     * two 4 KB images live in the RAM under KERNAL at $e000 and $f000. */
    __asm__("sei");
    *(volatile uint8_t *)1u = 0x35u;
    status = lm_conic_kepler_phase_check((apollo_overlay_state *)0xe000u,
                                      (apollo_overlay_state *)0xf000u, &completed);
    failures = status ? 1u : 0u;
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)completed; mail[4] = (uint8_t)(completed >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
