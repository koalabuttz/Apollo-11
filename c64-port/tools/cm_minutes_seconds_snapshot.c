/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_minutes_seconds_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;uint8_t p;for(n=0;n<CM_MINUTES_SECONDS_CASES;++n){cm_minutes_seconds_input(&s,&io,n);for(p=0;p<6u;++p){if(!cm_minutes_seconds_phase(&s,&io,p)){fprintf(stderr,"nativeMS case%u p%u unsupportedPC%oBB%o\n",n,p,s.words[5],s.words[6]);return 2;}printf("%u 1",n*6u+p);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}}return 0;}
