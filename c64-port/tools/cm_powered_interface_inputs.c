/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_powered_interface_cases.h"
int main(void){uint16_t h[2];while(fread(h,sizeof h,1,stdin)==1){apollo_overlay_state m;agc_word irq;cm_powered_interface_input(&m,&irq,h[0],(uint8_t)h[1]);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
