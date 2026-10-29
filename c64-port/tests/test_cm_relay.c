/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_relay_check.h"
static apollo_overlay_state s,before;static uint8_t screen[1002],color[1002];
int main(void){uint8_t n;uint16_t failures=0;for(n=0;n<CM_RELAY_TARGET_COUNT;++n)failures=(uint16_t)(failures+cm_relay_check(n,&s,&before));screen[0]=screen[1001]=color[0]=color[1001]=0xa5;failures=(uint16_t)(failures+cm_relay_display_check(&s,screen+1,color+1));if(screen[0]!=0xa5||screen[1001]!=0xa5||color[0]!=0xa5||color[1001]!=0xa5)++failures;printf("CM relay channel10:64 complete2052-field states and real MODE89 screen, %u failures\n",failures);return failures!=0;}
