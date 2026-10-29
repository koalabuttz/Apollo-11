/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "unit_cases.h"
int main(void){unit_case c;uint32_t n;for(n=0;n<UNIT_CASES;++n){uint16_t w[UNIT_WORDS];unsigned i,path;unit_input(&c,n);path=unit_apply(&c,(uint8_t)(n%3u));unit_words(&c,w);printf("%u %u",(unsigned)n,path);for(i=0;i<UNIT_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
