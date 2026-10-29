/* SPDX-License-Identifier: MIT */
#include "lm_danzig_change_cases.h"
void lm_danzig_change_input(apollo_overlay_state*m,p66_bias_io*io,uint16_t n){lm_interpreter_change_input(m,io,n);m->words[5]=(n&2u)?06063:06061;m->words[3]=(uint16_t)(((n/16u)%8u)*0400u);m->words[4]=(uint16_t)(((n/128u)%32u)*02000u);m->words[6]=(uint16_t)(m->words[4]|(m->words[3]>>8));}
