/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "sum_cases.h"
int main(void) {
    uint16_t n;uint8_t operation;
    for(n=0;n<SU_CASE_COUNT;++n)for(operation=0;operation<4u;++operation) {
        vector_case c;uint16_t words[SU_WORD_COUNT],path;unsigned i;
        su_prepare(&c,n,operation);path=su_apply(&c,operation);ve_words(&c,words);
        printf("%u %u %u",n,operation,path);for(i=0;i<SU_WORD_COUNT;++i)printf(" %u",words[i]);puts("");
    }
    return 0;
}
