/* SPDX-License-Identifier: MIT */
#include "cm_rectify_cases.h"
#include "agc_word.h"
uint8_t cm_rectify_first(uint16_t n){return (uint8_t)(n>=1536u?5u:0u);}
void cm_rectify_input(apollo_overlay_state *m,uint16_t n){
 static const uint16_t bases[5]={0401,0455,0531,0605,0661};
 static const agc_word corners[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 uint32_t seed=UINT32_C(0x60373421)+(uint32_t)n*UINT32_C(0x9e3779b9);unsigned i;uint16_t base=bases[n%5u];
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(uint16_t)(seed&077777u);}
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=01234;m->words[3]=01400;m->words[4]=022000;m->words[5]=06037;m->words[6]=022003;m->words[7]=0;
 m->words[067]=0;m->words[020]=0;m->words[023]=0;m->words[0115]=0;
 m->words[0120]=base;m->words[0164]=(uint16_t)(cm_rectify_first(n)?03435u:03421u);m->words[0165]=022003;m->words[0166]=base;m->words[0167]=base;
 m->words[0165]=(uint16_t)(022000u|((n/8u)%8u));m->words[3]=(uint16_t)((m->words[0165]&7u)<<8);m->words[6]=m->words[0165];
 m->words[base+052]=(uint16_t)(04000u+((n/64u)%14u)*04000u+01700u+(n%32u));m->words[02150]=(uint16_t)(n&1u?2u:0u);m->words[0163]=cm_rectify_first(n)?077776:corners[(n/2u)%16u];
 if(n<256u||n>=1792u){for(i=0;i<6u;++i){m->words[01520+i]=corners[(n+i)%16u];m->words[01526+i]=corners[(n/4u+3u*i)%16u];m->words[01534+i]=corners[(n/16u+5u*i)%16u];m->words[01542+i]=corners[(n/32u+7u*i)%16u];}}
}
