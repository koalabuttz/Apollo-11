/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_normal_display_check.h"
static apollo_overlay_state s,before;
int main(void){static const uint8_t cases[12]={0,1,2,3,4,7,48,49,50,51,52,55};cm_native_io io,old;uint16_t failures=0,n;uint8_t guard,phase;for(n=0;n<64u;++n)failures=(uint16_t)(failures+cm_normal_display_checkpoint_check((uint8_t)n,&s,&before));for(n=0;n<12u;++n)for(guard=0;guard<9u;++guard){phase=cm_normal_display_checkpoint_input(cases[n],&s,&io);switch(guard){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.interrupt_enabled=2;break;case 6:io.superbank=1;break;case 7:phase=6;break;default:s.words[0371]=0;break;}before=s;old=io;if(cm_normal_display_caller(&s,&io,phase)||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("64 complete CM normal V06N40/N85 caller states and 108 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
