/* SPDX-License-Identifier: MIT */
#include "dsky_load_control_cases.h"
#include "p66_bias_cases.h"
void dsky_load_control_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},cadrs[]={0,077777,02546,077776},lm[]={03442,03450,03457,03452,03547},cm[]={03347,03355,03364,03357,03454},loads[]={0,1,077776,077777,037777,040000};uint16_t i,cores=n<4096u ? 8u : 7u,entry=(uint16_t)(n%5u),type=(uint16_t)((n/8u)%4u);
 p66_bias_prepare(s,n);s->words[3]=(uint16_t)((n/16u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|(s->words[3]>>8));s->words[5]=n<4096u ? lm[entry] : cm[entry];s->words[0]=raw[(n/32u)%8u];s->words[1]=raw[(n/64u)%8u];s->words[01042]=cadrs[type];s->words[01043]=cadrs[(n/32u)%4u];s->words[01014]=loads[(n/128u)%6u];s->words[01001]=(uint16_t)(n%101u);if(s->words[01001]==100u)s->words[01001]=077777;s->words[01002]=(uint16_t)((n/3u)%101u);if(s->words[01002]==100u)s->words[01002]=077777;
 s->words[0167]=030123;s->words[0163]=raw[(n/256u)%4u];s->words[067]=n%3u ? 0 : 077777;for(i=1;i<cores;++i)s->words[0167u+12u*i]=077777;
 if(n%3u){i=(uint16_t)(1u+(n/32u)%(cores-1u));s->words[0167u+12u*i]=agc_negate((uint16_t)(020000u+i));s->words[0164u+12u*i]=n%3u==1u ? s->words[01042] : s->words[01043];s->words[0165u+12u*i]=(uint16_t)((n/2u)&0167u);}
 if((entry==4u||s->words[01043]==0)&&s->words[01042]==0){s->words[0167]=0;s->words[0167u+12u]=0;s->words[067]=037777;}
 if(entry==4u&&s->words[01014]==0&&s->words[01042]!=0){s->words[01043]=077775;s->words[0167u+24u]=077776;s->words[0164u+24u]=077775;s->words[0165u+24u]=077777;}
 io->control.superbank=(uint16_t)(0100u+((n/128u)%4u)*020u);io->control.interrupt_enabled=(uint16_t)((n/256u)%2u);io->channel11=(uint16_t)((n*151u+04100u)&077777u);io->wrote_channel11=0;
}
