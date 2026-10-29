/* SPDX-License-Identifier: MIT */
#include "cm_restart_select_cases.h"
#include "agc_word.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_rsselect_input(apollo_overlay_state *s,uint16_t n){
 unsigned i,g=n%6u;uint32_t seed=0x73520u+n;agc_word phase;
 static const agc_word vars[]={00402,00401,00403,01003,01403};
 static const agc_word odd[6][32]={
 {3,5,7,011},{3,5,7,011,013},{3,7,011,013,015},
 {3,5,7,011,013,015,017,021,023,025,027,031,033,035,037,041,043,045,047,051,053,055,057,061,063,065,067,071,073,075,077},
 {3,5,7,011,013,015,017,021,023,025,027,033,037,041},{3,5,011}};
 static const uint8_t counts[6]={4,5,5,31,14,3};
 static const agc_word size[6]={063,063,063,063,0242,0336};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=03520;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);s->words[0161]=(agc_word)g;
 phase=n<1024u?vars[(n/6u)%5u]:n<2048u?odd[g][(n/6u)%counts[g]]:n<3072u?(g==3u?(agc_word)(2u+2u*((n/6u)%3u)):g==4u?(agc_word)(2u+2u*((n/6u)%2u)):2u):n<4096u?1u:n<5120u?2u:odd[g][(n/6u)%counts[g]];
 s->words[0154]=phase;
 if(n>=4096u){s->words[0155]=(agc_word)(g*2u);s->words[0157]=04675;s->words[0156]=(agc_word)(size[g]+3u*phase);s->words[5]=n<5120u?03755:03533;}s->words[01436+g*2u]=(agc_word)(01200u+(n&0377u));s->words[01437+g*2u]=02003;
 s->words[01054+g*2u]=phase==00401u?(agc_word)(n%300u):phase==00403u?agc_negate(01110u):(n&1u)?010000u:067777u;
 s->words[01110]=0;s->words[01111]=(agc_word)(n%12000u);s->words[01135]=0;s->words[01136]=0;s->words[024]=0;s->words[025]=(agc_word)(n%10000u);
}
