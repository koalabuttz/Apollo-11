/* SPDX-License-Identifier: MIT */
#include "ephemeris_add_cases.h"
void ephemeris_add_input(apollo_overlay_state *m,agc_word *irq,agc_word *super,uint32_t n,enum apollo_fixed_mission mission)
{
 static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 static const agc_word address[12]={01000,01500,0121,0122,0130,0152,0153,0154,0155,0156,01775,0377};
 uint32_t seed=UINT32_C(0x45504845)+(n/32u)*UINT32_C(0x9e3779b9);unsigned i,at;
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=(agc_register)(n*257u);m->words[1]=(agc_register)(n*977u);m->words[2]=(agc_word)(n&AGC_MASK);
 m->words[3]=(agc_word)((n%8u)*0400u);m->words[4]=(agc_word)(((n/8u)%32u)*02000u);m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));
 m->words[5]=mission==APOLLO_FIXED_LM?07071u:07040u;m->words[7]=0;m->words[0116]=n<256u?01000u:address[(n/256u)%12u];
 *irq=(agc_word)(n&1u);*super=(agc_word)(((n/32u)%8u)*020u);
 for(i=0;i<3u;++i)m->words[0154u+i]=(agc_word)(n<256u?edge[(n+i*(n<128u?0u:3u))%16u]:(agc_word)((seed>>(i*3u))&AGC_MASK));
 at=m->words[0116];if(at>=01400u)at=(unsigned)(m->words[3]+(at&0377u));
 for(i=0;i<3u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);m->words[at+i]=n<256u?edge[(n/16u+i*(n<128u?0u:7u))%16u]:(agc_word)((seed>>11)&AGC_MASK);}
}
