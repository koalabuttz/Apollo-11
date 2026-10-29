/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "vector_cases.h"
int main(void) {
    uint16_t n; uint8_t subtract;
    for(n=0;n<VE_CASE_COUNT;++n) for(subtract=0;subtract<2u;++subtract) {
        vector_case c; uint16_t words[VE_WORD_COUNT],path; unsigned i;
        ve_prepare(&c,n,subtract); path=ve_apply(&c,subtract); ve_words(&c,words);
        printf("%u %u %u",n,subtract,path);
        for(i=0;i<VE_WORD_COUNT;++i) printf(" %u",words[i]);
        puts("");
    }
    return 0;
}
