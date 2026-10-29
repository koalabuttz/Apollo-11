/* SPDX-License-Identifier: MIT */
#include "sband_trig_cases.h"
void sband_trig_input(apollo_overlay_state *s,agc_word *irq,agc_word *super,uint32_t n,enum apollo_fixed_mission mission){
 static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};unsigned i;uint8_t cosine=(uint8_t)((n&65536u)!=0);uint32_t local=n&65535u,seed=UINT32_C(0x53425452)+(local/32u)*UINT32_C(0x9e3779b9);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(local*257u);s->words[1]=(agc_register)(local*977u);s->words[2]=(agc_word)(local&AGC_MASK);s->words[3]=(agc_word)((local%8u)*0400u);s->words[4]=0;s->words[6]=(agc_word)(s->words[3]>>8);s->words[5]=mission==APOLLO_FIXED_CM?(cosine?03516:03527):(cosine?03517:03530);s->words[7]=0;s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[0154]=(agc_word)(local&AGC_MASK);s->words[0155]=local<256u?edge[(local/16u)%16u]:(agc_word)((local*977u)&AGC_MASK);s->words[0156]=local<256u?edge[local%16u]:s->words[0156];s->words[0163]=0;*irq=(agc_word)(local&1u);*super=(agc_word)(((local/32u)%8u)*020u);
}
