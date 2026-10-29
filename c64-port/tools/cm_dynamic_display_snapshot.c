/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_dynamic_display_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;uint8_t p;for(n=0;n<CM_DYNAMIC_DISPLAY_CASES;++n){cm_dynamic_display_input(&s,&io,n);for(p=0;p<53u;++p){if(!cm_dynamic_display_step(&s,&io,p)){fprintf(stderr,"native dynamic case%u p%u unsupportedPC%oBB%o\n",n,p,s.words[5],s.words[6]);return 2;}printf("%u 1",n*53u+p);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}}return 0;}
