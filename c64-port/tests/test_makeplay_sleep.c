/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "makeplay_sleep_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t n,k;p66_bias_io io,old;
 for(j=0;j<64u;++j)failures+=makeplay_sleep_check((uint8_t)j,&s,&before);
 for(n=0;n<4u;++n)for(k=0;k<13u;++k){uint16_t number=(uint16_t)(n*4096u);
  makeplay_sleep_input(&s,&io,number);
  switch(k){case 0:io.superbank=0200;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=0;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[0155]=01475;break;case 8:s.words[0157]=0;break;case 9:s.words[0160]=0;break;case 10:s.words[01072]=0;break;case 11:s.words[0371]=0;break;default:io.superbank=1;break;}
  before=s;old=io;if(makeplay_sleep_apply(&s,&io,number)!=P66_SLEEP_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }
 printf("64 complete MAKEPLAY sleep images and52 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
