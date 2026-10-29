/* SPDX-License-Identifier: MIT; diagnostic native composition. */
#include "standard_lead_target.h"
#include "standard_lead_bank.h"
#include "standard_lead_wake.h"
#include "standard_lead_cm.h"
#include "standard_lead_fixed_inputs.h"
bool standard_lead_target_execute(apollo_overlay_state *s,p66_bias_io *io,enum standard_lead_mission mission,uint16_t kind,uint16_t n,uint16_t phase,uint16_t *status){
 unsigned p;enum exec_status result;cm_native_io cmio={0};*status=0;
 if(kind==0u)return standard_lead_stage(s,mission,(enum standard_lead_phase)(n/512u),standard_lead_fixed_inputs[mission][n%64u]);
 if(kind==1u){if(phase>2u||!standard_lead_stage(s,mission,STANDARD_LEAD_BANK_PREPARE,0)||!standard_lead_bank_call(s,io,mission))return false;
  if(phase>=1u){agc_word target=s->words[2];s->words[2]=(agc_word)(s->words[5]+1u);s->words[5]=target;if(!standard_lead_bank_return(s,io,mission))return false;}
  return phase<2u||standard_lead_stage(s,mission,STANDARD_LEAD_BANK_RETURN0,0);
 }
 if(kind==2u){if(phase>3u||!standard_lead_stage(s,mission,STANDARD_LEAD_INTERP_PREPARE,0)||!standard_lead_interp_control(s,io,mission,STANDARD_LEAD_CALL))return false;
  for(p=1;p<=phase;++p){if(p==3u&&!standard_lead_stage(s,mission,STANDARD_LEAD_INTERP_RESUME,0))return false;if(!standard_lead_interp_control(s,io,mission,(enum standard_lead_interp_boundary)p))return false;}return true;
 }
 if(kind==3u){if(mission==STANDARD_LEAD_LM)result=standard_lead_wake_lm(s,io);else{cmio.superbank=io->superbank;cmio.interrupt_enabled=io->interrupt_enabled;result=standard_lead_wake_cm(s,&cmio);io->superbank=cmio.superbank;io->interrupt_enabled=cmio.interrupt_enabled;}return result==EXEC_OK||result==EXEC_NOT_FOUND;}
 if(kind==4u&&mission==STANDARD_LEAD_CM){cmio.superbank=io->superbank;cmio.interrupt_enabled=io->interrupt_enabled;result=standard_lead_cm_test(s,&cmio,(enum standard_lead_cm_entry)(n/512u));io->superbank=cmio.superbank;io->interrupt_enabled=cmio.interrupt_enabled;*status=(uint16_t)result;return result==EXEC_OK||result==EXEC_NO_CORE;}
 if(kind==5u)return standard_lead_stage(s,mission,STANDARD_LEAD_INTERP_RESUME,0)&&standard_lead_interp_control(s,io,mission,STANDARD_LEAD_FINISH);
 return false;
}
