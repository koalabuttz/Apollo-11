/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_wake_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t n,failures=0;cm_native_io io,old;for(n=0;n<64u;++n)failures=(uint16_t)(failures+cm_wake_check((uint8_t)n,&s,&before));for(n=0;n<19u;++n){cm_wake_input(&s,&io,1);switch(n){case 0:s.words[5]=0;break;case 1:s.words[3]=1;break;case 2:s.words[4]=1;break;case 3:s.words[6]=0;break;case 4:s.words[7]=1;break;case 5:io.superbank=0200;break;case 6:io.interrupt_enabled=2;break;case 7:io.superbank=1;break;case 8:s.words[0]=040000;break;case 9:s.words[2]=0;break;case 10:s.words[2]=010000;break;case 11:s.words[0167]=0;break;case 12:s.words[0167u+36u]=0;break;case 13:s.words[0203]=0;break;case 14:s.words[067]=1;break;case 15:s.words[067]=84;break;case 16:s.words[0201]=020000;break;case 17:io.executive_alarm=1;break;default:s.words[3]=04000;break;}before=s;old=io;if(cm_basic_wake(&s,&io)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("64 complete CM JOBWAKE states and19 unchanged invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
