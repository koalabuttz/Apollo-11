/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "throttle_service_cases.h"
int main(void){uint16_t n;for(n=0;n<TS_CASES;++n){throttle_service_case c;uint16_t w[TS_WORDS],i;unsigned result;ts_input(&c,n);result=ts_apply(&c);ts_words(&c,w);printf("%u %u",n,result);for(i=0;i<TS_WORDS;++i)printf(" %u",w[i]);puts("");}return 0;}
