/* SPDX-License-Identifier: MIT; caller-boundary guard/ownership checks. */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "standard_lead_target.h"
#include "standard_lead_bank.h"
#include "standard_lead_wake.h"
#include "standard_lead_cm_cases.h"
int main(void){apollo_overlay_state s,before;p66_bias_io io,oldio;cm_native_io cmio,oldcm;unsigned mission;
 for(mission=0;mission<2u;++mission){
  standard_lead_target_input(&s,&io,(enum standard_lead_mission)mission,1,31);
  assert(standard_lead_stage(&s,(enum standard_lead_mission)mission,STANDARD_LEAD_BANK_PREPARE,0));io.superbank=1;before=s;oldio=io;
  assert(!standard_lead_bank_call(&s,&io,(enum standard_lead_mission)mission));assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io));
  standard_lead_target_input(&s,&io,(enum standard_lead_mission)mission,2,191);
  assert(standard_lead_stage(&s,(enum standard_lead_mission)mission,STANDARD_LEAD_INTERP_PREPARE,0));s.words[0120]=0402;before=s;oldio=io;
  assert(!standard_lead_interp_control(&s,&io,(enum standard_lead_mission)mission,STANDARD_LEAD_CALL));assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io));
 }
 standard_lead_target_input(&s,&io,STANDARD_LEAD_LM,3,255);io.interrupt_enabled=1;before=s;oldio=io;
 assert(standard_lead_wake_lm(&s,&io)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io));
 standard_lead_cm_input(&s,&cmio,1025);cmio.superbank=0100;before=s;oldcm=cmio;
 assert(standard_lead_cm_test(&s,&cmio,STANDARD_LEAD_REDO)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&cmio,&oldcm,sizeof cmio));
 standard_lead_cm_input(&s,&cmio,1025);s.words[0167]=0;before=s;oldcm=cmio;
 assert(standard_lead_cm_test(&s,&cmio,STANDARD_LEAD_REDO)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&cmio,&oldcm,sizeof cmio));
 standard_lead_cm_input(&s,&cmio,1025);s.words[067]=1;before=s;oldcm=cmio;
 assert(standard_lead_cm_test(&s,&cmio,STANDARD_LEAD_REDO)==EXEC_OUT_OF_SCOPE);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&cmio,&oldcm,sizeof cmio));
 puts("PASS: lead-in caller guards preserve complete image and latches, including invalid VAC base/producer state, Executive pointer and physical bank selection");return 0;
}
