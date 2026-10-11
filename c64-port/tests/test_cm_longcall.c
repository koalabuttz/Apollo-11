/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_longcall_check.h"
#include "cm_longcall_cases.h"
static apollo_overlay_state state,before;
int main(void){unsigned failures=cm_longcall_check(),phase;exec_address task;
 for(phase=4u;phase<256u;++phase){cm_longcall_input(&state,&task,0);before=state;if(cm_longcall_phase_run(&state,(enum cm_longcall_phase)phase,task)!=CM_LONGCALL_INVALID||memcmp(&state,&before,sizeof state))++failures;}
 cm_longcall_input(&state,&task,0);before=state;
 if(cm_longcall_phase_run(&state,(enum cm_longcall_phase)-1,task)!=CM_LONGCALL_INVALID||memcmp(&state,&before,sizeof state))++failures;
 task.location=0;
 if(cm_longcall_phase_run(&state,CM_LONGCALL_ENTER,task)!=CM_LONGCALL_INVALID||memcmp(&state,&before,sizeof state))++failures;
 cm_longcall_input(&state,&task,2048);state.words[01434]=0;before=state;
 if(cm_longcall_phase_run(&state,CM_LONGCALL_RETURN,task)!=CM_LONGCALL_INVALID||memcmp(&state,&before,sizeof state))++failures;
 printf("4096 original CM long-call full-image states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
