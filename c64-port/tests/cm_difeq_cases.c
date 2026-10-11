/* SPDX-License-Identifier: MIT */
#include "cm_difeq_cases.h"
#include "agc_word.h"
uint8_t cm_difeq_stage(uint16_t n){return (uint8_t)(n<1536u?n/512u:(n-1536u)/128u);}
void cm_difeq_input(apollo_overlay_state *m,uint16_t n){
 static const uint16_t bases[5]={0401,0455,0531,0605,0661};
 static const agc_word corners[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 static const uint16_t entry[3]={03446,03452,03463},index[3]={0,077763,077747};
 uint32_t seed=UINT32_C(0x34463626)+(uint32_t)n*UINT32_C(0x9e3779b9);unsigned i,j;uint16_t base=bases[n%5u];uint8_t stage=cm_difeq_stage(n);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(uint16_t)(seed&077777u);}
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=01234;m->words[3]=01400;m->words[4]=022000;m->words[5]=06037;m->words[6]=022003;m->words[7]=0;
 m->words[067]=0;m->words[020]=0;m->words[023]=0;m->words[0115]=0;
 m->words[0120]=base;m->words[0164]=entry[stage];m->words[0165]=(uint16_t)(022000u|((n/8u)%8u));m->words[0166]=base;m->words[0167]=base;
 m->words[3]=(uint16_t)((m->words[0165]&7u)<<8);m->words[6]=m->words[0165];m->words[base+046]=index[stage];m->words[01500]=index[stage];m->words[0163]=corners[(n/2u)%16u];
 if(n>=1536u){m->words[base+046]=n%128u<64u?corners[n%16u]:m->words[base+046];m->words[01500]=m->words[base+046];}
 if(n%512u<256u){static const uint16_t vector[6]={02165,02173,02201,01111,01117,02151};for(j=0;j<6u;++j)for(i=0;i<6u;++i)m->words[vector[j]+i]=corners[(n/(j+1u)+(2u*j+1u)*i)%16u];m->words[02211]=corners[(n/16u)%16u];m->words[02212]=corners[(n/32u)%16u];m->words[02313]=corners[(n/64u)%16u];m->words[02314]=corners[(n/128u)%16u];}
}
