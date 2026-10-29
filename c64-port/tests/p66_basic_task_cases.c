/* SPDX-License-Identifier: MIT
 * Independent deterministic physical input images; no Executive kernel calls. */
#include "p66_basic_task_cases.h"
void p66_basic_task_prepare(apollo_overlay_state *m,uint16_t n){uint16_t i,freecore=(uint16_t)(n%9u),freevac=(uint16_t)((n/9u)%6u),eb=(uint16_t)((n/54u)%8u);uint32_t seed=n+10943u;
 for(i=0;i<APOLLO_SHARED_WORDS;++i){seed=seed*1664525u+1013904223u;m->words[i]=(uint16_t)((seed>>16)&077777u);}
 m->words[3]=(uint16_t)(eb*0400u);m->words[4]=064000;m->words[6]=(uint16_t)(064000u|eb);m->words[5]=03263;m->words[7]=0;m->words[067]=freecore==0 ? 077777 : 0;m->words[0120]=0110;m->words[0121]=0;
 for(i=0;i<8;++i)m->words[0154+i*12u+11u]=i==freecore ? 077777 : (uint16_t)(035110u+i);
 for(i=0;i<5;++i)m->words[0400+i*054u]=i==freevac ? (uint16_t)(0400+i*054u) : 0;
}
