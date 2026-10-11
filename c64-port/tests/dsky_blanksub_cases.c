/* SPDX-License-Identifier: MIT */
#include "dsky_blanksub_cases.h"
#include "p66_bias_cases.h"
void dsky_blanksub_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1};uint16_t i,cm=n>=4096u;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/8u)%8u*0400u);s->words[4]=(uint16_t)((n/64u)%32u*02000u);s->words[6]=(uint16_t)(s->words[4]|((n/8u)%8u));s->words[0]=(uint16_t)((raw[(n/256u)%8u]&0177770u)|(n%8u));s->words[1]=raw[(n/64u)%8u];s->words[0777]=raw[(n/128u)%4u];s->words[0130]=raw[(n/64u)%4u];s->words[0131]=raw[(n/128u)%4u];s->words[2]=(uint16_t)(02000u+(n%512u));s->words[01012]=(uint16_t)((n/512u)%2u);s->words[01021]=(uint16_t)((s->words[01021]&057777u)|((n/1024u)%2u)*020000u);
 for(i=0;i<12;++i)s->words[01023+i]=(uint16_t)(n%3u ? (s->words[01023+i]|1u) : 0u);
 if(s->words[01012]==0&&(s->words[01021]&020000u)==0){s->words[01025]|=1;s->words[01032]|=1;}
 s->words[01016]=raw[(n/256u)%4u];io->superbank=(uint16_t)(((n/32u)%8u)*020u);io->interrupt_enabled=(uint16_t)((n/16u)%2u);s->words[5]=cm ? 04271u : 04255u;
}
