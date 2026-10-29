/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "product_cases.h"
int main(void) {
    uint16_t n; uint8_t vector;
    for(n=0;n<PR_CASE_COUNT;++n) for(vector=0;vector<2u;++vector) {
        product_case c; uint16_t words[PR_WORD_COUNT]; unsigned i;
        pr_prepare(&c,n,vector); if(!pr_apply(&c,vector)) return 2; ve_words(&c,words);
        printf("%u %u",n,vector);
        for(i=0;i<PR_WORD_COUNT;++i) printf(" %u",words[i]);
        puts("");
    }
    return 0;
}
