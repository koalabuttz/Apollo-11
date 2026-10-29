/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_idle_dispatch_check.h"
#include "cm_idle_dispatch_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_idle_dispatch_check();exec_transfer next;cm_idle_dispatch_input(&state,&io,0);state.words[0067]=13;before=state;oldio=io;
 if(cm_idle_dispatch(&state,&io,&next)!=EXEC_OUT_OF_SCOPE||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;
 printf("4096 original CM idle adoption complete basic/interpreted/idle states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
