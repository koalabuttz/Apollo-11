/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_dyndisp_cases.h"
static apollo_overlay_state s;
int main(void){cm_native_io io;uint16_t n,i;for(n=0;n<CM_DYNDISP_CASES;++n){cm_dyndisp_input(&s,&io,n);printf("%u 1",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
