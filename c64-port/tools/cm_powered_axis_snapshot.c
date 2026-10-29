/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_powered_axis_cases.h"
static int row(apollo_overlay_state *m,agc_word irq,uint32_t n,unsigned phase){uint16_t h[4];h[0]=(uint16_t)n;h[1]=(uint16_t)(n>>16);h[2]=(uint16_t)phase;h[3]=irq;return fwrite(h,sizeof h,1,stdout)!=1||fwrite(m->words,sizeof m->words,1,stdout)!=1;}
int main(void){uint32_t n;for(n=0;n<CM_POWERED_AXIS_CASES;++n){apollo_overlay_state m;agc_word irq;cm_powered_axis_input(&m,&irq,n);if(cm_powered_axis(&m)!=CM_POWERED_AXIS_OK||row(&m,irq,n,0)||cm_powered_axis_return(&m)!=CM_POWERED_AXIS_OK||row(&m,irq,n,1))return 2;}return 0;}
