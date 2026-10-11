/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_single_trig_cases.h"
static int row(apollo_overlay_state *m,agc_word irq,uint16_t n,uint8_t selector,uint8_t phase){uint16_t h[4];h[0]=n;h[1]=selector;h[2]=phase;h[3]=irq;return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(void){uint8_t selector;for(selector=0;selector<2u;++selector){uint32_t n;for(n=0;n<LM_SINGLE_TRIG_CASES;++n){apollo_overlay_state m;agc_word irq;lm_single_trig_input(&m,&irq,(uint16_t)n,selector);if(lm_single_trig(&m,selector)!=LM_SINGLE_TRIG_OK||row(&m,irq,(uint16_t)n,selector,0)||lm_single_trig_return(&m)!=LM_SINGLE_TRIG_OK||row(&m,irq,(uint16_t)n,selector,1))return 2;}}return 0;}
