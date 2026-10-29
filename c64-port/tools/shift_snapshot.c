/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "shift_cases.h"
int main(void) {uint16_t n;for(n=0;n<SH_CASE_COUNT;++n) {
    shift_case c;uint16_t w[SH_WORD_COUNT];unsigned i;
    sh_prepare(&c,n);if(!sh_apply(&c))return 2;sh_words(&c,w);
    printf("%u",n);for(i=0;i<SH_WORD_COUNT;++i)printf(" %u",w[i]);puts("");
}return 0;}
