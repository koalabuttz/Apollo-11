/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_clocktask_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_clocktask_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i;uint32_t seed=0x0326055u+n;
 static const agc_word nv[8]={0,077777,01525,01400,06000,077377,077000,037777};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=03400;s->words[4]=050000;s->words[6]=050007;s->words[5]=n<512u?03260:n<1024u?03301:03315;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);s->words[01145]=nv[n%8u];
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)((n>>3)&1u);
}
