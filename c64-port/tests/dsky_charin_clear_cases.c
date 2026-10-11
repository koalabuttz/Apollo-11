/* SPDX-License-Identifier: MIT */
#include "dsky_charin_clear_cases.h"
#include "p66_bias_cases.h"
void dsky_charin_clear_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){
 static const uint16_t counts[]={0,1,4,5,9,10,14,16,17,18,19,077777},raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},cadrs[]={0,077777,1,077776},cl[]={0,077777,077776,077775,040000,040001,077774,040002},nout[]={0,077777,037777,040000,1,077776};uint16_t i;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/2u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/2u)%8u));s->words[5]=n%8192u<4096u ? 02077 : 02000;s->words[0154]=036;s->words[1]=raw[(n/64u)%8u];s->words[0777]=counts[(n/2u)%12u];if(n%2u&&s->words[0777]!=077777)s->words[0777]=agc_negate(s->words[0777]);s->words[01000]=(uint16_t)((s->words[01000]&077774u)|((n/24u)%4u));s->words[01042]=cadrs[(n/96u)%4u];s->words[01015]=cl[(n/384u)%8u];s->words[01016]=nout[(n/256u)%6u];
 for(i=0;i<11;++i)if(s->words[01023+i]==0)s->words[01023+i]=1;
 /* 2BLANK accepts clean +0; keep isolated DSPIN and verb words nonzero. */
 if(n%16u==0){static const uint16_t paired_words[]={0,1,3,4,5,6};for(i=0;i<6;++i)s->words[01023+paired_words[i]]=0;}
 if(n>=8192u){s->words[01015]=(uint16_t)(n%3u==0 ? 037777u : 1u+n%128u);s->words[01001]=(uint16_t)(n%100u);}
 io->control.superbank=0100;io->control.interrupt_enabled=(uint16_t)(n%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
