/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_basic_finish_check.h"
#include "cm_basic_finish_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_basic_finish_check();exec_transfer next;cm_basic_finish_input(&state,&io,0);state.words[0203]=0;before=state;oldio=io;
 if(cm_basic_finish(&state,&io,&next)!=EXEC_OUT_OF_SCOPE||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;
 printf("16384 original CM ENDJOB complete basic/interpreted/idle states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
