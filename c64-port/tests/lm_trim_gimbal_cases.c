/* SPDX-License-Identifier: MIT */
#include "lm_trim_gimbal_cases.h"
#include "lm_interbank_routines.h"
#define ANY_FB 0177777u
#define SWRETURN 04631u
/* Physical word of an EBANK6 address. */
#define EB6(a) (03000u+((a)-01400u))
const agc_word lm_tg_entry_z[LM_TG_ENTRIES]={LM_TG_GTS,LM_TG_ACDTC12,LM_TG_TIMEGMBL,LM_TG_TIMERET};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* A signed value below 2^w (w 0-14), or +0/-0. */
static agc_word width(uint32_t *s,unsigned w){agc_word v=(agc_word)((rnd(s)>>12)&((1u<<w)-1u));unsigned k=(rnd(s)>>20)%16u;
 if(k==0u)return 0u;
 if(k==1u)return 077777u;
 return (rnd(s)>>24)&1u?(agc_word)(~v&077777u):v;}
static unsigned w15(uint32_t *s){return (rnd(s)>>20)%15u;}
/* A non-negative value below 2^w for a random w. */
static agc_word positive(uint32_t *s){unsigned w=w15(s);return (agc_word)((rnd(s)>>12)&((1u<<w)-1u));}
static int inside(agc_word q,agc_word fb){return fb==LM_TG_FB21?q>=LM_TG_FIRST21&&q<=LM_TG_LAST21:q>=LM_TG_FIRST16&&q<=LM_TG_LAST16;}
void lm_tg_input(apollo_overlay_state *m,lm_flight_io *io,lm_tg_case *c,uint16_t n){uint32_t seed=0x3077u+n;unsigned i,v=n/LM_TG_ENTRIES,e=n%LM_TG_ENTRIES;
 agc_word q,fb=e==1u?LM_TG_FB16:LM_TG_FB21,qfb;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 /* Counters and the interrupt registers are zero, except SR. */
 m->words[7]=0;for(i=010;i<=061u;++i)if(i!=021u)m->words[i]=0;
 c->superbank=0;io->bias.superbank=0;io->bias.interrupt_enabled=0;
 io->hw.channel14=0;io->hw.channel5=io->hw.channel6=io->hw.channel30=io->hw.channel31=0;io->hw.channel12=(agc_word)((rnd(&seed)>>12)&077777u);
 if(v%16u!=0u){
  /* AOSQ/R (with their low words), EDOTQ/R, KQ/R and ACCDOTQ/R (positive
   * except in cohort 8), QDIFF/RDIFF, NEGUQ/R +1, +0, -1 or -0. */
  for(i=0;i<4u;++i)m->words[EB6(01537u+i)]=width(&seed,w15(&seed));
  for(i=0;i<2u;++i){m->words[EB6(01436u+i)]=width(&seed,w15(&seed));m->words[EB6(01450u+2u*i)]=width(&seed,w15(&seed));
   m->words[EB6(01451u+2u*i)]=width(&seed,w15(&seed));
   m->words[EB6(01504u+2u*i)]=v%16u!=8u?positive(&seed):width(&seed,w15(&seed));
   m->words[EB6(01507u+2u*i)]=v%16u!=8u?positive(&seed):width(&seed,w15(&seed));
   {static const agc_word negu[4]={1u,0u,077776u,077777u};m->words[EB6(01501u+2u*i)]=negu[(rnd(&seed)>>20)%4u];}}
  if(v%4u)m->words[021]=width(&seed,13u);
  /* GTS near the switching curve with a small offset acceleration, where
   * GTSQRT takes the zero root and DOSHIFT's INDEX SHFTFLAG indexes CA BIT15
   * by ACENTRAL/2: the certificate review's four cases (AOSQ, EDOTQ, KQ),
   * EDOTQ moved by up to +-3 and the signs of AOSQ and EDOTQ flipped
   * together, with SR, QDIFF and the R axis zero. */
  if(e==0u&&v%16u==5u){static const agc_word ex[4][3]={{077773u,2u,01750u},{077731u,0102u,05360u},{046u,077675u,05360u},
    {077740u,0173u,01750u}};const agc_word *x=ex[(v/16u)%4u];int d=(int)((rnd(&seed)>>20)%7u)-3;agc_word aos=x[0],ed=x[1];
   if((v/64u)%2u){aos=(agc_word)(~aos&077777u);ed=(agc_word)(~ed&077777u);}
   {int s=(ed&040000u?-(int)(~ed&037777u):(int)ed)+d;ed=(agc_word)(s<0?077777u&~(unsigned)(-s):(unsigned)s);}
   for(i=0;i<4u;++i)m->words[EB6(01537u+i)]=0u;
   m->words[EB6(01537u)]=aos;m->words[EB6(01436u)]=ed;m->words[EB6(01437u)]=0u;m->words[EB6(01504u)]=x[2];m->words[EB6(01506u)]=0u;
   for(i=0;i<4u;++i)m->words[EB6(01450u+i)]=0u;
   m->words[021]=0u;}
 }
 /* Return: SWRETURN, fixed-fixed 06000-07777, or this entry's bank outside
  * this file. ACDT+C12 returns to Q, 21,3435 to the word in RUPTREG2. */
 if(v%3u==0u){q=SWRETURN;qfb=ANY_FB;}
 else if(v%3u==1u){q=(agc_word)(06000u+(rnd(&seed)>>17)%02000u);qfb=ANY_FB;}
 else{do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(inside(q,fb));qfb=fb;}
 c->stop_z=0;c->stop_fb=ANY_FB;
 if(e==1u){m->words[2]=q;c->stop_z=q;c->stop_fb=qfb;}
 if(e==2u){c->stop_z=LM_TG_ACDTC12;c->stop_fb=LM_TG_FB16;}
 if(e==3u){m->words[071]=q;c->stop_z=q;c->stop_fb=qfb;}
 m->words[3]=LM_TG_EB;m->words[4]=fb;m->words[6]=(agc_word)(fb|(LM_TG_EB>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[5]=lm_tg_entry_z[e];
}
/* This file's compiled site operand: the IBNKCALL CADR ACDT+C12, as in
 * src/lm_interbank_sites.inc (the host test checks it against
 * lm_interbank_site_table). */
const agc_site_operand lm_tg_sites[LM_TG_SITES]={{18204u,035736}};
/* The program's passive fixed_data: the rope words DOSHIFT's CA can read
 * after the zero root (bank 21 and fixed-fixed). */
#include "lm_trim_gimbal_ca_table.inc"
static bool tg_fixed(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;
 if(physical>=021u*1024u&&physical<022u*1024u){*word=gts_ca_fixed[physical-021u*1024u];return true;}
 if(physical>=2048u&&physical<4096u){*word=gts_ca_fixed[1024u+physical-2048u];return true;}
 return false;
}
unsigned lm_tg_run(apollo_overlay_state *s,lm_flight_io *io,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};lm_tg_case c;enum exec_status st;uint16_t stop;
 program.hardware=&io->hw;
 tables[0]=lm_trim_gimbal_routines;tables[1]=lm_interbank_routines;
 program.basic=tables;program.basic_count=2;program.operands=lm_tg_sites;program.operand_count=LM_TG_SITES;
 program.fixed_data=tg_fixed;
 lm_tg_input(s,io,&c,n);
 stop=c.stop_z==0u?(uint16_t)(AGC_TRANSFER_NONE-1u):agc_fixed_physical(c.stop_z,c.stop_fb==ANY_FB?0u:c.stop_fb,c.superbank);
 st=agc_transfer_run(&program,s,io,&c.superbank,stop,8,steps);
 if(st==EXEC_OK)return LM_TG_RETURNED;
 if(st==EXEC_OUT_OF_SCOPE&&n%LM_TG_ENTRIES==0u&&*steps==1u)return LM_TG_REFUSED;
 if(st==EXEC_UNMAPPED&&s->words[5]==LM_TG_CLOSEOUT&&(s->words[4]&076000u)==036000u)return LM_TG_CLOSED;
 return LM_TG_NONE;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_tg_digest(apollo_overlay_state *s,uint16_t n){lm_flight_io io;uint16_t steps;unsigned i,kind=lm_tg_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);mix(io.hw.channel12);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
