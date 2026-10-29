/* SPDX-License-Identifier: MIT */
#include "cm_display_control_cases.h"
#include "cm_clockjob_cases.h"
void cm_display_control_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){uint16_t k=(uint16_t)(n%8192u);static const agc_word pc[3]={02707,02715,02720},edge[8]={0,077777,1,077776,037777,040000,012345,065432};cm_clockjob_input(s,io,k);s->words[0]=agc_extend(k<128u?edge[k%8u]:(agc_word)(((uint32_t)k*977u)&AGC_MASK));s->words[3]=(agc_word)(((k/4u)%8u)*0400u);s->words[4]=020000;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));s->words[5]=pc[n/8192u];s->words[0133]=(agc_word)(k&07777u);s->words[0134]=(agc_word)((k%32u)*02000u);io->superbank=(agc_word)((k%8u)*020u);}
