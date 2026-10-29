/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_waitlist_extended_check.h"
#include "cm_waitlist_extended_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_waitlist_extended_check();cm_waitlist_extended_input(&state,&io,0);before=state;oldio=io;
 if(cm_waitlist_extended(&state,&io,(exec_address){03207,032066},2)!=CM_WL_OUT_OF_SCOPE||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;
 printf("2048 original CM WAITLIST/TWIDDLE/alarm-entry full-image states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
