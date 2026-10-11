/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_delay_image_check.h"
#include "cm_delay_image_cases.h"
static apollo_overlay_state state,before;static cm_native_io io,oldio;
int main(void){unsigned failures=cm_delay_image_check();cm_delay_image_input(&state,&io,0);before=state;oldio=io;
 if(cm_delay_image(&state,&io,500,2)!=CM_WL_OUT_OF_SCOPE||memcmp(&state,&before,sizeof state)||memcmp(&io,&oldio,sizeof io))++failures;
 printf("1024 original CM FIXDELAY/VARDELAY full-image states and guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
