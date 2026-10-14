/* SPDX-License-Identifier: MIT */
#include "resident_cdu_cases.h"
static uint32_t mix(uint32_t h, uint16_t v) {
    h = (h ^ (v & 0xffu)) * 16777619u;
    return (h ^ (uint16_t)(v >> 8)) * 16777619u;
}
static uint32_t hash(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; x ^= x >> 16;
    return x;
}
/* Channel 14 stays the same for runs of 8 or 32 periods: no drive, one
 * axis, two or all three. A write is rare: mostly small (at most two
 * bursts), now and then +0 or -0, and seldom large (a coarse alignment, up
 * to 1023 pulses, or the 16383 extremes). */
void rc_input(uint32_t n, agc_word *ch14, uint8_t set[3], agc_word write[3]) {
    static const agc_word edge[6] = {0, 077777, 1, 077776, 037777, 040000};
    uint32_t seg = n >> 5, r = hash(seg * 2654435761u + 0x43445531u);
    uint8_t k;
    if (r % 4u == 0u) seg = (n >> 3);
    r = hash(seg ^ 0x5a17u);
    *ch14 = (agc_word)(r % 5u == 0u ? 0u : ((r >> 3) % 7u + 1u) << 12);
    for (k = 0; k < 3u; ++k) {
        uint32_t w = hash(n * 3u + k + 0x77u);
        agc_word v;
        set[k] = (uint8_t)(w % 16u == 0u);
        switch ((w >> 4) % 64u) {
        case 0: v = (w >> 10) % 8u ? (agc_word)((w >> 10) % 1024u) : edge[(w >> 13) % 4u + 2u]; break;
        case 1: case 2: case 3: case 4: case 5: case 6: v = edge[(w >> 10) % 2u]; break;
        case 7: case 8: case 9: case 10: case 11: case 12: v = (agc_word)((w >> 10) % 200u); break;
        default: v = (agc_word)((w >> 10) % 49u); break;
        }
        if ((w >> 20) & 1u && v != 0u && v != 077777u && v != 037777u && v != 040000u) v = (agc_word)(v ^ 077777u);
        write[k] = v;
    }
}
uint32_t rc_digest(const agc_word *cmd) {
    uint32_t h = RC_BASIS;
    uint8_t k;
    for (k = 0; k < 3u; ++k) h = mix(h, (uint16_t)(cmd[k] & 077777u));
    return h;
}
uint32_t rc_fold(uint32_t h, uint32_t digest) { return mix(mix(h, (uint16_t)digest), (uint16_t)(digest >> 16)); }
