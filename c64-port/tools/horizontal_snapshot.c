/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "horizontal_cases.h"
int main(int argc,char **argv){uint16_t n;(void)argv;for(n=0;n<HH_CASES;++n){horizontal_case c;uint8_t phase;hh_input(&c,n);if(argc>1){uint16_t w[HH_WORDS];unsigned i;hh_words(&c,w);printf("%u 8 0",n);for(i=0;i<HH_WORDS;++i)printf(" %u",w[i]);puts("");continue;}for(phase=0;phase<8u;++phase){uint16_t w[HH_WORDS];unsigned i,status=hh_apply(&c,phase);hh_words(&c,w);printf("%u %u %u",n,phase,status);for(i=0;i<HH_WORDS;++i)printf(" %u",w[i]);puts("");if(status==P66_HORIZONTAL_ALARM_1302)break;}}return 0;}
