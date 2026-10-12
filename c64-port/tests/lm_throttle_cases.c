/* SPDX-License-Identifier: MIT */
#include "lm_throttle_cases.h"
#define ANY_FB 0177777u
#define SWRETURN 04631u
const agc_word lm_thr_entry_z[LM_THR_ENTRIES]={LM_THR_THROTTLE,LM_THR_THROTTLE3,LM_THR_FLATOUT,LM_THR_WHATOUT};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static int inside(agc_word q){return (q>=LM_THR_FIRST&&q<=LM_THR_LAST)||(q>=LM_THR_FASTCHNG&&q<=LM_THR_FASTCHNG+5u);}
void lm_thr_input(apollo_overlay_state *m,lm_flight_io *io,lm_thr_case *c,uint16_t n){uint32_t seed=0x2216u+n;unsigned i,v=n/LM_THR_ENTRIES;agc_word q,eb;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 /* Counters other than TIME2/TIME1 are zero; they are not inputs here. */
 m->words[7]=0;for(i=010;i<=061u;++i)if(i!=024u&&i!=025u)m->words[i]=0;
 if(v%4u==1u)m->words[01011]=0102u; /* MODREG = 66 */
 c->superbank=0;io->bias.superbank=0;io->bias.interrupt_enabled=0;/* Channel 14 bits 1-5 (bit 4 is the thrust drive); the CDU, gyro and
  * optics drive bits would start yaAGC pulse output. */
 io->chan14=(agc_word)((rnd(&seed)>>12)&037u);
 if(v%3u==0u){q=SWRETURN;c->stop_fb=ANY_FB;}
 /* Fixed-fixed returns stay below BRANCH, DMP and TPAGREE (06723-07300),
  * which MASSMULT passes through. */
 else if(v%3u==1u){q=(agc_word)(06000u+(rnd(&seed)>>17)%0700u);c->stop_fb=ANY_FB;}
 else{do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(inside(q));c->stop_fb=LM_THR_FB;}
 eb=03400u;
 m->words[3]=eb;m->words[4]=LM_THR_FB;m->words[6]=(agc_word)(LM_THR_FB|(eb>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=q;m->words[5]=lm_thr_entry_z[n%LM_THR_ENTRIES];c->stop_z=q;
 /* THROTTLE +3 with the guidance thrust /AFC/ next to the present
  * acceleration in A,L, FWEIGHT not applied (TTHROT 512 cs old) and both
  * critical bands high, so PIF = FC - FP is small. Sub-cohorts: both
  * positive; both zero (MASSMULT's TPAGREE sees an all-zero MPAC); a tiny
  * negative acceleration (-0 high word) against a tiny positive /AFC/, so
  * PIF's high word is +0. */
 if(n%LM_THR_ENTRIES==1u&&v%8u==3u){unsigned sub=(v/8u)%3u;agc_word a=(agc_word)((rnd(&seed)>>17)%0400u),l=(agc_word)((rnd(&seed)>>17)%010000u),fa=a,fl=(agc_word)(l+(rnd(&seed)>>17)%8u);
  if(sub==1u)a=l=fa=fl=0;
  if(sub==2u){a=077777u;l=(agc_word)(077777u-(rnd(&seed)>>17)%0100u);fa=0;fl=(agc_word)((rnd(&seed)>>17)%0100u+1u);}
  m->words[0]=agc_extend(a);m->words[1]=agc_extend(l);m->words[03463]=fa;m->words[03464]=fl;
  m->words[03617]=(agc_word)((m->words[025]-01000u)&037777u);
  m->words[02507]=037000u;m->words[03620]=(agc_word)((rnd(&seed)>>17)%0100u);
 }
}
static uint16_t stop_point(const lm_thr_case *c){return agc_fixed_physical(c->stop_z,c->stop_fb==ANY_FB?0u:c->stop_fb,c->superbank);}
unsigned lm_thr_run(apollo_overlay_state *s,lm_flight_io *io,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};lm_thr_case c;
 program.basic=&lm_throttle_routines;program.basic_count=1;
 lm_thr_input(s,io,&c,n);
 return agc_transfer_run(&program,s,io,&c.superbank,stop_point(&c),24,steps)==EXEC_OK?0u:1u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_thr_digest(apollo_overlay_state *s,uint16_t n){lm_flight_io io;uint16_t steps;unsigned i,kind=lm_thr_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);mix(io.chan14);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
