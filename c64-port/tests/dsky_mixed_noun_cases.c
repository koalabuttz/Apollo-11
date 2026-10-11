/* SPDX-License-Identifier: MIT */
#include "dsky_mixed_noun_cases.h"
#include "p66_bias_cases.h"
void dsky_mixed_noun_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},verbs[]={0,1,6,7,27,99,077777};uint16_t i;p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/13u)%8u*0400u);s->words[4]=062000;s->words[6]=(uint16_t)(062000u|((n/13u)%8u));s->words[5]=02221;s->words[1]=raw[(n/64u)%8u];s->words[01001]=verbs[(n/128u)%7u];s->words[0146]=raw[(n/32u)%4u];s->words[0153]=(uint16_t)((n%14u)|((n/14u)%14u)<<5|((n/196u)%14u)<<10);
 for(i=0;i<3;++i)s->words[0150+i]=(uint16_t)(((n*73u+31u*i)&03777u)|(((n/16u)%8u)<<11));
 if(s->words[0146]==0||(s->words[01001]!=077777u&&s->words[01001]>6u))s->words[0153]=077777;
 io->superbank=(uint16_t)(0100u+((n/32u)%4u)*020u);io->interrupt_enabled=(uint16_t)((n/16u)%2u);}
