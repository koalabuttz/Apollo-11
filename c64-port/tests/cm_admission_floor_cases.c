/* SPDX-License-Identifier: MIT */
#include "cm_admission_floor_cases.h"
#include "cm_admission_complete_cases.h"
exec_address cm_admission_floor_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){uint32_t local=n%6144u;unsigned zero=n/6144u,first=(local/4u)%8u;exec_address job=cm_admission_complete_input(s,io,local);
 s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));io->superbank=(agc_word)(((local>>8)&7u)*020u);
 s->words[0063]=0;if(local%4u<2u)s->words[0]=0;if(first>0u)s->words[0167]=0110;if(zero<7u)s->words[0167u+zero*12u]=0;return job;}
