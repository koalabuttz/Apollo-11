/* SPDX-License-Identifier: MIT */
#include "scale_cases.h"
#include "scale_vectors.h"
unsigned sc_check(scale_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<SC_TARGET_COUNT;++n) {
        uint8_t step;
        sc_prepare(c,n);
        for(step=0;step<sc_steps(n);++step) {
            uint16_t w[SC_WORD_COUNT]; unsigned i;
            sc_next(c,n,step); sc_apply(c,n); sc_words(c,w);
            for(i=0;i<SC_WORD_COUNT;++i) if(w[i]!=scale_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=SC_TARGET_ROWS) ++failures;
    return failures;
}
