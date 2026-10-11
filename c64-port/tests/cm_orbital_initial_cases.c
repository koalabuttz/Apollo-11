/* SPDX-License-Identifier: MIT */
#include "cm_orbital_initial_cases.h"
#include "agc_word.h"
void cm_orbital_initial_input(apollo_overlay_state *m,agc_word *irq,uint16_t n){
 static const uint16_t bases[5]={0401,0455,0531,0605,0661};
 static const agc_word corners[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 uint32_t seed=UINT32_C(0x33053317)+(uint32_t)n*UINT32_C(0x9e3779b9);unsigned i;uint16_t base=bases[n%5u];
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(uint16_t)(seed&077777u);}
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);m->words[2]=01234;m->words[3]=(uint16_t)(((n/8u)%8u)<<8);m->words[4]=022000;m->words[5]=06037;m->words[6]=(uint16_t)(022000u|(m->words[3]>>8));m->words[7]=0;
 m->words[067]=0;m->words[020]=0;m->words[023]=0;m->words[0115]=0;m->words[0120]=base;m->words[0164]=03305;m->words[0165]=m->words[6];m->words[0166]=base;m->words[0167]=base;m->words[0163]=corners[(n/2u)%16u];
 if(n<256u){for(i=0;i<6u;++i){m->words[01111+i]=corners[(n+i)%16u];m->words[02151+i]=corners[(n/4u+3u*i)%16u];}m->words[02211]=corners[(n/16u)%16u];m->words[02212]=corners[(n/32u)%16u];}
 *irq=(agc_word)(n&1u);
}
