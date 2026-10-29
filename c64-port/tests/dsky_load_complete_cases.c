/* SPDX-License-Identifier: MIT */
#include "dsky_load_complete_cases.h"
#include "p66_bias_cases.h"
void dsky_load_complete_input(apollo_overlay_state *s,dsky_lamp_io *io,uint16_t n){static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},words[]={0,077777,1,077776,037776,037777,040000,040001},cadr[]={0,077777,02546,077776},lists[]={0,077777,02546,03377};uint16_t cm=(uint16_t)(n>=4096u),j=(uint16_t)(n%4096u),cores=cm ? 7u : 8u,i,slot=(uint16_t)(1u+(j/16u)%(cores-1u)),other=(uint16_t)(slot%(cores-1u)+1u);p66_bias_prepare(s,n);s->words[3]=(uint16_t)((j/8u)%8u*0400u);s->words[4]=062000;s->words[6]=(uint16_t)(062000u|(s->words[3]>>8));s->words[5]=02775;s->words[0]=raw[(j/8u)%8u];s->words[1]=raw[(j/64u)%8u];s->words[2]=raw[(j/512u)%8u];s->words[01042]=cadr[j%4u];s->words[01043]=lists[(j/4u)%4u];s->words[01014]=words[(j/128u)%8u];s->words[01000]=words[(j/256u)%8u];s->words[0167]=030123;s->words[067]=(uint16_t)((j/32u)%2u ? 077777u : (j/64u)%cores*12u);
 for(i=1;i<cores;++i){s->words[0167u+12u*i]=i%3u==0u ? 077777u : (uint16_t)(010000u+i);s->words[0164u+12u*i]=03300;s->words[0165u+12u*i]=(uint16_t)((j/2u)&0167u);}
 if((j/16u)%3u){s->words[0167u+12u*slot]=agc_negate((uint16_t)(020000u+slot));s->words[0164u+12u*slot]=s->words[01042];}
 if((j/48u)%3u==0u){s->words[0167u+12u*other]=agc_negate((uint16_t)(025000u+other));s->words[0164u+12u*other]=s->words[01043];}
 s->words[064]=(uint16_t)((j/8u)%cores*12u);if((j/1024u)%2u)s->words[064]=077776;
 if(s->words[01042]==0u&&s->words[01043]==0u){s->words[0167]=0;s->words[067]=1;s->words[0203]=0;}
 io->control.superbank=(uint16_t)(0100u+(j/256u)%4u*020u);io->control.interrupt_enabled=(uint16_t)((j/128u)%2u);io->channel11=(uint16_t)((j*317u)&077777u);io->channel12=(uint16_t)((j*171u)&077777u);io->channel13=(uint16_t)((j*173u)&077777u);io->restart_light=(uint16_t)((j/32u)%2u);io->wrote_channels=0;}
