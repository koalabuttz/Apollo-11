/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_spectest_handoff_cases.h"
#include "lm_admission_raw.h"
#include "lm_wake_image.h"
static int reject(unsigned n){apollo_overlay_state s,before;p66_bias_io io,oldio;exec_address job=lm_spectest_handoff_input(&s,&io,0);enum lm_spectest_entry entry=LM_SPECTEST_NOVAC;agc_word *w=s.words;
 switch(n){case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[3]=04000;break;
 case 3:w[3]|=1;break;case 4:w[4]|=1;break;case 5:w[6]^=1;break;
 case 6:io.superbank=1;break;case 7:io.interrupt_enabled=2;break;case 8:w[067]=014;break;
 case 9:w[2]=077;break;case 10:w[2]=07776;break;case 11:w[0167]=0;break;
 case 12:w[0]=040000;break;case 13:w[0]=037001;break;
 case 14:w[2]=02000;job.bank=0100000;break;case 15:entry=(enum lm_spectest_entry)6;break;
 case 16:job=lm_spectest_handoff_input(&s,&io,4);entry=LM_SPECTEST_WAKE;w[0164]++;break;
 case 17:job=lm_spectest_handoff_input(&s,&io,4);entry=LM_SPECTEST_WAKE;w[0165]|=010;break;
 case 18:job=lm_spectest_handoff_input(&s,&io,5);entry=LM_SPECTEST_DIRECT;io.interrupt_enabled=1;break;
 case 19:job=lm_spectest_handoff_input(&s,&io,5);entry=LM_SPECTEST_DIRECT;w[067]=AGC_NEG_ZERO;break;
 case 20:job=lm_spectest_handoff_input(&s,&io,1);entry=LM_SPECTEST_FINDVAC;w[0400]=1;break;
 default:return 0;}
 before=s;oldio=io;return lm_spectest_handoff(&s,&io,entry,job)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io);
}
int main(void){uint16_t n;unsigned rejected=0;apollo_overlay_state s,before;p66_bias_io io,oldio;exec_address job;enum exec_status status;
 for(n=0;n<21u;++n)if(!reject(n)){fprintf(stderr,"guard%u failed\n",n);return 1;}
 for(n=0;n<LM_SPECTEST_HANDOFF_CASES;++n){uint8_t entry=(uint8_t)(n%6u);job=lm_spectest_handoff_input(&s,&io,n);before=s;oldio=io;
 if(entry<4u)status=lm_admission_raw(&s,&io,(enum lm_admission_entry)entry,job);
 else if(entry==4u)status=lm_wake_image(&s,&io);else continue;
 if(status!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io)){fprintf(stderr,"oldguard%u failed\n",n);return 2;}++rejected;}
 if(lm_spectest_handoff(0,&io,LM_SPECTEST_NOVAC,job)!=EXEC_OUT_OF_SCOPE||lm_spectest_handoff(&s,0,LM_SPECTEST_NOVAC,job)!=EXEC_OUT_OF_SCOPE)return 3;
 printf("PASS:%u prior frozen admission/wake exclusions unchanged;21 all-object mutation guards and2 null interfaces\n",rejected);return 0;}
