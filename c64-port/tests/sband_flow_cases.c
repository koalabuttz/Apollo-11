/* SPDX-License-Identifier: MIT */
#include "sband_flow_cases.h"
void sband_flow_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t id,
                     enum apollo_fixed_mission *mission,enum sband_flow_entry *entry)
{
 unsigned i,n=(unsigned)(id%4096u),group=(unsigned)(id/4096u),eb=n%8u;
 uint32_t seed=UINT32_C(0x524f3546)+(n/32u)*UINT32_C(0x9e3779b9);
 *mission=group>=5u?APOLLO_FIXED_CM:APOLLO_FIXED_LM;
 *entry=(enum sband_flow_entry)(group%5u);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_register)(n*983u);
 s->words[3]=(agc_word)(eb*0400u);s->words[4]=064000;s->words[6]=(agc_word)(064000u|eb);s->words[7]=0;
 s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[067]=0;s->words[0120]=(agc_word)(0401u+(n%5u)*054u);
 s->words[0164]=*mission==APOLLO_FIXED_LM?03611u:03655u;
 s->words[0165]=s->words[6];s->words[0115]=040000;s->words[023]=0;
 s->words[5]=*mission==APOLLO_FIXED_LM?06070u:06037u;
 io->superbank=(agc_word)(0100u+((n/8u)%4u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
 if(*entry==SBAND_FLOW_AFTER_TIME){
  s->words[0]=0;s->words[1]=agc_extend(s->words[0155]);
  s->words[0154]=s->words[024];s->words[0155]=s->words[025];s->words[0156]=0;s->words[0163]=0;
  s->words[020]=010006;s->words[0117]=*mission==APOLLO_FIXED_LM?021573u:045510u;
  s->words[0164]=*mission==APOLLO_FIXED_LM?03613u:03656u;
  s->words[023]=*mission==APOLLO_FIXED_LM?0:0153u;
  s->words[0133]=*mission==APOLLO_FIXED_LM?02453u:02500u;s->words[0134]=02000;
  s->words[2]=*mission==APOLLO_FIXED_LM?04631u:04675u;
  s->words[4]=*mission==APOLLO_FIXED_LM?020000u:044000u;s->words[6]=(agc_word)(s->words[4]|eb);
  s->words[5]=*mission==APOLLO_FIXED_LM?06061u:06030u;
  if(*mission==APOLLO_FIXED_LM){s->words[0116]=s->words[0166]=s->words[0120];}
 }
 if(*entry==SBAND_FLOW_AFTER_CDU){
  s->words[5]=06037;s->words[0164]=03660;s->words[020]=7;s->words[023]=0;s->words[0163]=0;
 }
}
