/* SPDX-License-Identifier: MIT */
#include "cm_priolarm_cases.h"
#include "cm_alarm_record_cases.h"
void cm_priolarm_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){(void)cm_alarm_record_input(s,io,n%CM_ALARM_RECORD_CASES);s->words[0]=(agc_register)n;s->words[2]=(agc_register)(n^0177777u);s->words[3]=(agc_word)((n%8u)*0400u);s->words[4]=020000;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));s->words[5]=03766;s->words[0133]=(agc_word)(0100u+n%07677u);s->words[0134]=(agc_word)((((n/8u)%32u)*02000u)|((n/256u)%8u)|(((n/2048u)%8u)*020u));}
