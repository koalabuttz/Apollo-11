/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_task_transfer_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_task_transfer_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i;uint32_t seed=0x03423055u+n;cm_waitlist w;
 static const exec_address task[4]={{03207,032066},{03260,050067},{02612,050066},{05306,04063}};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[2]&=07777;s->words[012]&=07777;s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 s->words[6]=n<512u?02003:(agc_word)((((n>>3)&31u)<<10)|(n&7u));s->words[3]=(agc_word)((s->words[6]&7u)<<8);s->words[4]=(agc_word)(s->words[6]&076000u);
 s->words[5]=n<512u?03423:05324;s->words[016]=(agc_word)((((n>>4)&31u)<<10)|(n&7u)|((n&7u)<<4));
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)((n>>3)&1u);
 cm_wl_init(&w);w.time3=(agc_word)(n%128u);w.ruptagn=n&1u?077777:1;
 for(i=0;i<8u;++i)w.interval[i]=agc_negate((agc_word)(1u+((n*17u+i*31u)%8191u)));
 w.task[0]=task[n%4u];cm_native_commit_waitlist(s,io,&w);
}
