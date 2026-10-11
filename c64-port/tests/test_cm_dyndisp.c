/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_dyndisp_check.h"
static apollo_overlay_state s,before;
int main(void){cm_native_io io,old;uint16_t failures=0,n;uint8_t guard,phase;for(n=0;n<68u;++n)failures=(uint16_t)(failures+cm_dyndisp_checkpoint_check((uint8_t)n,&s,&before));for(n=0;n<17u;++n)for(guard=0;guard<12u;++guard){phase=cm_dyndisp_checkpoint_input((uint8_t)n,&s,&io);switch(guard){case 0:s.words[5]=0;break;case 1:s.words[0163]=0;break;case 2:s.words[4]=1;break;case 3:s.words[3]=1;break;case 4:s.words[6]^=1;break;case 5:s.words[7]=1;break;case 6:s.words[067]=1;break;case 7:s.words[0120]=0400;break;case 8:io.interrupt_enabled=2;break;case 9:io.superbank=1;break;case 10:io.superbank=0100;break;default:phase=17;break;}before=s;old=io;if(cm_dyndisp_stage(&s,&io,phase)||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("68 complete CM DYNDISP caller states and204 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
