/* SPDX-License-Identifier: MIT */
#include "dsky_charin_reset_cases.h"
#include "p66_bias_cases.h"
void dsky_charin_reset_input(apollo_overlay_state *s,dsky_reset_io *io,uint16_t n){
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},cadrs[]={0,077777,1,077776},corners[]={0,077777,04000,073777,1,077776,037777,040000};uint16_t i;
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/2u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/2u)%8u));s->words[5]=n<4096u ? 02077 : 02000;s->words[0154]=022;s->words[1]=raw[(n/64u)%8u];s->words[01042]=cadrs[(n/96u)%4u];
 if(n%2u==0)for(i=0;i<12;++i)s->words[01023+i]=corners[(n/16u+i)%8u];
 if(n%4u==0){s->words[01302]=corners[n/32u%8u];s->words[01303]=corners[n/64u%8u];s->words[01320]=corners[n/32u%8u];s->words[01321]=corners[n/64u%8u];s->words[0110]=corners[n/128u%8u];s->words[01331]=corners[n/128u%8u];}
 io->control.superbank=0100;io->control.interrupt_enabled=(uint16_t)(n%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->channel13=(uint16_t)((n*73u+05200u)&043777u);io->wrote_channels=0;io->restart_light=(uint16_t)((n/2u)%2u);
}
