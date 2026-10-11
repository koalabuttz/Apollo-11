/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "trig_cases.h"
int main(void){uint32_t n;for(n=0;n<TG_CASES;++n){trig_case c;uint16_t w[TG_WORDS];unsigned i,status;tg_input(&c,n);status=tg_apply(&c,tg_operation(n));tg_words(&c,w);printf("%u %u",(unsigned)n,status);for(i=0;i<TG_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
