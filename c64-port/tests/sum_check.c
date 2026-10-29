/* SPDX-License-Identifier: MIT */
#include "sum_cases.h"
#define SUM_FROZEN_IMPLEMENTATION
#include "sum_vectors.h"
unsigned su_check(vector_case *c) {
    unsigned failures=0;uint16_t row;
    for(row=0;row<SU_TARGET_ROWS;++row) {
        uint16_t words[SU_WORD_COUNT],path;unsigned i;frozen_words reader;
        su_prepare(c,ve_target_number((uint16_t)(row/4u)),(uint8_t)(row%4u));
        path=su_apply(c,(uint8_t)(row%4u));ve_words(c,words);
        if(path!=sum_reference_path[row]||!sum_reference_begin(&reader,row))++failures;
        for(i=0;i<SU_WORD_COUNT;++i) {
            uint16_t expected;if(!frozen_words_next(&reader,&expected)||words[i]!=expected)++failures;
        }
        if(!frozen_words_finished(&reader))++failures;
    }
    return failures;
}
