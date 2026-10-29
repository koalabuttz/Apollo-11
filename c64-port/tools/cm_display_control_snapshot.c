/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_display_control_cases.h"
static apollo_overlay_state s;
int main(void){uint16_t n,i;for(n=0;n<CM_DISPLAY_CONTROL_CASES;++n){cm_native_io io;unsigned status;cm_display_control_input(&s,&io,n);status=(unsigned)cm_display_control_enter(&s,&io);printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
