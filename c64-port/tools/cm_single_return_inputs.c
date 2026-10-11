/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_single_return_cases.h"
int main(void){uint16_t n;while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;agc_word irq;cm_single_return_input(&m,&irq,n);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return 0;}
