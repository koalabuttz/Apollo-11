/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_clock_normal_display_check.h"
static apollo_overlay_state s,before;
int main(void){cm_native_io io,old;cm_mission_loop r,b;uint16_t failures=0;uint8_t n,g,p;for(n=0;n<9u;++n){failures=(uint16_t)(failures+cm_clock_normal_checkpoint_check(n,&s,&before));for(g=0;g<4u;++g){if(!cm_clock_normal_checkpoint_input(n,&s,&io,&r))return 2;p=29;switch(g){case 0:p=6;break;case 1:p=30;break;case 2:r.in_callback=1;break;default:s.words[0371]=0;break;}before=s;old=io;b=r;if(cm_clock_normal_display_step(&r,p)==CM_SERVICE_ADVANCED||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||memcmp(&r,&b,sizeof r))++failures;}}printf("9 genuine full mission-END source states and36 unchanged wrapper guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
