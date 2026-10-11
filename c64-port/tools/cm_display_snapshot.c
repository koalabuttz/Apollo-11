/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_display_cases.h"
static apollo_overlay_state state;
int main(void){uint16_t n,i;for(n=0;n<CM_DISPLAY_CASES;++n){cm_native_io io;exec_address next;enum exec_status status;cm_display_input(&state,&io,n);status=cm_display_run(&state,&io,n,&next);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",state.words[i]);printf(" %u %u\n",io.superbank,io.interrupt_enabled);}return 0;}
