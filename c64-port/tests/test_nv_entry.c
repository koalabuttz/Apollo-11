/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "nv_entry_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;p66_bias_io io,old;
 for(j=0;j<64u;++j)failures+=nv_entry_check((uint8_t)j,&s,&before);
 for(k=0;k<13u;++k){nv_entry_input(&s,&io,0);
  switch(k){case 0:io.superbank=0100;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=0;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[0]=01475;break;case 8:s.words[1]=1;break;case 9:s.words[2]=03120;break;case 10:s.words[01012]=8;break;case 11:io.superbank=1;break;default:io.superbank=077777;break;}
  before=s;old=io;if(p66_nv_enter(&s,&io)!=P66_NV_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }
 printf("64 complete NVMONOPT images and13 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
