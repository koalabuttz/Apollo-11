/* SPDX-License-Identifier: MIT */
#include "sband_shift_cases.h"
void sband_shift_input(apollo_overlay_state *s,agc_word *irq,agc_word *super,
                      uint32_t id,enum apollo_fixed_mission *mission,uint8_t *count)
{
 unsigned i,n=(unsigned)(id%32768u),group=(unsigned)(id/32768u),eb=n%8u;uint32_t seed=UINT32_C(0x524f3553)+(n/32u)*UINT32_C(0x9e3779b9);
 *mission=group>=8u?APOLLO_FIXED_CM:APOLLO_FIXED_LM;*count=(uint8_t)(group%8u+1u);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_word)(n*43u&AGC_MASK);
 s->words[3]=(agc_word)(eb*0400u);s->words[4]=0;s->words[6]=(agc_word)eb;s->words[7]=0;s->words[5]=02121;
 s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 for(i=0;i<7u;++i){s->words[0154u+i]=(agc_word)(n*(2u*i+1u)&AGC_MASK);}
 s->words[0163]=077776;s->words[020]=(agc_word)(020000u+*count-1u);
 *super=(agc_word)(((n/8u)%8u)*020u);*irq=(agc_word)(n&1u);
}
