/* SPDX-License-Identifier: MIT */
#include "dsky_octal_display_cases.h"
#include "p66_bias_cases.h"
void dsky_octal_display_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},edges[]={0,077777,037777,040000,1,077776};uint16_t i,count,len=(uint16_t)(n%2u ? 2u : 5u);p66_bias_prepare(s,n);count=(uint16_t)(n%2u ? 1u+(n/2u)%21u : 4u+(n/2u)%18u);s->words[0777]=count;s->words[3]=(uint16_t)((n/16u)%8u*0400u);s->words[4]=062000;s->words[6]=(uint16_t)(062000u|(s->words[3]>>8));s->words[5]=n%2u ? 03416 : 03363;s->words[0]=agc_extend(n%16u<6u ? edges[n%16u] : (uint16_t)((n*317u+02534u)&077777u));s->words[1]=raw[(n/64u)%8u];s->words[2]=(uint16_t)(07100u+n%0100u);s->words[01016]=edges[(n/8u)%6u];for(i=0;i<13;++i)if(s->words[01023u+i]==0)s->words[01023u+i]=073777;
 if((n/32u)%3u){uint16_t pos=(uint16_t)((n/64u)%len);s->words[01023u+(count-pos)/2u]=0;}s->words[01023u+12u]=0;
 io->superbank=(uint16_t)(0100u+((n/128u)%4u)*020u);io->interrupt_enabled=(uint16_t)((n/256u)%2u);}
