/* SPDX-License-Identifier: MIT
 * Native-independent canonical KEYCOM input, actual idle core0 admission. */
#include "dsky_keyboard_number_flow_cases.h"
#include "dsky_nv_key_cases.h"
void dsky_keyboard_number_flow_prepare(apollo_overlay_state *s,uint16_t n){uint16_t i;
 dsky_nv_key_prepare(s,n);s->words[0]=2;s->words[1]=(uint16_t[]){0,077777,037777,040000,0100000,0177777,0140000,1}[(n/2u)%8u];s->words[2]=(uint16_t[]){0,077777,037777,040000,0100000,0177777,0140000,1}[(n/16u)%8u];s->words[4]=02000;s->words[6]=(uint16_t)(02000u|(s->words[3]>>8));s->words[5]=03214;s->words[067]=077777;s->words[0120]=0110;s->words[0121]=0;
 for(i=0;i<8;++i)s->words[0154+i*12u+11u]=077777;
 s->words[0777]=(uint16_t)(n%19u);if(s->words[0777]>=15u)++s->words[0777];
 s->words[01000]=(uint16_t)(1u+(n/19u)%2u);
 for(i=01001;i<=01005;++i)s->words[i]=(uint16_t)((n*41u+i)%(i<01003?100u:10000u));
 for(i=0;i<11;++i)if(s->words[01023+i]==0)s->words[01023+i]=073777;
 s->words[01042]=(uint16_t[]){0,077777,1,040000}[n%4u];
}
uint16_t dsky_keyboard_number_flow_channel11(uint16_t n){return (uint16_t)(((uint32_t)n*151u+04100u)&077777u);}
uint16_t dsky_keyboard_number_flow_key(uint16_t n){return n%10u ? (uint16_t)(n%10u) : 020;}
