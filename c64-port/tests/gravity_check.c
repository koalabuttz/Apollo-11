/* SPDX-License-Identifier: MIT */
#include "gravity_cases.h"
#define GRAVITY_FROZEN_IMPLEMENTATION
#include "gravity_vectors.h"
unsigned gv_check(gravity_case *c) {
    uint16_t n,row=0; unsigned failures=0;
    for(n=0;n<GV_TARGET_COUNT;++n) {
        uint8_t step;
        gv_prepare(c,n);
        for(step=0;step<gv_steps(n);++step) {
            frozen_words expected;
            uint16_t words[GV_WORD_COUNT],word; unsigned i;
            gv_next(c,n,step);
            if(gv_integrated(n)) gv_prefix(c,n);
            if(gv_apply(c)!=P66_PUSH_OK) ++failures;
            gv_words(c,words);
            if(!gravity_reference_begin(&expected,row)) ++failures;
            else {
                for(i=0;i<GV_WORD_COUNT;++i) {
                    if(!frozen_words_next(&expected,&word)) { ++failures; break; }
                    if(words[i]!=word) ++failures;
                }
                if(!frozen_words_finished(&expected)) ++failures;
            }
            ++row;
        }
    }
    if(row!=GV_TARGET_ROWS) ++failures;
    return failures;
}
