/* SPDX-License-Identifier: MIT */
/* Lane C C64 diagnostic: runs a frozen subset of a suite's cases through the
 * native routines on the C64 and compares each complete result (image,
 * channels, superbank, IRQ, status) by CRC-32 with the Virtual AGC
 * oracle's result for the same case. Build with
 *   -DLANEC_SUITE=lanec_suite_<name> -DLANEC_FROZEN='"lanec_<name>_c64.h"'
 * The frozen header holds {case, oracle hash} pairs (tools/lanec/freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $c000=$a5. */
#include <stdint.h>
#include <string.h>
#include "lanec_suites.h"
#include LANEC_FROZEN

static apollo_overlay_state s;
#ifdef LANEC_C64_SKIP   /* a program that carries only some frozen rows: nonzero after make for the others */
int LANEC_C64_SKIP(void);
#endif

int main(void)
{
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    lanec_io io;
    lanec_meta meta;
    enum lanec_status status;
    uint16_t i, failures = 0, checks = 0;
    for (i = 0; i < sizeof lanec_frozen / sizeof lanec_frozen[0]; ++i) {
        memset(&meta, 0, sizeof meta);
        LANEC_SUITE.make(lanec_frozen[i].n, &s, &io, &meta);
#ifdef LANEC_C64_SKIP
        if (LANEC_C64_SKIP()) continue;
#endif
        status = LANEC_SUITE.run(&s, &io, meta.mission);
        ++checks;
        if (lanec_hash(&s, &io, status) != lanec_frozen[i].hash) ++failures;
    }
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
