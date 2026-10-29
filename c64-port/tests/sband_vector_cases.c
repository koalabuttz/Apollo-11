/* SPDX-License-Identifier: MIT */
#include "sband_vector_cases.h"
void sband_vector_input(apollo_overlay_state *s,agc_word *irq,agc_word *super,uint32_t n,enum apollo_fixed_mission mission){
 static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 static const agc_word addr[8]={01000,01500,0122,0124,0130,0135,0154,0157};
 static const uint8_t map[6]={0,1,3,4,5,6};
 uint8_t projection=(uint8_t)((n&32768u)!=0),fixed=(uint8_t)((n&65536u)!=0);
 uint32_t seed;unsigned i,at;
 n &=32767u;seed=UINT32_C(0x5342414e)+(n/32u)*UINT32_C(0x9e3779b9);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_word)(n&AGC_MASK);
 s->words[3]=(agc_word)((n%8u)*0400u);s->words[4]=(agc_word)(((n/8u)%32u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));
 s->words[5]=projection?(mission==APOLLO_FIXED_CM?07374:07425):(mission==APOLLO_FIXED_CM?07427:07460);s->words[7]=0;s->words[035]=s->words[036]=s->words[047]=0;
 for(i=050;i<=054;++i)s->words[i]=0;
 s->words[0116]=n<256u?01000:addr[(n/256u)%8u];s->words[0163]=077776;
 *irq=(agc_word)(n&1u);*super=(agc_word)(((n/32u)%8u)*020u);
 for(i=0;i<6u;++i){seed=seed*1664525u+1013904223u;s->words[0154u+map[i]]=n<256u?edge[(n+i*(n<128u?0u:3u))%16u]:(agc_word)((seed>>11)&AGC_MASK);}
 at=s->words[0116]<01400u?s->words[0116]:(unsigned)(s->words[3]+(s->words[0116]&0377u));
 for(i=0;i<6u;++i){seed=seed*1664525u+1013904223u;s->words[at+i]=n<256u?edge[(n/16u+i*(n<128u?0u:7u))%16u]:(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0163]=077776;
 if(fixed){
  unsigned axis=n<256u?(unsigned)((n/16u)%3u):0u;
  s->words[0116]=(agc_word)((mission==APOLLO_FIXED_CM?03332u:02517u)-2u*axis);
  s->words[4]=mission==APOLLO_FIXED_CM?054000u:046000u;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));
 }
}
void sband_vector_fixed_input(agc_word factor[6],uint32_t n){
 uint32_t seed=UINT32_C(0x46697865)+(n&32767u)*UINT32_C(0x9e3779b9);unsigned i; n&=32767u;
 for(i=0;i<6u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;factor[i]=n<256u?(agc_word)(i==2u*((n/16u)%3u)?020000u:0u):(agc_word)(seed&AGC_MASK);}
}
