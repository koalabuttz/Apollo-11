/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_cases.h"
#include "p66_bias_cases.h"
void dsky_decimal_prepare(apollo_overlay_state *s,uint16_t n){
 static const uint16_t corners[][2]={{0,0},{077777,077777},{037777,037777},{040000,040000},{0,1},{077777,077776},{016252,016252},{061525,061525}};
 uint16_t i;p66_bias_prepare(s,n);s->words[3]=(uint16_t)(((n/3u)%8u)*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/3u)%8u));s->words[2]=(uint16_t)(04000+n%03777u);s->words[5]=n%2u ? 03226 : 03262;s->words[0777]=(uint16_t)(14u-n%3u*5u);
 if(n<96){s->words[0154]=corners[(n/3u)%8u][0];s->words[0155]=corners[(n/3u)%8u][1];}else s->words[0155]=(uint16_t)((s->words[0155]&037777u)|(s->words[0154]&040000u));
 for(i=0;i<11;++i)s->words[01023+i]=(uint16_t)(s->words[01023+i]|04000u);
}
