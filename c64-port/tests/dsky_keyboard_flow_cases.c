/* SPDX-License-Identifier: MIT
 * Native-independent canonical KEYCOM input, actual idle core0 admission. */
#include "dsky_keyboard_flow_cases.h"
#include "dsky_nv_key_cases.h"
void dsky_keyboard_flow_prepare(apollo_overlay_state *s,uint16_t n){uint16_t i;
 dsky_nv_key_prepare(s,n);s->words[0]=n%2u ? 037 : 021;s->words[4]=010000;s->words[6]=(uint16_t)(010000u|(s->words[3]>>8));s->words[5]=03303;s->words[067]=077777;s->words[0120]=0110;s->words[0121]=0;
 for(i=0;i<8;++i)s->words[0154+i*12u+11u]=077777;
 s->words[01042]=(uint16_t[]){0,077777,1,040000}[n%4u];
}
uint16_t dsky_keyboard_flow_channel11(uint16_t n){return (uint16_t)(((uint32_t)n*151u+04100u)&077777u);}
