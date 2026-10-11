/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "guidance_cases.h"
int main(void) {
    uint16_t n;
    for (n = 0; n < GN_CASE_COUNT; ++n) {
        guidance_case c; uint8_t step;
        gn_prepare(&c,n);
        for (step = 0; step < gn_steps(n); ++step) {
            uint16_t words[6]; unsigned i;
            gn_next(&c,n,step); gn_apply(&c); gn_words(&c,words);
            printf("%u %u %u",n,step,(unsigned)c.operation);
            for (i = 0; i < 6; ++i) printf(" %u",words[i]);
            puts("");
        }
    }
    return 0;
}
