/* SPDX-License-Identifier: MIT */
#include "store_cases.h"
#include "store_vectors.h"
unsigned st_check(store_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<ST_TARGET_COUNT+ST_TARGET_EXTRA;++n) {
        uint8_t step;
        uint16_t index=n<ST_TARGET_COUNT ? n : st_target_extra((uint8_t)(n-ST_TARGET_COUNT));
        st_prepare(c,index);
        for(step=0;step<st_steps(index);++step) {
            uint16_t w[ST_WORD_COUNT]; unsigned i; uint8_t path=0;
            st_next(c,index,step);
            if(st_integrated(index)) path=st_prefix(c,index);
            if(st_apply(c)!=P66_STORE_OK) ++failures;
            el_words(c,w);
            if(path!=store_reference_path[row]) ++failures;
            for(i=0;i<ST_WORD_COUNT;++i) if(w[i]!=store_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=ST_TARGET_ROWS) ++failures;
    return failures;
}
