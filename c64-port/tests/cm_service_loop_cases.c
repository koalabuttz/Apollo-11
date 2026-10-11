/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_service_loop_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_service_loop_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i,cohort=n/64u;uint32_t seed=0x3432055u+n;cm_waitlist w;cm_executive e;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)((n>>3)&1u);
 cm_exec_init(&e);e.core[0][CM_EXEC_PRIORITY]=010110;cm_native_commit_executive(s,io,&e);
 s->words[0]=agc_extend((agc_word)((n*53u)&077777u));s->words[1]=agc_extend((agc_word)((n*31u)&077777u));s->words[2]=(agc_word)(04000u+n);s->words[012]=(agc_word)(02000u+n);
 s->words[6]=02003;s->words[3]=01400;s->words[4]=02000;s->words[5]=03423;s->words[016]=(agc_word)((((n>>3)&31u)<<10)|(n&7u)|((n&7u)<<4));
 cm_wl_init(&w);w.time3=(agc_word)(n%32u);w.interval[0]=agc_negate(100);
 w.task[0]=cohort==0u?(exec_address){03207,032066}:cohort<3u?(exec_address){03260,050067}:(exec_address){05306,04063};
 cm_native_commit_waitlist(s,io,&w);
 if(cohort==1u)s->words[0103]&=075777u;else s->words[0103]|=02000u;
 s->words[01145]=0;s->words[0076]=cohort<4u?1:077777;s->words[01322]=cohort==4u?0:1;
}
