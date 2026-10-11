/* SPDX-License-Identifier: MIT */
#include "dot_cases.h"
#define DOT_FROZEN_IMPLEMENTATION
#include "dot_vectors.h"
unsigned dt_check(dot_case *c) {
    unsigned failures=0;uint16_t row;
    for(row=0;row<DT_TARGET_ROWS;++row) {
        uint16_t words[DT_WORD_COUNT];unsigned i;frozen_words reader;
        dt_prepare(c,dt_target_number(row));if(!dt_apply(c))++failures;dt_words(c,words);
        if(!dot_reference_begin(&reader,row))++failures;
        for(i=0;i<DT_WORD_COUNT;++i) {
            uint16_t expected;
            if(!frozen_words_next(&reader,&expected)||words[i]!=expected)++failures;
        }
        if(!frozen_words_finished(&reader))++failures;
    }
    return failures;
}
