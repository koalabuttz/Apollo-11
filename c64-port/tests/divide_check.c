/* SPDX-License-Identifier: MIT */
#include "divide_cases.h"
#include "divide_vectors.h"
unsigned dv_check(divide_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<DV_TARGET_COUNT;++n) {
        uint8_t step;
        dv_prepare(c,n);
        for(step=0;step<dv_steps(n);++step) {
            uint16_t w[DV_WORD_COUNT]; unsigned i; uint8_t path;
            dv_next(c,n,step);
            if(dv_integrated(n)) dv_prefix(c,n);
            path=(uint8_t)dv_apply(c); dv_words(c,w);
            if(path!=divide_reference_path[row]) ++failures;
            for(i=0;i<DV_WORD_COUNT;++i) if(w[i]!=divide_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=DV_TARGET_ROWS) ++failures;
    return failures;
}
