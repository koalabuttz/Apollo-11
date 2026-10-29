/* SPDX-License-Identifier: MIT */
#include "lm_finish_cases.h"
#include "lm_sleep_cases.h"
void lm_finish_input(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t n){uint8_t i;static const agc_word base[8]={0,0110,0401,0455,0531,0605,0661,1},q[8]={0,0177777,1,0177776,040000,0137777,077777,0100000};lm_sleep_input(s,io,channel,(uint16_t)(n%8192u));s->words[5]=05155;s->words[0167]=(agc_word)(022000u+base[(n/128u)%8u]);s->words[2]=q[n%8u];for(i=1;i<8u;++i)if(n>=8192u&&i%2u==n%2u)s->words[0164u+12u*i]=agc_negate((agc_word)(03360u+i));}
