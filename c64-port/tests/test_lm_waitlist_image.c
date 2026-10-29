/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_waitlist_image_cases.h"
static int reject(unsigned n){apollo_overlay_state m,before;p66_bias_io io,old;exec_address task=lm_waitlist_image_input(&m,&io,2);enum lm_waitlist_entry entry=LM_WAITLIST_ENTRY;uint16_t *w=m.words;
 switch(n){case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[0]=040000;break;case 3:w[2]=07776;break;case 4:entry=LM_TWIDDLE_ENTRY;w[5]=05173;w[2]=0;break;case 5:w[3]=04000;break;case 6:w[3]|=1;break;case 7:w[4]|=1;break;case 8:w[6]^=1;break;case 9:io.superbank=1;break;case 10:io.interrupt_enabled=2;break;case 11:task.location=0;break;case 12:task.location=010000;break;case 13:task.bank|=01000;break;case 14:w[026]=040000;break;case 15:w[0]=16251;break;case 16:entry=(enum lm_waitlist_entry)2;break;default:return 0;}
 before=m;old=io;return lm_waitlist_image(&m,&io,entry,task)==WL_OUT_OF_SCOPE&&!memcmp(&m,&before,sizeof m)&&!memcmp(&io,&old,sizeof io);
}
int main(void){unsigned n;apollo_overlay_state m;p66_bias_io io;exec_address task=lm_waitlist_image_input(&m,&io,2);for(n=0;n<17u;++n)if(!reject(n)){fprintf(stderr,"guard%u failed\n",n);return 1;}if(lm_waitlist_image(0,&io,LM_WAITLIST_ENTRY,task)!=WL_OUT_OF_SCOPE||lm_waitlist_image(&m,0,LM_WAITLIST_ENTRY,task)!=WL_OUT_OF_SCOPE)return 2;puts("PASS:17 invalid WAITLIST domains and2 null interfaces preserve complete canonical image and I/O");return 0;}
