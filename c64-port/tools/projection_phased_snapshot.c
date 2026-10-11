/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "projection_phased_cases.h"
int main(int argc,char **argv){uint16_t n;for(n=0;n<PJ_CASES;++n){projection_case c;uint8_t phase;pj_input(&c,n);if(argc>1){uint16_t w[PJ_WORDS];unsigned i;(void)argv;pj_words(&c,w);printf("%u 3 0",n);for(i=0;i<PJ_WORDS;++i)printf(" %u",w[i]);puts("");continue;}for(phase=0;phase<3u;++phase){uint16_t w[PJ_WORDS];unsigned i,status=pjp_apply(&c,phase);pj_words(&c,w);printf("%u %u %u",n,phase,status);for(i=0;i<PJ_WORDS;++i)printf(" %u",w[i]);puts("");}}return 0;}
