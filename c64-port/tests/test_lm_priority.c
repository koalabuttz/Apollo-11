/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_priority_check.h"
static apollo_overlay_state s,before;
int main(void){unsigned f=0,j;uint8_t k;p66_bias_io io,old;exec_address next,oldnext={02345,020007};for(j=0;j<64u;++j)f+=lm_priority_check((uint8_t)j,&s,&before);for(k=0;k<16u;++k){lm_priority_input(&s,&io,0);switch(k){case 0:io.superbank=1;break;case 1:io.interrupt_enabled=2;break;case 2:s.words[3]=1;break;case 3:s.words[4]=1;break;case 4:s.words[6]=1;break;case 5:s.words[7]=1;break;case 6:s.words[5]=0;break;case 7:s.words[0]=0;break;case 8:s.words[0]=040000;break;case 9:s.words[2]=077;break;case 10:s.words[2]=010000;break;case 11:s.words[0167]=077777;break;case 12:s.words[0167+12]=0;break;case 13:s.words[0167+12]=037777;s.words[0164+12]=077777;break;case 14:s.words[0167+12]=037777;s.words[0165+12]=017777;break;default:io.superbank=077777;break;}
before=s;old=io;next=oldnext;if(lm_basic_priority_change(&s,&io,&next)!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||memcmp(&next,&oldnext,sizeof next))++f;}
printf("64 complete original PRIOCHNG images;16 invalid inputs: %s\n",f?"FAIL":"PASS");return f?1:0;}
