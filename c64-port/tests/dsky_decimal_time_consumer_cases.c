/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_time_consumer_cases.h"
void dsky_decimal_time_consumer_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,enum dsky_nv_mission *mission)
{
 static const agc_word routines[3]={8,9,12};static const agc_word corners[8]={0,077777,1,077776,037777,040000,037776,040001};uint32_t seed=0x2537u+n%16u;uint16_t i,t=(uint16_t)(n%2048u),count=(uint16_t)(t%3u);agc_word routine=routines[(t/3u)%3u],factor=(agc_word)((t/9u)%20u);
 *mission=n<2048u?DSKY_NV_LM:DSKY_NV_CM;
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(02000u+t%02000u);s->words[3]=(agc_word)(((t>>3)&7u)*0400u);s->words[4]=062000;s->words[5]=02523;s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;io->superbank=(agc_word)(0100u+(t&3u)*020u);io->interrupt_enabled=(agc_word)((t>>2)&1u);
 s->words[0140]=(agc_word)(1u+((t>>4)&1u));s->words[0145]=01000;s->words[0146]=(agc_word)(count<<10);s->words[0147]=(agc_word)((count<<10)|(routine<<5)|factor);s->words[0153]=0;
 if(s->words[0140]==2u){s->words[0147]=(agc_word)((factor<<10)|(factor<<5)|factor);s->words[0153]=(agc_word)((routine<<10)|(routine<<5)|routine);}
 for(i=0;i<3u;++i){s->words[0150u+i]=(agc_word)(01100u+2u*i);s->words[01000u+i]=routine==12u?(agc_word)((t*13u+i*31u)%100u):corners[(t+i)&7u];s->words[01100u+2u*i]=routine==12u?(agc_word)((t*17u+i*7u)%100u):corners[(t+i)&7u];s->words[01101u+2u*i]=routine==12u?(agc_word)((t*19u+i*11u)%100u):(agc_word)((t*149u+i*23u)&AGC_MASK);}
 for(i=0;i<11u;++i)s->words[01023u+i]=(agc_word)((t&1u)?073777:037777);

}

uint8_t dsky_decimal_time_consumer_stage(enum dsky_nv_mission mission,agc_word pc)
{static const agc_word lm[19]={03230,03234,03240,03242,03253,03255,03264,03266,03303,03343,03347,03351,03361,03371,02732,02733,02734,02736,02745},cm[19]={03143,03147,03153,03155,03166,03170,03177,03201,03216,03256,03262,03264,03274,03304,02636,02637,02640,02642,02651};unsigned i;if(pc==02523u)return 0;if(pc==02537u)return 1;if(pc==02570u)return 21;for(i=0;i<19u;++i)if(pc==(mission==DSKY_NV_LM?lm[i]:cm[i]))return (uint8_t)(i+2u);return 255;}
