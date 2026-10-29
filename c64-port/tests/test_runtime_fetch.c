/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "runtime_fetch_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t n,k;p66_bias_io io,old;
 for(j=0;j<64u;++j)failures+=runtime_fetch_check((uint8_t)j,&s,&before);
 for(n=0;n<2u;++n){for(k=0;k<13u;++k){runtime_fetch_input(&s,&io,(uint16_t)(n*4096u));old=io;
  switch(k){case 0:io.superbank=0100;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=0;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[067]=0100000;break;case 8:s.words[0164]=0;break;case 9:s.words[0165]=0;break;case 10:s.words[0163]=077777;break;case 11:s.words[023]=1;break;default:if(n)s.words[020]=0100000;else s.words[020]=0;break;}
  before=s;old=io;if(p66_runtime_throttle_fetch(&s,&io)!=P66_BRIDGE_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }}
 printf("64 complete runtime fetch images and26 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
