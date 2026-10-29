/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "dsky_minutes_output_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,k;enum dsky_nv_mission mission;p66_bias_io io;for(n=0;n<DSKY_MINUTES_OUTPUT_CASES;++n){dsky_minutes_output_input(&s,&io,n,&mission);for(k=0;k<6u;++k){if(dsky_minutes_output(&s,&io,mission,(enum dsky_minutes_stage)k)!=P66_BIAS_OK){fprintf(stderr,"unsupported %u/%u PC%o\n",n,k,s.words[5]);return 2;}printf("%u",n*16u+k);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
