/* SPDX-License-Identifier: MIT */
#include "cm_sleep_cases.h"
#include "cm_basic_finish_cases.h"
void cm_sleep_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){static const agc_register raw[8]={0,0177777,1,0177776,040000,0137777,077777,0100000};cm_basic_finish_input(s,io,n);s->words[0]=agc_extend((agc_word)(((uint32_t)n*13u)&AGC_MASK));s->words[1]=raw[(n/8u)%8u];s->words[5]=05175;}
