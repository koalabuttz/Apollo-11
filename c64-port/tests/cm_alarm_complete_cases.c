/* SPDX-License-Identifier: MIT */
#include "cm_alarm_complete_cases.h"
#include "cm_alarm_image_cases.h"
void cm_alarm_complete_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){uint32_t caller=n/8u;cm_alarm_image_input(s,io,caller*4u+n%4u);s->words[3]=(agc_word)((caller/512u%8u)*0400u);if(n%4u==CM_RECORD_ALARM&&s->words[2]>=01400u&&s->words[2]<01500u&&s->words[3]==0)s->words[3]=0400;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));io->interrupt_enabled=(agc_word)((n/4u)%2u);}
