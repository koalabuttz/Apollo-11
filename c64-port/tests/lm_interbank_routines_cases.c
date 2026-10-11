/* SPDX-License-Identifier: MIT */
#include <string.h>
#define AGC_SITES_CALLEES 1
#include "lm_interbank_routines_cases.h"
#include "../src/lm_interbank_sites.inc"
uint32_t lm_ibr_cases(void){return (uint32_t)LM_INTERBANK_SITES_COUNT*LM_IBR_CONTEXTS;}
uint16_t lm_ibr_site(uint32_t n){return lm_interbank_sites[n%LM_INTERBANK_SITES_COUNT].physical;}
enum lm_interbank_entry lm_ibr_entry(agc_word callee){return callee==04616u?LM_IB_BANKCALL:callee==04674u?LM_IB_IBNKCALL:callee==04635u?LM_IB_POSTJUMP:LM_IB_USPRCADR;}
const agc_native_program lm_ibr_program={&lm_interbank_routines,1,0,0,lm_interbank_sites,LM_INTERBANK_SITES_COUNT,AGC_TRANSFER_NONE,0164,0165,0,0};
agc_word lm_ibr_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t n){static const agc_word a[LM_IBR_CONTEXTS]={012345,0177777,040000,0100001};
 uint16_t site=(uint16_t)(n%LM_INTERBANK_SITES_COUNT),ctx=(uint16_t)(n/LM_INTERBANK_SITES_COUNT),p=lm_interbank_sites[site].physical,bank=(uint16_t)(p/1024u),offset=(uint16_t)(p%1024u),i;uint32_t x=(uint32_t)(0x9e3779b9ul*(n+1u));
 memset(s,0,sizeof *s);memset(io,0,sizeof *io);
 for(i=010;i<APOLLO_SHARED_WORDS;++i){x=(uint32_t)(x*1103515245ul+12345ul);s->words[i]=(agc_word)((x>>9)&077777u);}
 s->words[0]=a[ctx%LM_IBR_CONTEXTS];s->words[1]=(agc_word)(x&0177777u);s->words[3]=(agc_word)((ctx*3u%8u)*0400u);io->interrupt_enabled=(uint16_t)(ctx&1u);
 if(bank==2u||bank==3u){s->words[4]=(agc_word)(((ctx*5u)%030u)<<10);s->words[2]=(agc_word)(bank*02000u+offset);}
 else if(bank>=040u){s->words[4]=(agc_word)((bank-8u)<<10);io->superbank=0100u;s->words[2]=(agc_word)(02000u+offset);}
 else{s->words[4]=(agc_word)(bank<<10);s->words[2]=(agc_word)(02000u+offset);}
 s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=lm_interbank_sites_callee[site];
 return s->words[5];
}
