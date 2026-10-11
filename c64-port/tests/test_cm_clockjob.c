/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_clockjob_check.h"
static apollo_overlay_state s,before;
int main(void){uint16_t n,k,failures=0;cm_native_io io,old;for(n=0;n<64u;++n)failures=(uint16_t)(failures+cm_clockjob_check((uint8_t)n,&s,&before));for(n=0;n<3u;++n)for(k=0;k<(n==1u?13u:9u);++k){cm_clockjob_input(&s,&io,(uint16_t)(n*8192u));switch(k){case 0:s.words[5]=0;break;case 1:s.words[3]=0;break;case 2:s.words[4]=0;break;case 3:s.words[6]=0;break;case 4:s.words[7]=1;break;case 5:io.superbank=0200;break;case 6:io.interrupt_enabled=2;break;case 7:io.superbank=1;break;case 8:if(n==0u)s.words[032]=0100000;else if(n==1u)s.words[0]=0;else s.words[01145]=0100000;break;case 9:s.words[2]=0;break;case 10:s.words[0133]=0;break;case 11:s.words[0134]=0;break;default:s.words[1]=040000;break;}before=s;old=io;if(cm_clockjob_apply(&s,&io,(uint16_t)(n*8192u))!=CM_CLOCKJOB_UNSUPPORTED||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;}printf("64 complete CM CLOCKJOB states and31 unchanged invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
