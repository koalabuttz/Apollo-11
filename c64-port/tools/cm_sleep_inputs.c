/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_sleep_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_SLEEP_CASES;++n){cm_native_io io;cm_sleep_input(&s,&io,n);printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u\n",io.superbank,io.interrupt_enabled,io.channel11);}return 0;}
