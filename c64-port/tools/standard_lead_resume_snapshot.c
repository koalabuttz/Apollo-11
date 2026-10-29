/* SPDX-License-Identifier: MIT */
#include "standard_lead_resume_cases.h"
#include "standard_lead_dump.h"
int main(void){uint16_t n;apollo_overlay_state s;p66_bias_io io;
 for(n=0;n<STANDARD_LEAD_RESUME_CASES;++n){standard_lead_resume_input(&s,STANDARD_LEAD_MISSION,n);io.superbank=0160;io.interrupt_enabled=(agc_word)(n/512u);
  if(!standard_lead_stage(&s,STANDARD_LEAD_MISSION,STANDARD_LEAD_INTERP_RESUME,0)||!standard_lead_interp_control(&s,&io,STANDARD_LEAD_MISSION,STANDARD_LEAD_FINISH)||io.superbank!=0160||io.interrupt_enabled!=n/512u)return 1;
  standard_lead_dump(n,0,&s);
 }return 0;}
