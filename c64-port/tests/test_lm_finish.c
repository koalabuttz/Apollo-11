/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_finish_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;p66_bias_io io,old;agc_word channel,oldchannel;exec_transfer next,oldnext;for(j=0;j<64u;++j)failures+=lm_finish_check((uint8_t)j,&s,&before);for(k=0;k<21u;++k){uint8_t i;lm_finish_input(&s,&io,&channel,18);for(i=1;i<8u;++i)s.words[0167u+12u*i]=AGC_NEG_ZERO;s.words[0203]=037000;s.words[0200]=02001;s.words[0201]=020067;memset(&next,0x5a,sizeof next);
 switch(k){case 0:io.superbank=0200;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=1;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:channel=0100000;break;case 8:s.words[0167]=0;break;case 9:s.words[0167]=077777;break;case 10:s.words[0203]=0;break;case 11:io.superbank=017;break;case 12:s.words[0201]=02010;break;case 13:io.superbank=1;break;case 14:s.words[0167]=021177;break;case 15:s.words[0200]=0;break;case 16:s.words[0200]=010000;break;case 17:s.words[0200]=agc_negate(07777);break;case 18:s.words[3]=04000;break;case 19:s.words[4]=0100000;break;default:s.words[0167]=021777;break;}
 before=s;old=io;oldchannel=channel;oldnext=next;if(lm_basic_finish(&s,&io,&channel,&next)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||channel!=oldchannel||memcmp(&next,&oldnext,sizeof next))++failures;}
 printf("64 complete ENDOFJOB states and21 unchanged invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
