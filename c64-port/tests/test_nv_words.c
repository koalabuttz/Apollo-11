/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "nv_words_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned failures=0,j;uint16_t k;uint8_t phase;p66_bias_io io,old;
 for(j=0;j<64u;++j)failures+=nv_words_check((uint8_t)j,&s,&before);
 for(phase=0;phase<2u;++phase)for(k=0;k<10u;++k){nv_words_input(&s,&io,0);if(phase)(void)p66_nv_verb(&s,&io);
  switch(k){case 0:io.superbank=0;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=0;break;case 3:s.words[4]=0;break;case 4:s.words[6]=0;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[phase?0157:0]=1;break;case 8:s.words[phase?01033:01034]=0;break;default:io.superbank=077777;break;}
  before=s;old=io;if((phase?p66_nv_noun(&s,&io):p66_nv_verb(&s,&io))!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io))++failures;
 }
 printf("64 complete verb/noun images and20 invalid boundaries: %s\n",failures?"FAIL":"PASS");return failures?1:0;
}
