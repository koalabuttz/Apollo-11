/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "velocity_phased_cases.h"
static void row(const velocity_phased_case *c,uint16_t n,uint8_t step,uint8_t stage){uint16_t w[VLP_WORDS];unsigned i;vlp_words(c,w);printf("%u %u %u",n,step,stage);for(i=0;i<VLP_WORDS;++i)printf(" %u",w[i]);puts("");}
int main(void){uint16_t n;for(n=0;n<VL_CASE_COUNT;++n){velocity_phased_case c;uint8_t step;vl_prepare(&c.previous,n);for(step=0;step<vl_steps(n);++step){uint8_t phase;vl_next(&c.previous,n,step);if(vl_integrated(n))vl_prefix(&c.previous,n);c.addrwd=vl_integrated(n)?01636:0;row(&c,n,step,0);for(phase=0;phase<3u;++phase){if(vlp_apply(&c,phase))return 2;row(&c,n,step,(uint8_t)(phase+1u));}}}return 0;}
