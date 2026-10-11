/* SPDX-License-Identifier: MIT */
#include "dsky_source_alarm_cases.h"
#include "p66_bias_cases.h"
void dsky_source_alarm_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},lm[]={02351,03420,03416,03434,02120,04364,04370,04374},cm[]={02351,03325,03323,03341,02120,04400,04404,04410};uint16_t entry=(uint16_t)(n%8u),type=(uint16_t)((n/8u)%4u);uint8_t mission=(uint8_t)(n>=4096u);
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/16u)%8u*0400u);s->words[4]=entry>=5u ? (uint16_t)(((n/32u)%32u)*02000u) : entry==0u ? 062000u : entry==4u ? 066000u : 060000u;s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));s->words[5]=mission ? cm[entry] : lm[entry];s->words[0]=raw[(n/64u)%8u];s->words[1]=raw[(n/128u)%8u];s->words[2]=(uint16_t)(07100u+n%0100u);s->words[0136]=type==0u ? 0u : type==1u ? (mission ? 04216u : 04202u) : type==2u ? (mission ? 04140u : 04124u) : 077777u;
 io->control.superbank=(uint16_t)((entry>=5u ? 0u : 0100u)+((n/128u)%(entry>=5u ? 8u : 4u))*020u);io->control.interrupt_enabled=(uint16_t)((n/256u)%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
