/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "scale_cases.h"
int main(void) {
    uint16_t n;
    for(n=0;n<SC_CASE_COUNT;++n) {
        scale_case c; uint8_t step;
        sc_prepare(&c,n);
        for(step=0;step<sc_steps(n);++step) {
            uint16_t w[SC_WORD_COUNT]; unsigned i;
            sc_next(&c,n,step); sc_apply(&c,n); sc_words(&c,w);
            printf("%u %u",n,step);
            for(i=0;i<SC_WORD_COUNT;++i) printf(" %u",w[i]);
            puts("");
        }
    }
    return 0;
}
