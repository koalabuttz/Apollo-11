/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_tig5_check.h"
#include "cm_tig5_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_tig5_check(),phase;
 for(phase=3;phase<256u;++phase){cm_tig5_input(&state,&io,0);before=state;oldio=io;if(cm_tig5_phase(&state,&io,(uint8_t)phase)!=CM_TIG5_INVALID||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;}
 printf("1536 original CM TIG-5 full-image states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
