/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_pinball_cases.h"
void cm_pinball_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){
 uint16_t i,e=(uint16_t)(n%3u);uint32_t seed=(uint32_t)n+1971u;
 static const uint16_t raw[8]={0,0177777,037777,040000,0100000,0140000,077777,0123456};
 static const uint16_t nout[8]={0,077777,037777,037776,040000,040001,077776,1};
 for(i=0;i<2048u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);s->words[i]=(uint16_t)((seed>>9)&077777u);}
 memset(io,0,sizeof *io);io->superbank=0100;io->interrupt_enabled=(uint16_t)(n%2u);
 s->words[0]=e==1u?(uint16_t)(n%11u):raw[(n/3u)%8u];s->words[1]=raw[(n/24u)%8u];s->words[2]=(uint16_t)(04000u+n%03777u);
 s->words[3]=(uint16_t)(((n/3u)%8u)*0400u);s->words[4]=060000;s->words[6]=(uint16_t)(060000u|((n/3u)%8u));s->words[7]=0;
 s->words[5]=(uint16_t)(e==0u?03227u:e==1u?03311u:02502u);
 s->words[0143]=(uint16_t)((n/3u)%22u+((n/66u)%2u?020000u:0u));s->words[0124]=e==0u?(uint16_t)((n/3u)%32u):(uint16_t)((n/6u)%2u*02000u);s->words[0777]=(uint16_t)((n/3u)%22u);
 s->words[01016]=nout[(n/3u)%8u];
 for(i=0;i<11u;++i){s->words[01023u+i]|=04000u;
  if((n/24u)%4u==0u)s->words[01023u+i]=04000;
  if((n/24u)%4u==1u)s->words[01023u+i]=077777;
  if(e==2u&&(n/24u)%4u==2u)s->words[01023u+i]=0;
 }
}
