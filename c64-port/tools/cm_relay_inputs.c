/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_relay_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_RELAY_CASES;++n){cm_native_io io;cm_relay_output out;cm_relay_input(&s,&io,&out,n);printf("%u 0",n);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,out.wrote_channel10,out.channel10);}return 0;}
