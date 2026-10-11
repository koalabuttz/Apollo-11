/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_keyboard_irq_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
uint16_t cm_keyboard_irq_channel15(uint16_t n){return (uint16_t)((n*151u+04100u)&077777u);}
void cm_keyboard_irq_input(apollo_overlay_state *s,cm_native_io *io,uint16_t *isr,uint16_t n){uint16_t i,k=(uint16_t)(n%4096u),stage=(uint16_t)(n/4096u);uint32_t seed=0x4b455905u+n;
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1};
 for(i=0;i<2048;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&077777u);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=(uint16_t)((k%8u)<<8);s->words[4]=016000;s->words[6]=(uint16_t)(016000u|(k%8u));s->words[5]=stage==0 ? 03702 : stage==1 ? 03673 : 04024;
 s->words[0]=stage==0 ? (uint16_t)(k%32u) : raw[(k/64u)%8u];s->words[1]=raw[(k/16u)%8u];s->words[2]=raw[(k/8u)%8u];
 memset(io,0,sizeof *io);io->superbank=(uint16_t)((k/8u)%8u*020u);io->interrupt_enabled=(uint16_t)((k/512u)%2u);*isr=stage==2 ? 1 : 0;
 s->words[067]=k%7u==0 ? 077777 : 0;
 for(i=0;i<7;++i)s->words[0154+i*12u+11u]=i<k%7u ? 010110 : 077777;
 for(i=0;i<5;++i)s->words[0400+i*44u]=i<k%5u ? 0 : (uint16_t)(0400u+i*44u);
 if(stage==2){s->words[4]=(uint16_t)(((k/8u)%32u)<<10);s->words[6]=(uint16_t)(s->words[4]|(k%8u));s->words[015]=(uint16_t)(1u+k%07777u);io->interrupt_enabled=0;}
}
enum exec_status cm_keyboard_irq_run(apollo_overlay_state *s,cm_native_io *io,uint16_t *isr,uint16_t n){return n<4096u ? cm_keyboard_keycom(s,io) : n<8192u ? cm_keyboard_keyrupt(s,io,cm_keyboard_irq_channel15(n)) : cm_keyboard_interrupt(s,io,isr,cm_keyboard_irq_channel15(n));}
