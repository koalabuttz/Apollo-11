/* SPDX-License-Identifier: MIT */
#include "dsky_charin_number_cases.h"
#include "dsky_nv_key_cases.h"
uint16_t dsky_charin_number_channel11(uint16_t n){return (uint16_t)((n*151u+04100u)&077777u);}
void dsky_charin_number_prepare(apollo_overlay_state *s,uint16_t n){
 static const uint16_t cadrs[]={0,077777,1,077776},clpass[]={0,1,037777,040000,077776,077777},tp[]={0,077777,1,077776,037777,040000};uint16_t k;
 dsky_nv_key_prepare(s,n);s->words[5]=02077;s->words[0154]=n%10u ? (uint16_t)(n%10u) : 020;
 s->words[01042]=cadrs[(n/32u)%4u];s->words[0777]=(uint16_t)(16u+(n/10u)%4u);s->words[01000]=(uint16_t)(1u+(n/40u)%2u);s->words[01015]=clpass[(n/80u)%6u];
 s->words[01001]=(uint16_t)((n/4u)%100u);s->words[01002]=(uint16_t)((n/7u)%100u);s->words[0156]=tp[(n/480u)%6u];
 for(k=0;k<11;++k)if(s->words[01023+k]==0)s->words[01023+k]=073777;
 if(n>=4096u){uint16_t number=(uint16_t)(n-4096u),count=(uint16_t)(number%19u),reg;
  if(count>=15u){++count;}s->words[0777]=count;s->words[01000]=(uint16_t)((number/19u)%3u);
  if((s->words[01000]&3u)==0 && n%10u>=8u)s->words[0154]=7;
  for(reg=01001;reg<=01005;++reg)s->words[reg]=(uint16_t)(((uint32_t)number*71u+reg)%((reg<01003)?100u:10000u));
  if((s->words[01000]&3u)==0)for(reg=01001;reg<=01005;++reg)s->words[reg]=(uint16_t)(((uint32_t)number*151u+reg)&077777u);
  if(n>=7680u)s->words[0777]=(uint16_t)(040000u+((uint32_t)number*73u)%040000u);
 }
 s->words[1]=(uint16_t)((n%4u)==0 ? 0100000 : (n%4u)==1 ? 040000 : (n%4u)==2 ? 0177777 : 0);
}
