/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_scale_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_SCALE_CASES;++n){cm_native_io io;cm_scale_input(&s,&io,n);printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
