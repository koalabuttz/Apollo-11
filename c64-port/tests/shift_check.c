/* SPDX-License-Identifier: MIT */
#include "shift_cases.h"
#define SHIFT_FROZEN_IMPLEMENTATION
#include "shift_vectors.h"
unsigned sh_check(shift_case *c) {
    unsigned failures=0;uint16_t row;
    for(row=0;row<SH_TARGET_ROWS;++row) {
        uint16_t words[SH_WORD_COUNT];unsigned i;frozen_words reader;
        sh_prepare(c,sh_target_number(row));if(!sh_apply(c))++failures;sh_words(c,words);
        if(!shift_reference_begin(&reader,row))++failures;
        for(i=0;i<SH_WORD_COUNT;++i) {
            uint16_t expected;
            if(!frozen_words_next(&reader,&expected)||words[i]!=expected)++failures;
        }
        if(!frozen_words_finished(&reader))++failures;
    }
    return failures;
}
