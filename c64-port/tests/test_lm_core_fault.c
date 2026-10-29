/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_core_fault_cases.h"
#include "lm_admission_raw.h"
#include "lm_wake_image.h"
static int reject(unsigned n){apollo_overlay_state s,before;p66_bias_io io,oldio;exec_address job;enum lm_core_fault_entry entry=LM_CORE_FAULT_NOVAC;agc_word *w=s.words;uint8_t i;
 job=lm_core_fault_input(&s,&io,0);
 switch(n){case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[2]=077;break;
 case 3:w[2]=07776;break;case 4:w[3]=04000;break;case 5:w[3]|=1;break;
 case 6:w[4]|=1;break;case 7:w[6]^=1;break;case 8:io.superbank=1;break;
 case 9:io.interrupt_enabled=2;break;case 10:w[0]=040000;break;
 case 11:w[0]=037001;break;case 12:entry=(enum lm_core_fault_entry)5;break;
 case 13:job=lm_core_fault_input(&s,&io,2);entry=LM_CORE_FAULT_SPVACIN;io.interrupt_enabled=1;break;
 case 14:job=lm_core_fault_input(&s,&io,1);entry=LM_CORE_FAULT_FINDVAC;w[0400]=1;break;
 case 15:job=lm_core_fault_input(&s,&io,1);entry=LM_CORE_FAULT_FINDVAC;for(i=0;i<5u;++i)w[0400u+44u*i]=0;break;
 case 16:job=lm_core_fault_input(&s,&io,5);w[0167]=AGC_NEG_ZERO;break;
 case 17:w[0167]=020110;break;
 case 18:job=lm_core_fault_input(&s,&io,4);entry=LM_CORE_FAULT_WAKE;w[0]=040000;break;
 case 19:job=lm_core_fault_input(&s,&io,9);entry=LM_CORE_FAULT_WAKE;w[0167]=agc_negate(020110);w[0164]=agc_correct(w[0]);break;
 case 20:job=lm_core_fault_input(&s,&io,2);entry=LM_CORE_FAULT_SPVACIN;w[063]=037001;break;
 case 21:w[2]=03200;job.location=0100000;break;
 case 22:w[2]=03200;job.bank=0100000;break;
 default:return 0;}
 before=s;oldio=io;return lm_core_fault(&s,&io,entry,job)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io);
}
int main(void){uint16_t n;apollo_overlay_state s,before;p66_bias_io io,oldio;exec_address job;enum exec_status status;
 for(n=0;n<23u;++n)if(!reject(n)){fprintf(stderr,"guard%u failed\n",n);return 1;}
 for(n=0;n<LM_CORE_FAULT_CASES;++n){job=lm_core_fault_input(&s,&io,n);before=s;oldio=io;
 status=n%5u==4u?lm_wake_image(&s,&io):lm_admission_raw(&s,&io,(enum lm_admission_entry)(n%5u),job);
 if(status!=EXEC_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io)){fprintf(stderr,"oldguard%u failed\n",n);return 2;}}
 if(lm_core_fault(0,&io,LM_CORE_FAULT_NOVAC,job)!=EXEC_OUT_OF_SCOPE||lm_core_fault(&s,0,LM_CORE_FAULT_NOVAC,job)!=EXEC_OUT_OF_SCOPE)return 3;
 puts("PASS:16384 frozen admission/wake exclusions unchanged;23 all-object mutation guards and2 null interfaces");return 0;
}
