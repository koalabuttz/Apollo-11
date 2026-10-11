/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_sleep_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;p66_bias_io io,old;agc_word channel,oldchannel;exec_address next,oldnext;
 for(j=0;j<64u;++j)failures+=lm_sleep_check((uint8_t)j,&s,&before);
 for(k=0;k<17u;++k){lm_sleep_input(&s,&io,&channel,18);s.words[0203]=037777;next.location=01234;next.bank=056;
  switch(k){case 0:io.superbank=0200;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=1;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:channel=0100000;break;case 8:s.words[0]=040000;break;case 9:s.words[0167]=0;break;case 10:s.words[0167]=077777;break;case 11:s.words[0203]=0;break;case 12:io.superbank=017;break;case 13:s.words[0201]=02010;break;case 14:io.superbank=1;break;case 15:s.words[0]=0137777;break;default:s.words[3]=04000;break;}
  before=s;old=io;oldchannel=channel;oldnext=next;if(lm_basic_sleep(&s,&io,&channel,&next)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||channel!=oldchannel||memcmp(&next,&oldnext,sizeof next))++failures;
 }
 printf("64 complete JOBSLEEP images and17 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
