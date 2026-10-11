/* SPDX-License-Identifier: MIT */
#include "push_cases.h"
#include "push_vectors.h"
unsigned ps_check(push_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<PS_TARGET_COUNT;++n) {
        uint8_t step;
        ps_prepare(c,n);
        for(step=0;step<ps_steps(n);++step) {
            uint16_t w[PS_WORD_COUNT]; unsigned i;
            ps_next(c,n,step);
            if(ps_integrated(n)) ps_prefix(c,n);
            if(ps_apply(c)!=P66_PUSH_OK) ++failures;
            ps_words(c,w);
            for(i=0;i<PS_WORD_COUNT;++i) if(w[i]!=push_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=PS_TARGET_ROWS) ++failures;
    return failures;
}
