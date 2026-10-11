/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_restart_select_check.h"
#include "cm_restart_select_cases.h"
static apollo_overlay_state state,before;
int main(void){unsigned failures=cm_rsselect_check(),entry;
 for(entry=3u;entry<256u;++entry){cm_rsselect_input(&state,0);before=state;if(cm_restart_select(&state,(enum cm_restart_select_entry)entry)!=CM_RS_SELECT_INVALID||memcmp(&state,&before,sizeof state))++failures;}
 cm_rsselect_input(&state,0);state.words[6]^=1u;before=state;
 if(cm_restart_select(&state,CM_RS_SELECT_GROUP)!=CM_RS_SELECT_INVALID||memcmp(&state,&before,sizeof state))++failures;
 printf("6144 original CM restart selection full-image states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
