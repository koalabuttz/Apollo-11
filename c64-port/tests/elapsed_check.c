/* SPDX-License-Identifier: MIT */
#include "elapsed_cases.h"
#include "elapsed_vectors.h"
unsigned el_check(elapsed_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<EL_TARGET_COUNT;++n) {
        uint8_t step;
        el_prepare(c,n);
        for(step=0;step<el_steps(n);++step) {
            uint16_t w[EL_WORD_COUNT]; unsigned i; uint8_t path;
            el_next(c,n,step);
            if(el_integrated(n)) el_prefix(c,n);
            path=el_apply(c); el_words(c,w);
            if(path!=elapsed_reference_path[row]) ++failures;
            for(i=0;i<EL_WORD_COUNT;++i) if(w[i]!=elapsed_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=EL_TARGET_ROWS) ++failures;
    return failures;
}
