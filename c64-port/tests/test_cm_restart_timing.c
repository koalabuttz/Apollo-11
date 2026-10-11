/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_restart_timing_check.h"
#include "cm_restart_timing_cases.h"
static apollo_overlay_state state,before;
int main(void){unsigned failures=cm_rstime_check(),group;
 for(group=6u;group<256u;++group){cm_rstime_input(&state,0);before=state;if(cm_restart_wait_delay(&state,(uint8_t)group)!=CM_RSTIME_INVALID||memcmp(&state,&before,sizeof state))++failures;}
 cm_rstime_input(&state,0);state.words[6]^=1u;before=state;
 if(cm_restart_wait_delay(&state,0)!=CM_RSTIME_INVALID||memcmp(&state,&before,sizeof state))++failures;
 if(cm_restart_long_delay(&state)!=CM_RSTIME_INVALID||memcmp(&state,&before,sizeof state))++failures;
 cm_rstime_input(&state,0);state.words[1]=040000;before=state;
 if(cm_restart_long_delay(&state)!=CM_RSTIME_INVALID||memcmp(&state,&before,sizeof state))++failures;
 printf("4096 original CM full-image timing states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
