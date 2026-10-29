/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_busy_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;p66_bias_io io,old;agc_word channel,oldchannel;
 for(j=0;j<64u;++j)failures+=lm_busy_check((uint8_t)j,&s,&before);
 for(k=0;k<13u;++k){lm_busy_input(&s,&io,&channel,0);
  switch(k){case 0:io.superbank=0200;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=1;break;case 4:s.words[6]=1;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:channel=0100000;break;case 8:s.words[0]=040000;break;case 9:s.words[01043]=AGC_NEG_ZERO;break;case 10:io.superbank=1;break;case 11:s.words[0]=0137777;break;default:s.words[3]=04000;break;}
  before=s;old=io;oldchannel=channel;if(lm_pinball_busy(&s,&io,&channel)!=LM_BUSY_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||channel!=oldchannel)++failures;
 }
 printf("64 complete NVSUBUSY images and13 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
