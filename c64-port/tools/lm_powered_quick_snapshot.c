/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_powered_quick_cases.h"
#include "lm_powered_quick.h"
#include "lm_powered_quick.h"
static int row(apollo_overlay_state *m,agc_word irq,uint16_t n,uint16_t phase){uint16_t h[3]={n,phase,irq};return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(void){uint32_t n;for(n=0;n<LM_POWERED_QUICK_CASES;++n){apollo_overlay_state m;agc_word irq;lm_powered_quick_input(&m,&irq,(uint16_t)n);if(lm_powered_quick(&m,&irq)!=LM_POWERED_QUICK_OK||row(&m,irq,(uint16_t)n,0)||lm_powered_quick_return(&m)!=LM_POWERED_QUICK_OK||row(&m,irq,(uint16_t)n,1))return 2;}return 0;}
