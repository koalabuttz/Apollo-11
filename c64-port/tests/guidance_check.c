/* SPDX-License-Identifier: MIT */
#include "guidance_cases.h"
#include "guidance_vectors.h"
unsigned gn_check(guidance_case *c) {
    uint16_t n, row = 0; unsigned failures = 0;
    for (n = 0; n < GN_TARGET_COUNT; ++n) {
        uint8_t step;
        gn_prepare(c,n);
        for (step = 0; step < gn_steps(n); ++step) {
            uint16_t words[6]; unsigned i;
            gn_next(c,n,step); gn_apply(c); gn_words(c,words);
            for (i = 0; i < 6; ++i) if (words[i] != guidance_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if (row != GN_TARGET_ROWS) ++failures;
    return failures;
}
