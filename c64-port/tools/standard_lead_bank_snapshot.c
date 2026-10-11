/* SPDX-License-Identifier: MIT */
#include "standard_lead_bank_cases.h"
#include "standard_lead_dump.h"
int main(void){uint16_t n;unsigned phase;apollo_overlay_state s;p66_bias_io io={0160,0};
 for(n=0;n<STANDARD_LEAD_BANK_CASES;++n){standard_lead_bank_input(&s,STANDARD_LEAD_MISSION,n);
  if(!standard_lead_stage(&s,STANDARD_LEAD_MISSION,STANDARD_LEAD_BANK_PREPARE,0)||!standard_lead_bank_call(&s,&io,STANDARD_LEAD_MISSION))return 1;
  for(phase=0;phase<3u;++phase){
   if(phase==1u){agc_word target=s.words[2];s.words[2]=(agc_word)(s.words[5]+1u);s.words[5]=target;if(!standard_lead_bank_return(&s,&io,STANDARD_LEAD_MISSION))return 1;}
   if(phase==2u&&!standard_lead_stage(&s,STANDARD_LEAD_MISSION,STANDARD_LEAD_BANK_RETURN0,0))return 1;
   if(io.superbank!=0160||io.interrupt_enabled!=0)return 1;
   standard_lead_dump((unsigned)n*3u+phase,0,&s);
  }
 }return 0;}
