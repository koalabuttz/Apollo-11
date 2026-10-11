/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "display_control_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;p66_bias_io io,old;
 for(j=0;j<64u;++j)failures+=display_control_check((uint8_t)j,&s,&before);
 for(k=0;k<14u;++k){display_control_input(&s,&io,0);
  switch(k){case 0:io.superbank=0100;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=0;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[067]=1;break;case 8:s.words[0164]=0;break;case 9:s.words[0165]=0;break;case 10:s.words[0163]=077777;break;case 11:s.words[023]=1;break;case 12:s.words[020]=1;break;default:s.words[03621]=1;break;}
  before=s;old=io;if(p66_display_exit(&s,&io)!=P66_DISPLAY_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }
 printf("64 complete display control images and14 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
