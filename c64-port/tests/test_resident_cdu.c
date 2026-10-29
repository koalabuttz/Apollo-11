/* SPDX-License-Identifier: MIT */
/* rk_cdu_drive (src/resident_cdu.c) against yaAGC's IMU CDU drive over the
 * timeline of tests/resident_cdu_cases.h (tests/resident_cdu_vectors.h,
 * frozen by tools/resident_cdu_vectors.py). */
#include <stdio.h>
#include "resident_cdu.h"
#include "resident_cdu_cases.h"
#include "resident_cdu_vectors.h"
/* The CM optics counters (053, 054): a count drains at most 240 pulses a
 * burst, at once when the drive is idle, then each 75 ms; their channel 14
 * bits alone start it; the IMU counters stay untouched. */
static unsigned optics(void) {
    static apollo_overlay_state s;
    agc_hardware hw = {0};
    unsigned fail = 0, n;
    rk_cdu_count = 5u;
    rk_cdu = (rk_cdu_state){{0}, 0, 0};
    s.words[053] = 300u;               /* +300 TVCYAW */
    s.words[054] = (agc_word)(077777u - 5u); /* -5 TVCPITCH */
    s.words[050] = 7u;                 /* not driven: bit 15 clear */
    hw.channel14 = 06000u;
    rk_cdu_drive(&s, &hw);
    if (s.words[053] || s.words[054] || rk_cdu.owed[3] != 60 || rk_cdu.owed[4] || s.words[050] != 7u || !rk_cdu.active) ++fail;
    for (n = 0; n < 7u; ++n) rk_cdu_drive(&s, &hw);  /* 75 ms later */
    if (rk_cdu.owed[3] || rk_cdu.active) ++fail;
    hw.channel14 = 0u;
    s.words[053] = 9u;
    rk_cdu_drive(&s, &hw);                           /* bits clear: nothing */
    if (s.words[053] != 9u) ++fail;
    rk_cdu_count = 3u;
    rk_cdu = (rk_cdu_state){{0}, 0, 0};
    return fail;
}
/* The gyro drive: on channel 14 bit 10, GYROCMD (047) less 32 a period,
 * never past 0; a negative count (-0 too) or bit 10 clear sends nothing;
 * the CDU drive state stays as it was. */
static unsigned gyro(void) {
    static apollo_overlay_state s;
    agc_hardware hw = {0};
    unsigned fail = 0;
    rk_cdu = (rk_cdu_state){{0}, 0, 0};
    s.words[047] = 70u;
    hw.channel14 = 01000u;
    rk_cdu_drive(&s, &hw);
    if (s.words[047] != 38u || rk_cdu.active || rk_cdu.elapsed != 4u) ++fail;
    rk_cdu_drive(&s, &hw);
    rk_cdu_drive(&s, &hw);
    if (s.words[047]) ++fail;
    s.words[047] = 040000u;
    rk_cdu_drive(&s, &hw);
    if (s.words[047] != 040000u) ++fail;
    s.words[047] = 077777u;
    rk_cdu_drive(&s, &hw);
    if (s.words[047] != 077777u) ++fail;
    s.words[047] = 037777u;
    hw.channel14 = 0700u;
    rk_cdu_drive(&s, &hw);
    if (s.words[047] != 037777u) ++fail;
    hw.channel14 = 01740u;
    rk_cdu_drive(&s, &hw);
    if (s.words[047] != 037777u - 32u) ++fail;
    rk_cdu = (rk_cdu_state){{0}, 0, 0};
    return fail;
}
int main(void) {
    static apollo_overlay_state s;
    agc_hardware hw = {0};
    uint32_t n, block = RC_BASIS;
    unsigned checks = 0, failures = optics(), writes = 0, bursts = 0, k, g = gyro();
    if (failures) fprintf(stderr, "CM optics counters: %u checks fail\n", failures);
    if (g) fprintf(stderr, "gyro drive: %u checks fail\n", g);
    failures += g;
    for (n = 0; n < RC_PERIODS; ++n) {
        agc_word ch14, write[3];
        uint8_t set[3], was = rk_cdu.active;
        rc_input(n, &ch14, set, write);
        for (k = 0; k < 3u; ++k)
            if (set[k]) { s.words[050u + k] = write[k]; ++writes; }
        hw.channel14 = ch14;
        rk_cdu_drive(&s, &hw);
        bursts += was != rk_cdu.active;
        block = rc_fold(block, rc_digest(&s.words[050]));
        if (n % RC_BLOCK == RC_BLOCK - 1u) {
            ++checks;
            if (block != rc_vectors[n / RC_BLOCK]) {
                ++failures;
                fprintf(stderr, "CDU periods %u-%u differ from yaAGC\n", (unsigned)(n + 1u - RC_BLOCK), (unsigned)n);
            }
            block = RC_BASIS;
        }
    }
    printf("%s: resident-cdu: %u periods, %u counter writes, %u drive start/stop changes; %u checks, %u failures\n",
           failures ? "FAIL" : "PASS", (unsigned)RC_PERIODS, writes, bursts, checks, failures);
    return failures ? 1 : 0;
}
