/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_danzig_check.h"
#include "cm_danzig_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_danzig_check();exec_transfer next;cm_danzig_input(&state,&io,0);state.words[023]=1;before=state;oldio=io;
 if(cm_danzig(&state,&io,&next)!=EXEC_OUT_OF_SCOPE||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;
 printf("4096 original CM DANZIG complete basic/interpreted/idle states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
