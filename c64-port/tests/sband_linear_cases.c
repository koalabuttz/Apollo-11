/* SPDX-License-Identifier: MIT */
#include "sband_linear_cases.h"
void sband_linear_input(apollo_overlay_state *s,agc_word *irq,agc_word *super,agc_word fixed[6],uint8_t *use_fixed,uint32_t id,enum apollo_fixed_mission *mission,enum sband_vector_operation *op){
 unsigned i,n=(unsigned)(id%8192u),g=(unsigned)(id/8192u),kind=g%4u,eb=n%8u;uint32_t seed=UINT32_C(0x53424c49)+(n/16u)*UINT32_C(0x9e3779b9);
 static const agc_word aliases[24]={030,031,036,040,0100,0116,0117,0120,0121,0122,0126,0130,0131,0133,0134,0135,0136,0140,0142,0154,0155,0156,0157,0161};
 *mission=g>=8u?APOLLO_FIXED_CM:APOLLO_FIXED_LM;*use_fixed=(uint8_t)((g/4u)&1u);*op=kind<2u?SBAND_VXSC:kind==2u?SBAND_VAD:SBAND_VSU;
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 for(i=0;i<6u;++i)fixed[i]=(agc_word)((n*(2u*i+1u)+i*02143u)&AGC_MASK);
 if(n<32u){for(i=0;i<6u;++i)fixed[i]=0;fixed[(n/2u)%6u]=(agc_word)(n&1u?077777u:020000u);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_word)((n*43u)&AGC_MASK);
 s->words[3]=(agc_word)(eb*0400u);s->words[4]=*use_fixed?(*mission==APOLLO_FIXED_CM?054000u:046000u):0;s->words[6]=(agc_word)(s->words[4]|eb);s->words[7]=0;
 s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[0116]=*use_fixed?(*mission==APOLLO_FIXED_CM?03332u:02517u):aliases[n%24u];
 if(!*use_fixed&&(n/24u)%2u)s->words[0116]=(agc_word)(01400u+(n%0373u));
 if(!*use_fixed&&eb==0u&&s->words[0116]>=01400u&&(s->words[0116]&0377u)<030u)s->words[0116]=(agc_word)(s->words[0116]+030u);
 s->words[0163]=kind==1u?0u:077776u;
 s->words[5]=*mission==APOLLO_FIXED_LM?(*op==SBAND_VXSC?07401u:*op==SBAND_VAD?06751u:06747u):(*op==SBAND_VXSC?07350u:*op==SBAND_VAD?06720u:06716u);
 if(n<16u){for(i=0;i<7u;++i)s->words[0154u+i]=(agc_word)(n&1u?077777u:0);if(n>=2u)s->words[0154]=(agc_word)(n*01000u);}
 *irq=(agc_word)(n&1u);*super=(agc_word)(((n/8u)%8u)*020u);
}
