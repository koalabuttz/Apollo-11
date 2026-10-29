/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_powered_axis_cases.h"
int main(void){uint16_t h[2];while(fread(h,sizeof h,1,stdin)==1){apollo_overlay_state m;agc_word irq;uint32_t n=(uint32_t)h[0]+((uint32_t)h[1]<<16);lm_powered_axis_input(&m,&irq,n);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
