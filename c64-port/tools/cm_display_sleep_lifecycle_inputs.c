/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_display_sleep_lifecycle_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i,p;for(n=0;n<CM_DISPLAY_SLEEP_LIFECYCLE_CASES;++n){cm_native_io io;cm_display_sleep_lifecycle_input(&s,&io,n);for(p=0;p<4u;++p){printf("%u 0",n*4u+p);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}}return 0;}
