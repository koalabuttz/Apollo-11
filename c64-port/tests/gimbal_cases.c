/* SPDX-License-Identifier: MIT
 * Input-only original-source and native constructors. */
#include "gimbal_cases.h"
enum apollo_fixed_mission gimbal_mission(uint16_t n){return n<1280u?APOLLO_FIXED_LM:APOLLO_FIXED_CM;}
static uint16_t ext(uint16_t w){return (uint16_t)(w|((w&040000u)<<1));}
void gimbal_prepare(apollo_overlay_state *s,gimbal_io *io,uint16_t n){
 uint16_t local=(uint16_t)(n%1280u),variant=(uint16_t)(local/160u),i;
 uint16_t bank=(uint16_t)((local/4u)%8u),base=(uint16_t)(0401u+((local/32u)%5u)*054u);
 uint16_t rate=(uint16_t)(2u*(local%4u));
 for(i=0;i<APOLLO_SHARED_WORDS;++i)s->words[i]=(uint16_t)((251u*i+(local%16u)*947u+99u)&077777u);
 for(i=0;i<3u;++i)s->words[i]=(uint16_t)(ext(s->words[i])^((variant&1u)!=0?040000u:0));
 s->words[3]=(uint16_t)(bank<<8);s->words[4]=044000;s->words[6]=(uint16_t)(044000u|bank);s->words[7]=0;
 s->words[5]=n<1280u?06070:06037;s->words[023]=0;s->words[067]=0;
 s->words[0120]=base;s->words[0115]=040000;s->words[0165]=s->words[6];
 s->words[0164]=n<1280u?((variant&2u)!=0?02746:02744):02737;
 s->words[n<1280u?01325:01130]=rate;
 io->superbank=(uint16_t)(variant*020u);io->interrupt_enabled=(uint16_t)(variant&1u);
}
