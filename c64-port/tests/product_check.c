/* SPDX-License-Identifier: MIT */
#include "product_cases.h"
#include "product_vectors.h"
unsigned pr_check(product_case *c) {
    uint16_t index,row=0; unsigned failures=0;
    for(index=0;index<PR_TARGET_COUNT;++index) {
        uint8_t sub;
        for(sub=0;sub<2u;++sub) {
            uint16_t w[PR_WORD_COUNT]; unsigned i;
            pr_prepare(c,pr_target_number(index),sub); (void)pr_apply(c,sub); ve_words(c,w);
            for(i=0;i<PR_WORD_COUNT;++i) if(w[i]!=product_reference_words[row][i]) ++failures;
            ++row;
        }
    }
    if(row!=PR_TARGET_ROWS) ++failures;
    return failures;
}
