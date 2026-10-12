/* SPDX-License-Identifier: MIT */
#include "lm_restart_select_cases.h"
#include "agc_word.h"
#define LM_RESTART_TABLE_PHASES
#include "../src/lm_restart_table_data.inc"
/* Groups 2, 3 and 6 share 1.2SPOT for their even phase 2 (SIZETAB 010). */
static const uint8_t shared_even[3]={1,2,5};
uint16_t lm_rsselect_pairs(void){return (uint16_t)(LM_RESTART_TABLE_PHASE_COUNT+3u);}
void lm_rsselect_pair(uint16_t k,uint8_t *group,agc_word *phase){
 if(k<LM_RESTART_TABLE_PHASE_COUNT){*group=lm_restart_table_phases[k][0];*phase=lm_restart_table_phases[k][1];}
 else{*group=shared_even[k-LM_RESTART_TABLE_PHASE_COUNT];*phase=2;}
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word sizetab(unsigned index){return lm_restart_table_data[index-LM_RESTART_TABLE_FIRST];}
enum lm_restart_select_entry lm_rsselect_input(apollo_overlay_state *s,uint16_t n){
 static const agc_word vars[8]={00402,00401,00403,01003,01403,00400,00407,01002};
 unsigned i,cohort=n/1024u,g=n%6u,pairs=lm_rsselect_pairs(),k;uint32_t seed=0x93526u+n;agc_word phase=1,v;
 uint8_t pg;enum lm_restart_select_entry entry=LM_RS_SELECT_GROUP;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;for(i=010;i<=061;++i)s->words[i]=0;
 s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=LM_RS_RESTARTS;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 if(cohort==0u){phase=vars[(n/6u)%8u];}
 else if(cohort==2u){phase=1;}
 else{
  k=(n/6u)%pairs;
  if(cohort==3u){do{lm_rsselect_pair((uint16_t)k,&pg,&phase);k=(k+1u)%pairs;}while(phase&1u);}
  else lm_rsselect_pair((uint16_t)k,&pg,&phase);
  g=pg;
 }
 s->words[0161]=(agc_word)g;s->words[0154]=phase;
 if(cohort==3u||cohort==4u){
  s->words[0155]=(agc_word)(g*2u);s->words[0157]=04631;
  if(cohort==3u){s->words[0156]=(agc_word)(sizetab(2u+g*2u)+3u*phase);s->words[5]=LM_RS_PHSPART2;entry=LM_RS_SELECT_PART2;}
  else{s->words[5]=LM_RS_GETPART2;entry=LM_RS_SELECT_TYPEB_TABLE;}
 }
 /* Variable phases: the group's stored 2CADR, priority or delta. Waitlist
  * deltas are direct, indirect through 01112 or immediate; longcall deltas
  * name the DP time at 01110. */
 s->words[01436u+g*2u]=(agc_word)(01200u+(n&0377u));s->words[01437u+g*2u]=02003;
 v=(agc_word)(phase&7u);
 s->words[01054u+g*2u]=v>=3u?agc_negate(01110u):v==2u?((n&1u)?010000u:067777u):(n%3u)==0u?(agc_word)(n%300u):(n%3u)==1u?agc_negate(01112u):077777u;
 s->words[01110]=(agc_word)((n>>3)&3u);s->words[01111]=(agc_word)(n%12000u);s->words[01112]=(agc_word)(n%400u);
 if(cohort==5u){s->words[024]=(agc_word)(037770u+(n&7u));s->words[025]=(agc_word)(037700u+(n&077u));s->words[01053u+g*2u]=(agc_word)(040010u-(n&017u));}
 else{s->words[024]=(agc_word)((n>>4)&3u);s->words[025]=(agc_word)(n%10000u);}
 return entry;
}
