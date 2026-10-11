/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_display_rest_check.h"
static apollo_overlay_state s,before;
int main(void){cm_native_io io,old;uint16_t failures=0;uint8_t n,g;for(n=0;n<60u;++n)failures=(uint16_t)(failures+cm_display_rest_checkpoint(n,&s,&before));for(n=0;n<3u;++n)for(g=0;g<10u;++g){if(!cm_display_rest_checkpoint_input((uint8_t)(n*20u),&s,&io))return 2;switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=1;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=1;break;case 6:io.interrupt_enabled=2;break;case 7:s.words[0164]=3;break;case 8:s.words[0162]=0100000;break;default:s.words[01042]=0100000;break;}before=s;old=io;if(cm_display_rest(&s,&io)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("60 full CM display REST source states and30 unchanged invalid guards: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
