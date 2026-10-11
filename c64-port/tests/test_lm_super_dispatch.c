/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_super_dispatch_cases.h"
static int reject(unsigned n){apollo_overlay_state s,before;p66_bias_io io,oldio;exec_transfer next,oldnext;uint16_t *w=s.words;
 lm_super_dispatch_input(&s,&io,0);memset(&next,0xa7,sizeof next);
 switch(n){case 0:w[5]++;w[5]++;break;case 1:w[7]=1;break;case 2:w[3]=04000;break;
 case 3:w[3]|=1;break;case 4:w[4]|=1;break;case 5:w[6]^=1;break;
 case 6:io.superbank=1;break;case 7:io.interrupt_enabled=2;break;
 case 8:w[0]=040000;break;case 9:w[1]=040000;break;case 10:w[0]=077;break;
 case 11:w[0]=agc_extend(010000);break;case 12:w[1]=1u<<3;break;
 default:return 0;}
 before=s;oldio=io;oldnext=next;return lm_super_dispatch(&s,&io,&next)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io)&&!memcmp(&next,&oldnext,sizeof next);
}
int main(void){uint16_t n;apollo_overlay_state s;p66_bias_io io;exec_transfer next;
 for(n=0;n<13u;++n)if(!reject(n)){fprintf(stderr,"guard %u failed\n",n);return 1;}
 for(n=0;n<LM_SUPER_DISPATCH_CASES;++n){agc_word loc,bank;lm_super_dispatch_input(&s,&io,n);loc=agc_correct(s.words[n&1u?1:0]);bank=agc_correct(s.words[n&1u?0:1]);
 if(lm_super_dispatch(&s,&io,&next)!=EXEC_OK||next.interpretive||next.address.location!=loc||next.address.bank!=bank){fprintf(stderr,"metadata%u failed\n",n);return 2;}}
 if(lm_super_dispatch(0,&io,&next)!=EXEC_OUT_OF_SCOPE||lm_super_dispatch(&s,0,&next)!=EXEC_OUT_OF_SCOPE||lm_super_dispatch(&s,&io,0)!=EXEC_OUT_OF_SCOPE)return 3;
 puts("PASS:16384 actual transfer payloads;13 all-object mutation guards and3 null interfaces");return 0;}
