/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_entry_tasks_check.h"
#include "cm_entry_tasks_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_entrytask_check();cm_entrytask_input(&state,&io,0);state.words[6]^=1u;before=state;oldio=io;
 if(cm_setjtag_image(&state,&io)!=CM_WL_OUT_OF_SCOPE||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;
 printf("1024 original SETJTAG full-image states, actual C callbacks and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
