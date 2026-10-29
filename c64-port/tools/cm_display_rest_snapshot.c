/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_display_rest_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;for(n=0;n<CM_DISPLAY_REST_CASES;++n){cm_display_rest_input(&s,&io,n);if(cm_display_rest(&s,&io)!=EXEC_OK)return 2;printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
