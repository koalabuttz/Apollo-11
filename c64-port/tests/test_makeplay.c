/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "makeplay_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t n,k;p66_bias_io io,old;agc_word channel,oldchannel;
 for(j=0;j<64u;++j)failures+=makeplay_check((uint8_t)j,&s,&before);
 for(n=0;n<3u;++n)for(k=0;k<15u;++k){uint16_t number=(uint16_t)(n*4096u);
  makeplay_input(&s,&io,&channel,number);
  switch(k){case 0:io.superbank=0200;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=0;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[0155]=01475;break;case 8:s.words[0157]=0;break;case 9:s.words[0160]=0;break;case 10:if(n==0)s.words[0167]=0;else if(n==1)s.words[0163]=0;else s.words[0164]=0;break;case 11:if(n==0)s.words[0167]=077777;else if(n==1)s.words[01012]=8;else s.words[064]=1;break;case 12:if(n==0)s.words[0167]=0110;else if(n==1)channel=0100000;else s.words[01072]=0;break;case 13:if(n==0)s.words[0167]=040000;else if(n==1)s.words[0163]=033001;else s.words[0371]=0;break;default:io.superbank=1;break;}
  before=s;old=io;oldchannel=channel;if(makeplay_apply(&s,&io,&channel,number)!=P66_MAKEPLAY_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||channel!=oldchannel)++failures;
 }
 printf("64 complete MAKEPLAY images and45 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
