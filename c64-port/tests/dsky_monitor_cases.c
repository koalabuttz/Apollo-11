/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_cases.h"
void dsky_monitor_input(apollo_overlay_state *s,dsky_charin_io *io,uint16_t n,enum dsky_nv_mission *mission,enum dsky_monitor_stage *stage)
{static const agc_word pc[11]={03230,03262,03263,03271,03275,03307,03313,03314,03321,03333,03351},edge[8]={0,077777,1,077776,037777,040000,037776,040001};uint16_t i,t=(uint16_t)(n%32768u);uint32_t seed=0x3230u+n%16u;
 *mission=n<32768u?DSKY_NV_LM:DSKY_NV_CM;*stage=(enum dsky_monitor_stage)(t%11u);
 for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}
 s->words[0]=(agc_register)((seed*17u+n*97u)&0177777u);s->words[1]=(agc_register)((seed*19u+n*113u)&0177777u);s->words[2]=(agc_word)(0100u+t%07600u);s->words[3]=(agc_word)(((t>>4)&7u)*0400u);s->words[4]=062000;s->words[5]=pc[*stage];s->words[6]=(agc_word)(062000u|(s->words[3]>>8));s->words[7]=0;
 io->control.superbank=(agc_word)(0100u+(t&3u)*020u);io->control.interrupt_enabled=(agc_word)((t>>2)&1u);io->channel11=(agc_word)((t*271u)&AGC_MASK);io->wrote_channel11=(uint8_t)((t>>3)&1u);
 s->words[01020]=edge[(t/11u)&7u];s->words[01021]=edge[(t/88u)&7u];s->words[01012]=edge[(t/704u)&7u];s->words[01042]=edge[(t/11u)&7u];s->words[01001]=(agc_word)((t*13u)&AGC_MASK);s->words[01002]=edge[(t/88u)&7u];s->words[0136]=(t&3u)?edge[(t/44u)&7u]:(*mission==DSKY_NV_LM?05155u:05217u);
}
