/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lm_svct3_cases.h"
#include "agc_word.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void lm_svct3_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){unsigned i;uint32_t seed=0x05240099u+n;
 static const agc_word corner[8]={0,077777,1,077776,037777,040000,01000,077000};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=01400;s->words[4]=004000;s->words[6]=004003;s->words[5]=n<512u?05240:n<1024u?05255:05260;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);s->words[0076]=corner[n%8u];s->words[01304]=corner[(n/8u)%8u];
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)((n>>3)&1u);
}
