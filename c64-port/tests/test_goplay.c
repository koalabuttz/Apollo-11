/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "goplay_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned f=0,j;uint8_t phase,k;p66_bias_io io,old;agc_word channel,oldchannel;for(j=0;j<64u;++j)f+=goplay_check((uint8_t)j,&s,&before);for(phase=0;phase<2u;++phase)for(k=0;k<12u;++k){goplay_input(&s,&io,&channel,0,phase);switch(k){case 0:io.superbank=0100;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[0164]=0;break;case 8:s.words[01072]=0;break;case 9:s.words[0371]=0;break;case 10:io.superbank=1;break;default:if(phase)channel=0100000;else io.superbank=2;break;}
before=s;old=io;oldchannel=channel;if((phase?p66_goplay_finish(&s,&io,&channel):p66_goplay_enter(&s,&io))!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||channel!=oldchannel)++f;}printf("64 complete GOPLAY boundary images;24 invalid inputs: %s\n",f?"FAIL":"PASS");return f?1:0;}
