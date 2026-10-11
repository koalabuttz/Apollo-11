/* SPDX-License-Identifier: MIT */
#include "dsky_response_recall_cases.h"
#include "p66_bias_cases.h"
void dsky_response_recall_input(apollo_overlay_state *s,dsky_lamp_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},words[]={0,077777,1,077776,037776,037777,040000,040001},cadr[]={0,077777,02546,077776},lists[]={0,077777,02546,03377},lm[]={03547,03552,03563,03575,03601,03577},cmaddr[]={03454,03457,03470,03502,03506,03504};uint16_t cm=(uint16_t)(n>=4096u),j=(uint16_t)(n%4096u),cores=cm ? 7u : 8u,e=(uint16_t)(j%6u),i,slot=(uint16_t)(1u+(j/24u)%(cores-1u)),index;p66_bias_prepare(s,n);s->words[3]=(uint16_t)((j/8u)%8u*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|(s->words[3]>>8));s->words[5]=cm ? cmaddr[e] : lm[e];s->words[0]=raw[(j/6u)%8u];s->words[1]=raw[(j/48u)%8u];s->words[2]=raw[(j/384u)%8u];s->words[01042]=cadr[(j/6u)%4u];s->words[01043]=lists[(j/24u)%4u];s->words[01014]=words[(j/96u)%8u];s->words[0167]=030123;s->words[067]=(uint16_t)((j/12u)%2u ? 077777u : (j/48u)%cores*12u);
 for(i=1;i<cores;++i){s->words[0167u+12u*i]=i%3u==0u ? 077777u : (uint16_t)(010000u+i);s->words[0164u+12u*i]=03300;s->words[0165u+12u*i]=(uint16_t)((j/2u)&0167u);}
 if((j/6u)%3u){s->words[0167u+12u*slot]=agc_negate((uint16_t)(020000u+slot));s->words[0164u+12u*slot]=s->words[01042];}
 if((j/96u)%3u==0u){uint16_t other=(uint16_t)(slot%(cores-1u)+1u);s->words[0167u+12u*other]=agc_negate((uint16_t)(025000u+other));s->words[0164u+12u*other]=s->words[01043];}
 s->words[064]=(uint16_t)((j/6u)%cores*12u);if((j/768u)%2u)s->words[064]=077776;
 index=(uint16_t)(agc_add_word(0164u,s->words[064])&07777u);if(e>=2u&&e!=3u)s->words[index]=words[(j/12u)%8u];
 if((e==0u&&s->words[01042]==0u)||(e==3u&&s->words[01043]==0u)){s->words[0167]=0;s->words[067]=1;s->words[0203]=0;}
 io->control.superbank=(uint16_t)(0100u+(j/256u)%4u*020u);io->control.interrupt_enabled=(uint16_t)((j/128u)%2u);io->channel11=(uint16_t)((j*317u)&077777u);io->channel12=(uint16_t)((j*171u)&077777u);io->channel13=(uint16_t)((j*173u)&077777u);io->restart_light=(uint16_t)((j/32u)%2u);io->wrote_channels=0;}
