/* SPDX-License-Identifier: MIT */
#include "lm_idle_start_cases.h"
void lm_idle_start_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *c11,agc_word *c14,uint16_t n){uint32_t random=n+8371u;uint16_t i;static const uint16_t values[8]={0,077777,1,0177777,040000,0137777,0100000,0177776};
 for(i=0;i<2048u;++i){random=random*1664525u+1013904223u;s->words[i]=(uint16_t)((random>>16)&077777u);}
 s->words[0]=values[n%8u];s->words[1]=values[(n/8u)%8u];s->words[2]=values[(n/64u)%8u];s->words[3]=(uint16_t)((n%8u)*0400u);s->words[4]=02000;s->words[6]=(uint16_t)(02000u|(n%8u));s->words[5]=03214;s->words[7]=0;s->words[067]=0;s->words[0167]=(uint16_t)(022000u+0401u+(n%5u)*054u);
 s->words[0164]=(uint16_t)(02001u+(n/8u)%7u);s->words[0165]=(uint16_t)(((n/64u)%32u)*02000u+(n%8u)+((n/512u)%8u)*020u);
 if(n%8u==0){s->words[0164]=03275;s->words[0165]=064067;}
 io->superbank=(uint16_t)(((n/512u)%8u)*020u);io->interrupt_enabled=1;*c11=(agc_word)(((uint32_t)n*1537u)&077777u);*c14=(agc_word)(n&0777u);
}
