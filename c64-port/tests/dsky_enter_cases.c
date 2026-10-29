/* SPDX-License-Identifier: MIT */
#include "dsky_enter_cases.h"
#include "p66_bias_cases.h"
void dsky_enter_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},requests[]={0,077777,1,037777,074346,075777,040000,077776};uint16_t cm=n>=4096u,entry=(uint16_t)((n/2048u)%2u);
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/8u)%8u*0400u);s->words[4]=entry ? 062000u : 060000u;s->words[6]=(uint16_t)(s->words[4]|((n/8u)%8u));s->words[5]=entry ? 02002u : cm ? 02000u : 02077u;s->words[0154]=034;s->words[1]=raw[(n/64u)%8u];s->words[01013]=requests[n%8u];s->words[0777]=raw[(n/32u)%4u];s->words[01000]=(uint16_t)((s->words[01000]&077774u)|((n/16u)%4u));s->words[01042]=raw[(n/128u)%4u];io->control.superbank=0100;io->control.interrupt_enabled=(uint16_t)((n/8u)%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
