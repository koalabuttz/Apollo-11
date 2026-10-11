/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "matrix_cases.h"
static apollo_overlay_state s;
int main(int argc,char **argv){uint16_t n,i;uint8_t c,p;enum apollo_fixed_mission m=argc>1&&argv[1][0]=='L'?APOLLO_FIXED_LM:APOLLO_FIXED_CM;for(n=0;n<MATRIX_CASES;++n)for(c=0;c<2u;++c){agc_word irq,super;matrix_input(&s,&irq,&super,n,m,c);for(p=0;p<4u;++p){printf("%u 0",n*8u+c*4u+p);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",super,irq);}}return 0;}
