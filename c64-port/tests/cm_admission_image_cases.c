/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_admission_image_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
void cm_admission_image_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned i;uint32_t seed=0x5147055u+n;cm_executive e;
 static const exec_address job[5]={{03332,050066},{03325,050066},{02427,034067},{03542,014063},{03207,032066}};
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>11)&AGC_MASK);
 s->words[7]=0;s->words[035]=0;s->words[036]=0;s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 s->words[3]=(agc_word)((n&7u)<<8);s->words[4]=(agc_word)(((n>>3)&31u)<<10);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=n<512u?05147:05134;
 s->words[0]=(agc_word)(((n%31u)+1u)<<9);s->words[1]=agc_extend(s->words[1]);s->words[2]=0705;
 memset(io,0,sizeof *io);io->superbank=(agc_word)((n&7u)<<4);io->interrupt_enabled=(agc_word)((n>>3)&1u);
 cm_native_gather_executive(s,io,&e);e.alarm=0;e.newjob=n%7u==0?077777:0;
 for(i=0;i<7u;++i)e.core[i][CM_EXEC_PRIORITY]=i<n%7u?010110:077777;
 for(i=0;i<5u;++i)e.vac[i][0]=i<n%5u?0:(agc_word)(0400u+i*44u);
 cm_native_commit_executive(s,io,&e);s->words[0705]=job[n%5u].location;s->words[0706]=job[n%5u].bank;
}
