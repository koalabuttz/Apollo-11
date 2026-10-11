/* SPDX-License-Identifier: MIT */
#include "dsky_charin_sign_cases.h"
#include "p66_bias_cases.h"
void dsky_charin_sign_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){
 static const uint16_t counts[]={14,9,4,0,19,18,077776,077777,037777,040000,21,15},raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},cadrs[]={0,077777,1,077776},cl[]={0,077777,1,077776,037777,040000,2,077775},nout[]={0,077777,037777,040000,1,077776};uint16_t i;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/2u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/2u)%8u));s->words[5]=n<4096u ? 02077 : 02000;s->words[0154]=n%2u ? 033 : 032;s->words[1]=raw[(n/64u)%8u];s->words[0777]=counts[(n/2u)%12u];s->words[01000]=(uint16_t)((s->words[01000]&077774u)|((n/24u)%4u));s->words[01042]=cadrs[(n/96u)%4u];s->words[01015]=cl[(n/384u)%8u];s->words[01016]=nout[(n/256u)%6u];
 for(i=0;i<11;++i)if(s->words[01023+i]==0)s->words[01023+i]=1;
 io->control.superbank=0100;io->control.interrupt_enabled=(uint16_t)(n%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
