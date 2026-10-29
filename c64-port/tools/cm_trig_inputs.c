/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_trig_cases.h"
int main(void){uint16_t head[2];while(fread(head,sizeof head,1,stdin)==1){apollo_overlay_state m;agc_word irq;cm_trig_input(&m,&irq,head[0],(uint8_t)head[1]);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return 0;}
