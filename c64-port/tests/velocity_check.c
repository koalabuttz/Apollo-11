/* SPDX-License-Identifier: MIT */
#include "velocity_cases.h"
#define VELOCITY_FROZEN_IMPLEMENTATION
#include "velocity_vectors.h"
unsigned vl_check(velocity_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=VL_FIRST_CASE;n<VL_LAST_CASE;++n) {
        uint8_t step;
        vl_prepare(c,n);
        for(step=0;step<vl_steps(n);++step) {
            frozen_words expected;
            uint16_t words[VL_WORD_COUNT],word; unsigned i;
            vl_next(c,n,step);
            if(vl_integrated(n)) vl_prefix(c,n);
            if(vl_apply(c)!=P66_VELOCITY_OK) ++failures;
            vl_words(c,words);
            if(!velocity_reference_begin(&expected,row)) ++failures;
            else {
                for(i=0;i<VL_WORD_COUNT;++i) {
                    if(!frozen_words_next(&expected,&word)) { ++failures; break; }
                    if(words[i]!=word) ++failures;
                }
                if(!frozen_words_finished(&expected)) ++failures;
            }
            ++row;
        }
    }
    if(row!=VELOCITY_FROZEN_ROWS) ++failures;
    return failures;
}
