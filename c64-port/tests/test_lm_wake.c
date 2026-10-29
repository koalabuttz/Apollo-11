/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_wake_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;p66_bias_io io,old;
 for(j=0;j<64u;++j)failures+=lm_wake_check((uint8_t)j,&s,&before);
 for(k=0;k<20u;++k){lm_wake_input(&s,&io,1);
  switch(k){case 0:io.superbank=0200;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=1;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[067]=1;break;case 8:s.words[0]=040000;break;case 9:s.words[2]=077;break;case 10:s.words[2]=010000;break;case 11:s.words[0167]=0;break;case 12:s.words[0167u+36u]=0;break;case 13:s.words[0203]=0;break;case 14:s.words[067]=96;break;case 15:s.words[0201]=020000;break;case 16:s.words[0201]=010;break;case 17:io.superbank=1;break;case 18:s.words[0]=0137777;break;default:s.words[067]=040000;break;}
  before=s;old=io;if(lm_basic_wake(&s,&io)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }
 printf("64 complete JOBWAKE images and20 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
