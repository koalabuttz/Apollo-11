/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_display_control_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t n,k,failures=0;cm_native_io io,old;for(n=0;n<64u;++n)failures=(uint16_t)(failures+cm_display_control_check((uint8_t)n,&s,&before));for(n=0;n<3u;++n)for(k=0;k<14u;++k){cm_display_control_input(&s,&io,(uint16_t)(n*8192u));switch(k){case 0:s.words[5]=0;break;case 1:s.words[3]=1;break;case 2:s.words[4]=0;break;case 3:s.words[6]=0;break;case 4:s.words[7]=1;break;case 5:io.superbank=0200;break;case 6:io.interrupt_enabled=2;break;case 7:io.superbank=1;break;case 8:s.words[0]=040000;break;case 9:s.words[0133]=010000;break;case 10:s.words[0134]=0100000;break;case 11:s.words[0134]=1;break;case 12:s.words[3]=04000;break;default:s.words[0]=0100000;break;}before=s;old=io;if(cm_display_control_enter(&s,&io)!=CM_DISPLAY_CONTROL_UNSUPPORTED||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("64 complete CM display entry states and42 unchanged invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
