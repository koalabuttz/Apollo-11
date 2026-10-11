/* SPDX-License-Identifier: MIT */
#include "dsky_nv_key_cases.h"
#include "p66_bias_cases.h"
void dsky_nv_key_prepare(apollo_overlay_state *s,uint16_t n){uint16_t relative=(uint16_t)(n%2u ? 8 : 9);p66_bias_prepare(s,n);s->words[3]=(uint16_t)(((n/2u)%8u)*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/2u)%8u));s->words[5]=n%2u ? 02370 : 02354;switch((n/16u)%4u){case 0:s->words[01023+relative]=0;break;case 1:s->words[01023+relative]=077777;break;default:break;}}
