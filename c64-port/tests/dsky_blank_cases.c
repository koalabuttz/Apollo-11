/* SPDX-License-Identifier: MIT */
#include "dsky_blank_cases.h"
#include "p66_bias_cases.h"
void dsky_blank_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1};uint16_t i,cm=n>=4096u,stage=(uint16_t)((n/2048u)%2u);
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/8u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/8u)%8u));s->words[1]=raw[(n/64u)%8u];s->words[0777]=raw[(n/128u)%4u];s->words[0123]=(uint16_t)(n%8u);
 for(i=0;i<12;++i)s->words[01023+i]=(uint16_t)(n%3u ? (s->words[01023+i]|1u) : 0u);
 s->words[01025]|=1;s->words[01032]|=1;s->words[01016]=raw[(n/256u)%4u];s->words[0130]=raw[(n/64u)%4u];s->words[0131]=raw[(n/128u)%4u];
 io->superbank=0100;io->interrupt_enabled=(uint16_t)((n/16u)%2u);s->words[5]=cm ? 03412u : 03505u;
 if(stage){s->words[5]=cm ? 05230u : 05166u;s->words[0]=agc_extend((uint16_t)(((n/8u)%32u)*02000u|((n/256u)%8u)*020u|(n%8u)));s->words[1]=(uint16_t)(02000u+(n%512u));io->superbank=(uint16_t)(((n/64u)%8u)*020u);}
}
