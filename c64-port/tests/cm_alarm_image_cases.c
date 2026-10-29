/* SPDX-License-Identifier: MIT */
#include "cm_alarm_image_cases.h"
#include "cm_alarm_record_cases.h"
void cm_alarm_image_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned caller=(n/4u)%4030u;(void)cm_alarm_record_input(s,io,n%CM_ALARM_RECORD_CASES);s->words[2]=(agc_word)(0100u+caller);s->words[3]=(agc_word)((n/16120u)*0400u);s->words[4]=(agc_word)((caller%32u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));if(n%4u==CM_RECORD_ALARM&&s->words[2]>=01400u&&s->words[2]<01500u&&s->words[3]==0){s->words[3]=0400;s->words[6]=(agc_word)(s->words[4]|1u);}io->superbank=(agc_word)((caller/32u%8u)*020u);io->interrupt_enabled=(agc_word)((caller/256u)&1u);}
