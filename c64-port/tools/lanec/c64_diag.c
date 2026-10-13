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

/* lanec_hash (tests/lanec_case.c) with a 16-entry table: the same CRC-32,
 * 960 bytes smaller, for the programs that only just fit. */
static const uint32_t nibble[16] = {
    0x00000000u, 0x1db71064u, 0x3b6e20c8u, 0x26d930acu, 0x76dc4190u, 0x6b6b51f4u, 0x4db26158u, 0x5005713cu,
    0xedb88320u, 0xf00f9344u, 0xd6d6a3e8u, 0xcb61b38cu, 0x9b64c2b0u, 0x86d3d2d4u, 0xa00ae278u, 0xbdbdf21cu };
static uint32_t crc_byte(uint32_t crc, uint8_t b)
{
    crc ^= b;
    crc = nibble[crc & 15u] ^ (crc >> 4);
    return nibble[crc & 15u] ^ (crc >> 4);
}
static uint32_t hash(const lanec_io *io, enum lanec_status status)
{
    uint32_t crc = 0xffffffffu;
    uint16_t i, v;
    for (i = 0; i < APOLLO_SHARED_WORDS + LANEC_CHANNELS + 3u; ++i) {
        if (i < APOLLO_SHARED_WORDS) v = s.words[i];
        else if (i < APOLLO_SHARED_WORDS + LANEC_CHANNELS) v = io->channel[i - APOLLO_SHARED_WORDS];
        else if (i == APOLLO_SHARED_WORDS + LANEC_CHANNELS) v = io->superbank;
        else if (i == APOLLO_SHARED_WORDS + LANEC_CHANNELS + 1u) v = io->interrupt_enabled;
        else v = (uint16_t)status;
        crc = crc_byte(crc_byte(crc, (uint8_t)v), (uint8_t)(v >> 8));
    }
    return crc ^ 0xffffffffu;
}
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
        if (hash(&io, status) != lanec_frozen[i].hash) ++failures;
    }
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
