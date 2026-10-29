/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_fixed_compat_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_fixed_compat_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){uint16_t address=(uint16_t)(n%40960u),bank=(uint16_t)(address/1024u),offset=(uint16_t)(address%1024u);uint8_t selfcheck=(uint8_t)(n>=40960u);unsigned i;agc_word cadr;uint32_t seed=0x04715055u+(n%2u);
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 memset(io,0,sizeof *io);io->superbank=(agc_word)(0100u|((n%4u)*020u));io->interrupt_enabled=(agc_word)(n&1u);
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);s->words[2]=0705;
 s->words[3]=(agc_word)((n%8u)*0400u);s->words[4]=selfcheck?066000:062000;s->words[6]=(agc_word)(s->words[4]|(n%8u));s->words[5]=selfcheck?03535:03353;
 cadr=(agc_word)(((bank>=32u?bank-8u:bank)*02000u)|offset);
 s->words[01017]=cadr;s->words[01047]=bank>=32u?0100:0;
 s->words[01374]=(agc_word)((cadr&076000u)|(bank>=32u?0100u:0u));s->words[01373]=offset;
}
