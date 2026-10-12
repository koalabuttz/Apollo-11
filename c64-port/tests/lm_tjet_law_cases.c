/* SPDX-License-Identifier: MIT */
#include "lm_tjet_law_cases.h"
#define ANY_FB 0177777u
#define SWRETURN 04631u
/* Physical word of an EBANK6 address. */
#define EB6(a) (03000u+((a)-01400u))
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* A signed value of magnitude below m (m a power of two), or +0/-0. */
static agc_word small(uint32_t *s,unsigned m){agc_word v=(agc_word)((rnd(s)>>12)&(m-1u));unsigned k=(rnd(s)>>20)%16u;
 if(k==0u)return 0u;
 if(k==1u)return 077777u;
 return (rnd(s)>>24)&1u?(agc_word)(~v&077777u):v;}
static agc_word neg(agc_word v){return (agc_word)(~v&077777u);}
void lm_tj_input(apollo_overlay_state *m,lm_tj_case *c,uint16_t n){uint32_t seed=0x3252u+n;unsigned i,v=n/3u,kind;agc_word q;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 /* Counters are zero; they are not inputs here. */
 m->words[7]=0;for(i=010;i<=061u;++i)if(i!=024u&&i!=025u)m->words[i]=0;
 c->superbank=0;
 m->words[EB6(LM_TJ_AXISCTR)]=n%3u==0u?077776u:(agc_word)(n%3u-1u);
 kind=v%8u;
 if(kind!=0u){
  /* Fine-law ranges: E below the rough-law limit (|E| < 2^10 before
   * SCALEE), EDOT below 2^11; parameters of DAP size; SENSETYP and FLAT
   * signed or zero. */
  unsigned em=kind==1u?02000u:kind==2u?0200u:kind==3u?040u:01000u;
  m->words[EB6(01752u)]=small(&seed,em);
  m->words[EB6(01427u)]=small(&seed,kind>=5u?0400u:04000u);
  m->words[EB6(01500u)]=small(&seed,040000u);
  m->words[EB6(01555u)]=small(&seed,kind==6u?040u:02000u);
  m->words[EB6(01556u)]=(agc_word)((rnd(&seed)>>12)&(kind==4u?0377u:07777u));
  for(i=01551u;i<=01646u;++i)if(i!=01555u&&i!=01556u)m->words[EB6(i)]=(agc_word)((rnd(&seed)>>12)&(kind==7u?077777u:07777u));
  /* FIREDB, COASTDB, AXISDIST are small positive dead bands; the -16/+16
   * axes read them too. Cohort 7 leaves every parameter full-range. */
  for(i=0;i<3u&&kind!=7u;++i){unsigned b=01561u+020u*i;
   m->words[EB6(b)]=(agc_word)((rnd(&seed)>>12)&0377u);m->words[EB6(b+1u)]=(agc_word)((rnd(&seed)>>12)&0377u);
   m->words[EB6(b+2u)]=(agc_word)((rnd(&seed)>>12)&0777u);m->words[EB6(b+3u)]=(agc_word)((rnd(&seed)>>12)&0777u);
   m->words[EB6(b+4u)]=(agc_word)((rnd(&seed)>>12)&0377u);m->words[EB6(b+5u)]=(agc_word)((rnd(&seed)>>12)&0377u);}
  if(v%5u==4u){unsigned u=EB6(01524u+n%3u);m->words[u]=neg(m->words[u]);}
 }
 /* Rough law 2 with RUFRATE + EDOT = 1/8 / (1/ANET1 +2) exactly, so
  * RUFLAW12's AD -1/8 leaves -0 (BZF FULLTIME). */
 if(v%64u==0u){m->words[EB6(01752u)]=010000u;m->words[EB6(01427u)]=03302u;
  m->words[EB6(01551u+020u*(n%3u))]=020000u;}
 /* Return word in Q. */
 if(v%3u==0u){q=SWRETURN;c->stop_fb=ANY_FB;}
 else if(v%3u==1u){q=(agc_word)(06000u+(rnd(&seed)>>17)%01000u);c->stop_fb=ANY_FB;}
 else{do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(q>=LM_TJ_FIRST&&q<=LM_TJ_LAST);c->stop_fb=LM_TJ_FB;}
 m->words[3]=LM_TJ_EB;m->words[4]=LM_TJ_FB;m->words[6]=(agc_word)(LM_TJ_FB|(LM_TJ_EB>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=q;m->words[5]=LM_TJ_TJETLAW;c->stop_z=q;
}
static uint16_t stop_point(const lm_tj_case *c){return agc_fixed_physical(c->stop_z,c->stop_fb==ANY_FB?0u:c->stop_fb,c->superbank);}
unsigned lm_tj_run(apollo_overlay_state *s,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};lm_tj_case c;
 program.basic=&lm_tjet_law_routines;program.basic_count=1;
 lm_tj_input(s,&c,n);
 return agc_transfer_run(&program,s,0,&c.superbank,stop_point(&c),4,steps)==EXEC_OK?0u:1u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_tj_digest(apollo_overlay_state *s,uint16_t n){uint16_t steps;unsigned i,kind=lm_tj_run(s,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
