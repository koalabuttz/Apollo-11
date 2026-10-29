/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_scan_exception_cases.h"
#include "lm_context_lifecycle.h"
#include "lm_basic_finish.h"
static int reject(unsigned n){apollo_overlay_state s,before;p66_bias_io io,oldio;agc_word *w=s.words;enum lm_scan_entry entry=LM_SCAN_PRIORITY;uint8_t i;
 lm_scan_exception_input(&s,&io,0);
 switch(n){case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[3]=04000;break;
 case 3:w[3]|=1;break;case 4:w[4]|=1;break;case 5:w[6]^=1;break;
 case 6:io.superbank=1;break;case 7:io.interrupt_enabled=2;break;case 8:w[0167]=0;break;
 case 9:w[0167]=AGC_SIGN;break;case 10:w[2]=077;break;case 11:w[2]=010000;break;
 case 12:w[0]=040000;break;case 13:w[0]=agc_extend(037777);w[0167]|=1;break;
 case 14:for(i=1;i<8u;++i)w[0167u+12u*i]=AGC_NEG_ZERO;break;
 case 15:entry=(enum lm_scan_entry)4;break;
 case 16:lm_scan_exception_input(&s,&io,3);entry=LM_SCAN_DIRECT;io.interrupt_enabled=1;break;
 case 17:lm_scan_exception_input(&s,&io,31);entry=LM_SCAN_DIRECT;w[0131]=AGC_NEG_ZERO;break;
 case 18:lm_scan_exception_input(&s,&io,63);entry=LM_SCAN_DIRECT;w[0203]=01000;break;
 case 19:lm_scan_exception_input(&s,&io,2);entry=LM_SCAN_END;w[0167]=020123;break;
 case 20:lm_scan_exception_input(&s,&io,1);entry=LM_SCAN_SLEEP;w[0]=040000;break;
 default:return 0;}
 before=s;oldio=io;return lm_scan_exception(&s,&io,entry)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io);
}
int main(void){uint16_t n;unsigned rejected=0;apollo_overlay_state s,before;p66_bias_io io,oldio;exec_transfer next;agc_word channel;enum exec_status status;
 for(n=0;n<21u;++n)if(!reject(n)){fprintf(stderr,"guard%u failed\n",n);return 1;}
 for(n=0;n<LM_SCAN_EXCEPTION_CASES;++n){uint8_t entry=(uint8_t)(n%4u);if(entry==3u)continue;
 lm_scan_exception_input(&s,&io,n);before=s;oldio=io;channel=(agc_word)((n*177u)&AGC_MASK);
 if(entry==2u)status=lm_basic_finish(&s,&io,&channel,&next);
 else status=lm_context_lifecycle(&s,&io,&channel,(enum lm_context_lifecycle_entry)entry,&next);
 if(status!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io)||channel!=(agc_word)((n*177u)&AGC_MASK)){fprintf(stderr,"oldguard%u failed\n",n);return 2;}++rejected;}
 if(lm_scan_exception(0,&io,LM_SCAN_PRIORITY)!=EXEC_OUT_OF_SCOPE||lm_scan_exception(&s,0,LM_SCAN_PRIORITY)!=EXEC_OUT_OF_SCOPE)return 3;
 printf("PASS:%u frozen lifecycle/END exclusions unchanged;21 all-object mutation guards and2 null interfaces\n",rejected);return 0;}
