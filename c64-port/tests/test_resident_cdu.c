/* SPDX-License-Identifier: MIT */
/* rk_cdu_drive (src/resident_cdu.c) against yaAGC's IMU CDU drive over the
 * timeline of tests/resident_cdu_cases.h (tests/resident_cdu_vectors.h,
 * frozen by tools/resident_cdu_vectors.py). */
#include <stdio.h>
#include "resident_cdu.h"
#include "resident_cdu_cases.h"
#include "resident_cdu_vectors.h"
int main(void) {
    static apollo_overlay_state s;
    agc_hardware hw = {0};
    uint32_t n, block = RC_BASIS;
    unsigned checks = 0, failures = 0, writes = 0, bursts = 0, k;
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
