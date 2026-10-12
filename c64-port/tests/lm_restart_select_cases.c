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
/* Cohort 6: the signed-zero and overflow branches. Sub-cohort (j/16)%8, so
 * every sixteenth case (the C64 sample) visits each:
 *   0    CHKNOVAC with priority +0 or -0 (job phases 00402, 01002)
 *   1-2  FINDTIME: TBASE1 the complement of TIME1, both zero, both -0, sums
 *        that overflow positive or negative; direct or indirect deltas that
 *        make the second CCS +0 or -0
 *   3-7  ITSLGCL2: LONGTIME/TIME2/LONGBASE leaving a high word of +0 or -0
 *        with a low word of +, +0, - or -0. */
static enum lm_restart_select_entry targeted(apollo_overlay_state *s,uint16_t j){
 static const agc_word low[4]={5,0,077772,077777};
 unsigned sub=(j>>4)&7u,k=(j&15u)|((j>>7)<<4),g=k%6u;uint32_t seed=0x2900u+j;agc_word t1,base,phase;
 s->words[0161]=(agc_word)g;s->words[01436u+g*2u]=(agc_word)(01200u+(j&0377u));s->words[01437u+g*2u]=02003;
 s->words[024]=(agc_word)((rnd(&seed)>>17)&3u);t1=(agc_word)((rnd(&seed)>>17)&037777u);s->words[025]=t1;
 if(sub==0u){phase=(k&2u)?01002:00402;s->words[01054u+g*2u]=(k&1u)?077777u:0;}
 else if(sub<3u){
  phase=00401;
  switch(k%6u){
  case 0:base=agc_negate(t1);break;
  case 1:s->words[025]=0;base=0;break;
  case 2:s->words[025]=077777;base=077777;break;
  case 3:s->words[025]=(agc_word)(040001u+(k&0377u));base=(agc_word)(040001u+((k*7u)&0377u));break;
  case 4:base=(agc_word)(037777u-(k&7u));s->words[025]=037777;break;
  default:base=(agc_word)((t1+(k&1u))&037777u);break;}
  s->words[01053u+g*2u]=base;
  if(sub==1u)s->words[01054u+g*2u]=(agc_word)((k&1u)?0u:(rnd(&seed)>>17)%300u+1u);
  else{s->words[01054u+g*2u]=agc_negate(01112u);s->words[01112]=(k&1u)?077777u:0;}
 }else{
  /* Each high/low class pair of the time left (+, +0, -, -0 for each word),
   * with times TIME2, TIME1 and LONGBASE in their nonnegative range:
   * LONGTIME high, low, TIME2, TIME1, LONGBASE high, low. */
  static const agc_word left[16][6]={
   {0,0,0,0,1,1},{0,1,0,0,0,037777},{0,0,0,1,1,0},{0,0,0,0,1,0},
   {0,5,0,0,037777,037777},{0,1,0,0,037777,037777},{1,0,0,1,037777,0},{1,0,0,0,037777,0},
   {0,0,1,0,0,1},{0,1,5,0,0,037777},{0,0,1,1,0,0},{0,0,1,0,0,0},
   {0,0,0,0,0,1},{0,1,1,0,0,037777},{0,0,0,1,0,0},{0,0,0,0,0,0}};
  unsigned v=(k>>3)&3u;agc_word t2=s->words[024];
  phase=00403;s->words[01054u+g*2u]=agc_negate(01110u);
  s->words[01110]=(agc_word)((sub&1u)?077777u:0);s->words[01111]=low[(k+sub)&3u];
  if(v==3u){const agc_word *x=left[(k&7u)|((sub&1u)<<3)];
   s->words[01110]=x[0];s->words[01111]=x[1];s->words[024]=x[2];s->words[025]=x[3];s->words[01151]=x[4];s->words[01152]=x[5];}
  else if(v==0u){s->words[024]=0;s->words[025]=0;s->words[01151]=0;s->words[01152]=0;}
  else if(v==1u){s->words[01151]=t2;s->words[01152]=t1;}
  else{s->words[01151]=(agc_word)((rnd(&seed)>>17)&3u);s->words[01152]=(agc_word)((rnd(&seed)>>17)&037777u);}
 }
 s->words[0154]=phase;
 return LM_RS_SELECT_GROUP;
}
enum lm_restart_select_entry lm_rsselect_input(apollo_overlay_state *s,uint16_t n){
 static const agc_word vars[8]={00402,00401,00403,01003,01403,00400,00407,01002};
 unsigned i,cohort=n/1024u,g=n%6u,pairs=lm_rsselect_pairs(),k;uint32_t seed=0x93526u+n;agc_word phase=1,v;
 uint8_t pg;enum lm_restart_select_entry entry=LM_RS_SELECT_GROUP;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;for(i=010;i<=061;++i)s->words[i]=0;
 s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=LM_RS_RESTARTS;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
 if(cohort==6u)return targeted(s,(uint16_t)(n-6144u));
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
