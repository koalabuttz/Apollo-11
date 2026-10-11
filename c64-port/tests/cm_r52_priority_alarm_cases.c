/* SPDX-License-Identifier: MIT */
#include "cm_r52_priority_alarm_cases.h"
#include "cm_admission_complete_cases.h"
void cm_r52_priority_alarm_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned i,cores=(n/4u)%7u,vacs=n%5u;(void)cm_admission_complete_input(s,io,n%CM_ADMISSION_COMPLETE_CASES);s->words[0]=(agc_register)(n*37u);s->words[1]=(agc_register)(n*71u);s->words[2]=(agc_register)(n*97u);s->words[5]=02174;s->words[4]=030000;s->words[3]=(agc_word)((n%8u)*0400u);s->words[6]=(agc_word)(030000u|(s->words[3]>>8));s->words[0167]=(agc_word)(((n/32u)%31u+1u)*01000u+0401u);s->words[067]=AGC_NEG_ZERO;
 for(i=1;i<7u;++i){unsigned a=0154u+i*12u;s->words[a+8u]=03332;s->words[a+9u]=050066;s->words[a+10u]=0110;s->words[a+11u]=i<=cores?01110u:AGC_NEG_ZERO;}
 s->words[0400]=0;for(i=1;i<5u;++i)s->words[0400u+i*44u]=i<=vacs?0:(agc_word)(0400u+i*44u);
 io->superbank=(agc_word)(((n/8u)%8u)*020u);io->interrupt_enabled=(agc_word)((n/64u)&1u);io->executive_alarm=0;
}
