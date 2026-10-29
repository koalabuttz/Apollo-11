/* SPDX-License-Identifier: MIT */
#include "dsky_verb_dispatch_cases.h"
#include "p66_bias_cases.h"
void dsky_verb_dispatch_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1};uint16_t verb=(uint16_t)(n%101u);p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/13u)%8u*0400u);s->words[4]=062000;s->words[6]=(uint16_t)(062000u|((n/13u)%8u));s->words[5]=02133;s->words[1]=raw[(n/64u)%8u];s->words[01001]=verb==100u ? 077777u : verb;io->superbank=(uint16_t)(0100u+((n/32u)%4u)*020u);io->interrupt_enabled=(uint16_t)((n/16u)%2u);}
