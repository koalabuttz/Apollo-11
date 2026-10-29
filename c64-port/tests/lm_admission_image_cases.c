/* SPDX-License-Identifier: MIT */
#include "lm_admission_image_cases.h"
exec_address lm_admission_image_input(apollo_overlay_state *m,p66_bias_io *io,uint16_t n){uint16_t i;uint8_t kind=(uint8_t)(n%4u),core=(uint8_t)((n/4u)%9u),vac=(uint8_t)((n/36u)%6u);uint32_t seed=UINT32_C(0x9905134)+n/216u;executive e;static const uint16_t pcs[4]={05072,05105,05116,05112};static const exec_address jobs[5]={{03275,064067},{02077,060101},{03534,060102},{02546,020067},{02710,060102}};exec_address job=jobs[(n/216u)%5u];agc_word priority=(agc_word)(((n/216u)%31u+1u)*01000u);
 for(i=0;i<2048u;++i){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);m->words[i]=(agc_word)((seed>>11)&AGC_MASK);}io->superbank=(agc_word)(((n/216u)%8u)*020u);io->interrupt_enabled=kind<2u?(uint16_t)((n/1728u)%2u):0;
 m->words[7]=m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054u;++i)m->words[i]=0;
 m->words[3]=(uint16_t)(((n/216u)%8u)*0400u);m->words[4]=(uint16_t)(((n/1728u)%32u)*02000u);m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));m->words[5]=pcs[kind];m->words[0]=kind<2u?priority:agc_extend(job.location);m->words[1]=agc_extend(kind<2u?m->words[1]:job.bank);m->words[2]=0705;
 p66_native_import_executive(&e,m);e.alarm=0;e.newjob=core==0?AGC_NEG_ZERO:0;e.newprio=priority;
 for(i=0;i<EXEC_CORE_COUNT;++i)e.core[i][EXEC_PRIORITY]=i<core?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<EXEC_VAC_COUNT;++i)e.vac[i][0]=i<vac?0:(agc_word)(0400u+i*44u);
 p66_native_export_executive(m,&e);m->words[0705]=job.location;m->words[0706]=job.bank;return job;
}
