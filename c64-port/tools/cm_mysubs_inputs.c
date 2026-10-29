/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_mysubs_cases.h"
int main(void){uint16_t h[3];while(fread(h,sizeof h,1,stdin)==1){apollo_overlay_state m;agc_word irq;uint32_t n=(uint32_t)h[0]+((uint32_t)h[1]<<16);cm_mysubs_input(&m,&irq,n,(uint8_t)h[2]);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
