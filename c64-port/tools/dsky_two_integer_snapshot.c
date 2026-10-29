/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_two_integer_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,k;enum dsky_nv_mission mission;p66_bias_io io;for(n=0;n<DSKY_TWO_INTEGER_CASES;++n){dsky_two_integer_input(&s,&io,n,&mission);for(k=0;k<5u;++k){if(dsky_two_integer(&s,&io,mission,(enum dsky_two_integer_stage)k)!=P66_BIAS_OK){fprintf(stderr,"unsupported %u/%u PC%o\n",n,k,s.words[5]);return 2;}printf("%u",n*8u+k);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
