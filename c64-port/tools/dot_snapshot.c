/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dot_cases.h"
int main(void) {uint16_t n;for(n=0;n<DT_CASE_COUNT;++n) {
    dot_case c;uint16_t w[DT_WORD_COUNT];unsigned i;
    dt_prepare(&c,n);if(!dt_apply(&c))return 2;dt_words(&c,w);
    printf("%u",n);for(i=0;i<DT_WORD_COUNT;++i)printf(" %u",w[i]);puts("");
}return 0;}
