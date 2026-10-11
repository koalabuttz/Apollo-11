/* SPDX-License-Identifier: MIT */
#include "cm_trig_cases.h"
uint32_t cm_trig_count(uint8_t selector){return selector<2u?CM_SINGLE_TRIG_CASES:CM_QUIC_TRIG_CASES;}
void cm_trig_input(apollo_overlay_state *m,agc_word *irq,uint16_t n,uint8_t selector){static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};uint32_t seed=UINT32_C(0x35555075)+((uint32_t)n>>8)*UINT32_C(0x9e3779b9)+selector;unsigned i;for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=n;m->words[1]=(agc_word)((uint32_t)n*37u+selector*8191u);m->words[2]=(agc_word)(02000u+(n&01777u));m->words[3]=(agc_word)(((n/32u)%8u)<<8);m->words[4]=046000;m->words[5]=selector<2u?(selector?05074:05075):03555;m->words[6]=(agc_word)(046000u|(m->words[3]>>8));m->words[7]=0;m->words[020]=0;m->words[023]=0;m->words[067]=0;m->words[0163]=(agc_word)(n&AGC_MASK);
 if(selector==2u){uint32_t x=(uint32_t)n-256u;for(i=0;i<6u;i+=2u)m->words[0766u+i]=n<256u?edge[(((uint32_t)n>>(2u*i))+i)&15u]:(agc_word)((x*(2u*i+1u)+43u*i)&AGC_MASK);m->words[0]=agc_extend((agc_word)(n&AGC_MASK));}
 *irq=(agc_word)(n&1u);
}
