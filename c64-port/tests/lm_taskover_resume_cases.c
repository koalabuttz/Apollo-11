/* SPDX-License-Identifier: MIT */
#include "lm_taskover_resume_cases.h"
void lm_taskover_resume_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *c11,agc_word *c14,uint16_t n){uint32_t random=n+3943u;uint16_t i;static const uint16_t values[8]={0,077777,1,0177777,040000,0137777,0100000,0177776};
 for(i=0;i<2048u;++i){random=random*1664525u+1013904223u;s->words[i]=(uint16_t)((random>>16)&077777u);}
 s->words[0]=values[n%8u];s->words[1]=values[(n/8u)%8u];s->words[2]=values[(n/64u)%8u];s->words[3]=(uint16_t)((n%8u)*0400u);s->words[4]=(uint16_t)(((n/64u)%32u)*02000u);s->words[6]=(uint16_t)(s->words[4]|(n%8u));s->words[5]=05261;s->words[7]=0;s->words[0734]=AGC_NEG_ZERO;
 s->words[015]=(uint16_t)(03215u);s->words[017]=010067u;s->words[016]=(uint16_t)(((n/128u)%32u)*02000u+((n/8u)%8u)+((n/512u)%8u)*020u);
 s->words[010]=(uint16_t)(values[(n/16u)%8u]&077777u);s->words[011]=(uint16_t)(values[(n/32u)%8u]&077777u);s->words[012]=(uint16_t)(values[(n/128u)%8u]&077777u);
 io->superbank=(uint16_t)(((n/512u)%8u)*020u);io->interrupt_enabled=(uint16_t)(n%2u);*c11=(agc_word)(((uint32_t)n*1537u)&077777u);*c14=(agc_word)(n&0777u);
}
