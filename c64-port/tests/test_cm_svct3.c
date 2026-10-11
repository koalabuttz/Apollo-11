/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_svct3_check.h"
#include "cm_svct3_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_svct3_check(),phase;
 for(phase=3;phase<256u;++phase){cm_svct3_input(&state,&io,0);before=state;oldio=io;if(cm_svct3_phase(&state,&io,(uint8_t)phase)!=CM_SVCT3_INVALID||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;}
 printf("1536 original CM SVCT3 full-image states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
