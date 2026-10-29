/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_danzig_change_cases.h"
static int reject(unsigned n){apollo_overlay_state m,before;p66_bias_io io,old;exec_transfer next={{0777,012345},1},oldnext=next;uint16_t*w;lm_danzig_change_input(&m,&io,0);w=m.words;
 switch(n){case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[3]=04000;break;case 3:w[3]|=1;break;case 4:w[4]|=1;break;case 5:w[6]^=1;break;case 6:io.superbank=1;break;case 7:io.interrupt_enabled=2;break;case 8:w[067]=0;break;case 9:w[067]=1;break;case 10:w[067]=96;break;case 11:w[0167]=0;break;case 12:w[0167]=040000;break;case 13:w[0167+12]=0;break;case 14:w[0167+12]=040000;break;case 15:w[0164]=0;break;case 16:w[0164]=010000;break;case 17:w[0164+12]=0;break;case 18:w[0164+12]=010000;break;case 19:w[0164+12]=agc_negate(07777);break;case 20:w[0165+12]|=01000;break;case 21:w[0165]|=01000;break;case 22:w[023]=1;break;case 23:w[023]=077777;break;case 24:w[067]=077777;break;default:return 0;}
 before=m;old=io;return lm_danzig_change(&m,&io,&next)==EXEC_OUT_OF_SCOPE&&!memcmp(&m,&before,sizeof m)&&!memcmp(&io,&old,sizeof io)&&!memcmp(&next,&oldnext,sizeof next);
}
int main(void){unsigned n;apollo_overlay_state m;p66_bias_io io;exec_transfer next;lm_danzig_change_input(&m,&io,0);for(n=0;n<25u;++n)if(!reject(n)){fprintf(stderr,"guard%u failed\n",n);return 1;}if(lm_danzig_change(0,&io,&next)!=EXEC_OUT_OF_SCOPE||lm_danzig_change(&m,0,&next)!=EXEC_OUT_OF_SCOPE||lm_danzig_change(&m,&io,0)!=EXEC_OUT_OF_SCOPE)return 2;puts("PASS:25 invalid DANZIG/NOIBNKSW domains and3 null interfaces preserve complete canonical image/I/O/transfer");return 0;}
