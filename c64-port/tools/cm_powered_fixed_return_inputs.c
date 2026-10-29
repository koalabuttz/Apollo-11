/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "cm_powered_fixed_return_cases.h"
int main(int argc,char **argv){uint16_t n;uint8_t origin=argc>1?(uint8_t)strtoul(argv[1],0,0):0;while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;agc_word irq;cm_powered_fixed_return_input(&m,&irq,n,origin);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
