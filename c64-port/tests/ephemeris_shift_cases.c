/* SPDX-License-Identifier: MIT */
#include "ephemeris_shift_cases.h"
void ephemeris_shift_input(apollo_overlay_state *m,uint16_t *irq,uint16_t *super,uint32_t n,enum apollo_fixed_mission mission)
{
 static const uint16_t count[3]={5,8,16};static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};uint32_t seed=UINT32_C(0x45504853)+(n/32u)*UINT32_C(0x9e3779b9);unsigned i;(void)mission;
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=0177;m->words[1]=(agc_register)(n*977u);m->words[2]=(agc_word)(n&AGC_MASK);m->words[3]=(agc_word)((n%8u)*0400u);m->words[4]=0;m->words[6]=(agc_word)(m->words[3]>>8);m->words[5]=02214;m->words[7]=0;m->words[0116]=(agc_word)(020200u+count[n%3u]);m->words[0163]=0;*irq=(agc_word)(n&1u);*super=(agc_word)(((n/32u)%8u)*020u);
 for(i=0;i<3u;++i)m->words[0154u+i]=(agc_word)(n<256u?edge[(n+i*(n<128u?0u:3u))%16u]:(agc_word)((seed>>(i*3u))&AGC_MASK));
}
