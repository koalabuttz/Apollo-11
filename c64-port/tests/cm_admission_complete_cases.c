/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_admission_complete_cases.h"
exec_address cm_admission_complete_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned i,kind=n%4u,core=(n/4u)%8u,vac=(n/32u)%6u;uint32_t seed;cm_executive e;static const agc_word pc[4]={05134,05147,05160,05154};static const exec_address jobs[5]={{03332,050066},{03325,050066},{02427,034067},{03542,014063},{03207,032066}};exec_address job=jobs[(n/192u)%5u];agc_word priority=(agc_word)(((n/192u)%31u+1u)*01000u);
 seed=0x551234u+(n/192u)%2u;for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}memset(io,0,sizeof *io);
 s->words[7]=s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054u;++i)s->words[i]=0;
 s->words[3]=(agc_word)(((n/192u)&7u)*0400u);s->words[4]=(agc_word)((((n/192u)>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=pc[kind];s->words[0]=kind<2u?priority:agc_extend(job.location);s->words[1]=kind<2u?agc_extend(s->words[1]):agc_extend(job.bank);s->words[2]=0705;io->superbank=(agc_word)((n/192u&7u)*020u);io->interrupt_enabled=kind<2u?(agc_word)((n/192u>>3)&1u):0;
 cm_native_gather_executive(s,io,&e);e.alarm=0;e.newjob=core==0u?AGC_NEG_ZERO:0;e.newprio=priority;
 for(i=0;i<7u;++i)e.core[i][CM_EXEC_PRIORITY]=i<core?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<5u;++i)e.vac[i][0]=i<vac?0:(agc_word)(0400u+i*44u);
 cm_native_commit_executive(s,io,&e);s->words[0705]=job.location;s->words[0706]=job.bank;return job;
}
