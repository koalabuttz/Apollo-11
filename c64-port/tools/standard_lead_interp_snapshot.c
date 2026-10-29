/* SPDX-License-Identifier: MIT */
#include "standard_lead_interp_cases.h"
#include "standard_lead_dump.h"
int main(void){uint16_t n;unsigned phase;apollo_overlay_state s;p66_bias_io io;
 for(n=0;n<STANDARD_LEAD_INTERP_CASES;++n){standard_lead_interp_input(&s,STANDARD_LEAD_MISSION,n);io.superbank=0160;io.interrupt_enabled=0;
  if(!standard_lead_stage(&s,STANDARD_LEAD_MISSION,STANDARD_LEAD_INTERP_PREPARE,0)||!standard_lead_interp_control(&s,&io,STANDARD_LEAD_MISSION,STANDARD_LEAD_CALL))return 1;
  for(phase=0;phase<4u;++phase){
   if(phase==1u&&!standard_lead_interp_control(&s,&io,STANDARD_LEAD_MISSION,STANDARD_LEAD_RVQ))return 1;
   if(phase==2u&&!standard_lead_interp_control(&s,&io,STANDARD_LEAD_MISSION,STANDARD_LEAD_EXIT))return 1;
   if(phase==3u&&(!standard_lead_stage(&s,STANDARD_LEAD_MISSION,STANDARD_LEAD_INTERP_RESUME,0)||!standard_lead_interp_control(&s,&io,STANDARD_LEAD_MISSION,STANDARD_LEAD_FINISH)))return 1;
   if(io.superbank!=0160||io.interrupt_enabled!=1)return 1;
   standard_lead_dump((unsigned)n*4u+phase,0,&s);
  }
 }return 0;}
