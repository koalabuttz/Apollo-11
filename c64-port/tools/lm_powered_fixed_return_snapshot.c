/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_powered_fixed_return_cases.h"
#include "lm_powered_fixed_return.h"
int main(void){uint32_t n;uint8_t origin;for(origin=0;origin<4u;++origin)for(n=0;n<LM_POWERED_FIXED_RETURN_CASES;++n){apollo_overlay_state m;agc_word irq;uint16_t h[3];lm_powered_fixed_return_input(&m,&irq,(uint16_t)n,origin);if(lm_powered_fixed_return(&m)!=LM_POWERED_FIXED_RETURN_OK)return 2;h[0]=(uint16_t)n;h[1]=origin;h[2]=irq;if(fwrite(h,sizeof h,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return 0;}
