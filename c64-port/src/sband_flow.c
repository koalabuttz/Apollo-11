/* SPDX-License-Identifier: MIT */
#include "sband_flow.h"
#include "lm_interbank.h"
#include "cm_interbank_complete.h"
#include "rtb_angles.h"
#include "cm_powered_interface.h"
#include "cm_powered_trig.h"
static void bank(apollo_overlay_state *s,agc_word fb){s->words[4]=fb;s->words[6]=(agc_word)(fb|(s->words[3]>>8));}
static bool context(const apollo_overlay_state *s,const p66_bias_io *io){unsigned i;
 if(!s||!io||s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||s->words[6]!=(s->words[4]|(s->words[3]>>8))||(io->superbank&~0160u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||s->words[067]||s->words[0120]<0401u||s->words[0120]>0661u||(s->words[0120]-0401u)%054u)return false;
 for(i=3;i<2048u;++i){if(s->words[i]>AGC_MASK)return false;}
 return true;
}
bool sband_flow(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission,enum sband_flow_entry entry){bool cm;
 if((mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||(unsigned)entry>SBAND_FLOW_CM_TIME_TRIG||!context(s,io))return false;
 cm=mission==APOLLO_FIXED_CM;
 if(entry==SBAND_FLOW_TIME_PIPELINE||entry==SBAND_FLOW_CM_TIME_TRIG){unsigned i;
  if(entry==SBAND_FLOW_CM_TIME_TRIG&&!cm)return false;
  if(!sband_flow(s,io,mission,SBAND_FLOW_TIME_FRONT))return false;
  if(rtb_angles(s,io,cm?RTB_CM:RTB_LM,RTB_LOAD_TIME)!=RTB_OK||!sband_flow(s,io,mission,SBAND_FLOW_AFTER_TIME))return false;
  if(entry==SBAND_FLOW_TIME_PIPELINE)return true;
  if(cm_powered_interface_start(s,0)!=CM_POWERED_INTERFACE_OK||cm_powered_cdu_start(s,1)!=CM_POWERED_TRIG_OK)return false;
  for(i=0;i<3u;++i){if(cm_powered_cdu_cosine(s)!=CM_POWERED_TRIG_OK||cm_powered_cdu_sine(s)!=CM_POWERED_TRIG_OK||cm_powered_cdu_next(s)!=CM_POWERED_TRIG_OK)return false;}
  if(cm_powered_interface_reentry(s,&io->interrupt_enabled)!=CM_POWERED_INTERFACE_OK||cm_powered_interface_return(s)!=CM_POWERED_INTERFACE_OK)return false;
  return sband_flow(s,io,mission,SBAND_FLOW_AFTER_CDU);
 }
 if(entry==SBAND_FLOW_TIME_FRONT){
  if(s->words[4]!=064000u||s->words[5]!=(cm?06037u:06070u)||s->words[0164]!=(cm?03655u:03611u)||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[023])return false;
  s->words[1]=agc_extend(agc_correct(s->words[1]));
  if(!cm){s->words[0116]=s->words[0166]=s->words[0120];}
  s->words[020]=010006;s->words[023]=cm?0153u:0;
  s->words[0164]=cm?03656u:03613u;s->words[0117]=cm?045510u:021573u;
  bank(s,02000);s->words[0]=agc_extend(s->words[0117]);s->words[2]=cm?02500u:02453u;s->words[5]=cm?04666u:04622u;
  if(!cm)return lm_interbank(s,io,LM_IB_SWCALL,0,0);
  else {cm_native_io metadata={0};bool ok;metadata.superbank=io->superbank;metadata.interrupt_enabled=io->interrupt_enabled;ok=cm_interbank_complete(s,&metadata,CM_IC_SWCALL,0,0);if(ok){io->superbank=metadata.superbank;io->interrupt_enabled=metadata.interrupt_enabled;}return ok;}
 }
 if(entry==SBAND_FLOW_AFTER_CDU){
  if(!cm||s->words[5]!=06037u||s->words[4]!=064000u||s->words[0165]!=s->words[6]||s->words[0164]!=03660u||s->words[023]||s->words[0163]||s->words[0115]!=040000u)return false;
 }else if(s->words[5]!=(cm?06030u:06061u)||s->words[4]!=(cm?044000u:020000u)||s->words[0165]!=(agc_word)(064000u|(s->words[3]>>8))||s->words[0164]!=(cm?03656u:03613u)||s->words[023]!=(cm?0153u:0)||s->words[0163])return false;
 s->words[1]=agc_extend(agc_correct(s->words[1]));
 if(!cm||entry==SBAND_FLOW_AFTER_CDU){unsigned destination=s->words[0120]+040u;agc_word oldlow=s->words[destination+1u];
  s->words[0116]=(agc_word)destination;s->words[destination]=s->words[0154];s->words[destination+1u]=s->words[0155];s->words[1]=agc_extend(oldlow);s->words[2]=3;
 }
 s->words[s->words[0120]+052u]=agc_correct(agc_add_register(agc_add_register(agc_extend(064000),agc_extend(076000)),agc_extend(cm?(entry==SBAND_FLOW_AFTER_CDU?03661u:03657u):03615u)));
 s->words[020]=(cm&&entry!=SBAND_FLOW_AFTER_CDU)?050006u:040006u;s->words[023]=0;s->words[0117]=cm?(entry==SBAND_FLOW_AFTER_CDU?027114u:047477u):027100u;
 bank(s,(cm&&entry!=SBAND_FLOW_AFTER_CDU)?046000u:026000u);s->words[0165]=s->words[6];s->words[0164]=cm?(entry==SBAND_FLOW_AFTER_CDU?03114u:03477u):03100u;s->words[0115]=(cm&&entry!=SBAND_FLOW_AFTER_CDU)?040000u:0;
 s->words[0]=agc_extend(s->words[0115]);s->words[5]=cm?06037u:06070u;
 return true;
}
