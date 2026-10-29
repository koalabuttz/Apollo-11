/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "radial_phased_cases.h"
static void emit(radial_case *c,uint16_t n,uint8_t step,uint8_t stage,unsigned status){uint16_t w[RA_WORDS];unsigned i;ra_words(c,w);printf("%u %u %u %u",n,step,stage,status);for(i=0;i<RA_WORDS;++i)printf(" %u",w[i]);puts("");}
int main(void){uint16_t n;for(n=0;n<RA_CASES;++n){radial_case c;uint8_t step;ra_prepare(&c,n);for(step=0;step<ra_steps(n);++step){uint8_t phase;ra_next(&c,n,step);if(ra_integrated(n))ra_prefix(&c,n);emit(&c,n,step,0,0);for(phase=0;phase<9u;++phase){unsigned status=rp_apply(&c,phase);emit(&c,n,step,(uint8_t)(phase+1u),status);if(status==P66_UNIT_ALARM_1302)break;if(phase==1u&&status==P66_UNIT_OVERFLOW)++phase;}}}return 0;}
