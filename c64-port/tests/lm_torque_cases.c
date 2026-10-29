/* SPDX-License-Identifier: MIT
 * Input-only constructors, independently linked by original-source oracle. */
#include "lm_torque_cases.h"
static uint16_t extend(uint16_t w) {return (uint16_t)(w|((w&040000u)<<1));}
void lm_torque_prepare(apollo_overlay_state *s,lm_torque_io *io,uint16_t n) {
 static const uint16_t pulses[16]={0,077777,1,077776,0157,077620,0240,077537,
   0241,077536,0242,077535,037777,040000,020000,057777};
 static const uint16_t jets[16]={0,077777,1,077776,2,077775,4,077773,
   010,077767,01000,076777,037777,040000,020000,057777};
 uint16_t i,bank=(uint16_t)(n%8u),base=(uint16_t)(bank*0400u),p,j;
 for(i=0;i<APOLLO_SHARED_WORDS;++i)
  s->words[i]=(uint16_t)((251u*i+(n%16u)*947u+99u)&077777u);
 for(i=0;i<3u;++i) s->words[i]=extend(s->words[i]);
 s->words[0]^=(uint16_t)((n&8u)!=0 ? 040000u:0);
 s->words[1]^=(uint16_t)((n&16u)!=0 ? 040000u:0);
 s->words[2]^=(uint16_t)((n&32u)!=0 ? 040000u:0);
 s->words[3]=(uint16_t)(bank<<8);s->words[4]=034000;
 s->words[5]=LM_TORQUE_RATELOOP;s->words[6]=(uint16_t)(034000u|bank);s->words[7]=0;
 p=(uint16_t)((n/8u)%16u);j=(uint16_t)((n/128u)%16u);
 for(i=0;i<3u;++i) {
  s->words[base+0124u+i]=pulses[(p+5u*i)%16u];
  s->words[base+0121u+i]=jets[(j+7u*i)%16u];
 }
 if(n>=4096u)s->words[base+0124u]=(uint16_t)(n-4096u);
 s->words[base+0131u]=jets[(j+3u)%16u];s->words[base+0132u]=jets[(j+11u)%16u];
 for(i=0;i<6u;++i)s->words[base+0113u+i]=jets[(j+3u*i)%16u];
 io->superbank=(uint16_t)(((n/2048u)%8u)*020u);io->interrupt_enabled=0;
}
