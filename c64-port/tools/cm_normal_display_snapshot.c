/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_normal_display_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;uint8_t p,count;for(n=0;n<CM_NORMAL_DISPLAY_CASES;++n){cm_normal_display_input(&s,&io,n);count=n<512u?30u:26u;for(p=0;p<count;++p){if(!cm_normal_display_step(&s,&io,p)){fprintf(stderr,"native normal case%u p%u unsupportedPC%oBB%o\n",n,p,s.words[5],s.words[6]);return 2;}printf("%u 1",n*30u+p);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}}return 0;}
