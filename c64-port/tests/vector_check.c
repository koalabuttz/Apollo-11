/* SPDX-License-Identifier: MIT */
#include "vector_cases.h"
#include "vector_vectors.h"
unsigned ve_check(vector_case *c) {
    uint16_t index,row=0; unsigned failures=0;
    for(index=0;index<VE_TARGET_COUNT;++index) {
        uint8_t sub;
        for(sub=0;sub<2u;++sub) {
            uint16_t w[VE_WORD_COUNT],path; unsigned i;
            ve_prepare(c,ve_target_number(index),sub); path=ve_apply(c,sub); ve_words(c,w);
            if(path!=vector_reference_path[row]) ++failures;
            for(i=0;i<VE_WORD_COUNT;++i) if(w[i]!=vector_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=VE_TARGET_ROWS) ++failures;
    return failures;
}
