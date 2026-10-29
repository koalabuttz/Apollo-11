/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_task_interrupt_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_task_interrupt_input(apollo_overlay_state *s,cm_native_io *io,cm_task_interrupt_state *irq,uint16_t n){unsigned i;uint32_t seed=0x4014055u+n;
 static const agc_register corner[8]={0,0177777,1,0177776,037777,0140000,040000,0137777};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=(agc_word)((n&7u)<<8);s->words[4]=(agc_word)(((n>>3)&31u)<<10);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=n<512u?06030:05341;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=corner[n%8u];s->words[2]&=07777;s->words[012]&=07777;
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=1;irq->in_isr=(uint8_t)(n>=512u);irq->pending_danzig=0;
 if(n>=512u){s->words[0]=corner[(n/8u)%8u];s->words[015]=06031;s->words[017]=030165;}
}
