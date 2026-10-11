/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_wake_image_cases.h"
#include "lm_wake_cases.h"
static int reject(unsigned n){apollo_overlay_state s,before;p66_bias_io io,oldio;uint16_t *w=s.words;lm_wake_image_input(&s,&io,8192);
 switch(n){case 0:w[5]++;break;case 1:w[0]=040000;break;case 2:w[2]=077;break;
 case 3:w[2]=010000;break;case 4:w[3]=04000;break;case 5:w[3]|=1;break;
 case 6:w[4]|=1;break;case 7:w[6]^=1;break;case 8:w[7]=1;break;
 case 9:io.superbank=1;break;case 10:io.interrupt_enabled=2;break;case 11:w[067]=1;break;
 case 12:w[067]=96;break;case 13:w[0167]=0;break;case 14:w[0165]|=010;break;
 case 15:w[067]=0;break;default:return 0;}
 before=s;oldio=io;return lm_wake_image(&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io);
}
int main(void){uint16_t n;apollo_overlay_state s,before;p66_bias_io io,oldio;enum exec_status a,b;
 for(n=0;n<16u;++n)if(!reject(n)){fprintf(stderr,"guard %u failed\n",n);return 1;}
 for(n=0;n<LM_WAKE_CASES;++n){lm_wake_input(&s,&io,n);before=s;oldio=io;a=lm_basic_wake(&before,&oldio);b=lm_wake_image(&s,&io);
 if(a!=b||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io)){fprintf(stderr,"compatibility %u failed\n",n);return 2;}}
 if(lm_wake_image(0,&io)!=EXEC_OUT_OF_SCOPE||lm_wake_image(&s,0)!=EXEC_OUT_OF_SCOPE)return 3;
 puts("PASS:8192 immutable old JOBWAKE results;16 all-object mutation guards and2 null interfaces");return 0;}
