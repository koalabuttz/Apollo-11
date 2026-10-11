/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "matrix_cases.h"
static apollo_overlay_state s;
int main(int argc,char **argv){uint16_t n,i;uint8_t column,phase;enum apollo_fixed_mission mission=argc>1&&argv[1][0]=='L'?APOLLO_FIXED_LM:APOLLO_FIXED_CM;for(n=0;n<MATRIX_CASES;++n)for(column=0;column<2u;++column){agc_word irq,super;matrix_input(&s,&irq,&super,n,mission,column);for(phase=0;phase<4u;++phase){unsigned status=matrix_native_phase(&s,mission,column,s.words+01000,phase);printf("%u %u",n*8u+column*4u+phase,status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",super,irq);}}return 0;}
