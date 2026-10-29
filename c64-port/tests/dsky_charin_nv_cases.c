/* SPDX-License-Identifier: MIT */
#include "dsky_charin_nv_cases.h"
#include "dsky_nv_key_cases.h"
uint16_t dsky_charin_nv_channel11(uint16_t n){return (uint16_t)((n*151u+04100u)&077777u);}
void dsky_charin_nv_prepare(apollo_overlay_state *s,uint16_t n){static const uint16_t cadrs[]={0,077777,1,077776};dsky_nv_key_prepare(s,n);s->words[5]=02077;s->words[0154]=n%2u ? 037 : 021;s->words[01042]=cadrs[(n/16u)%4u];}
