/* SPDX-License-Identifier: MIT */
#include "dsky_two_digit_cases.h"
#include "p66_bias_cases.h"
void dsky_two_digit_prepare(apollo_overlay_state *s,uint16_t n){uint16_t i;p66_bias_prepare(s,n);s->words[3]=(uint16_t)(((n/100u)%8u)*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/100u)%8u));s->words[2]=(uint16_t)(04000+n%03777u);s->words[5]=03306;s->words[0]=(uint16_t)(n%100u);s->words[0777]=(uint16_t)(1u+2u*((n/100u)%11u));for(i=0;i<11;++i)s->words[01023+i]=(uint16_t)(s->words[01023+i]|04000u);}
