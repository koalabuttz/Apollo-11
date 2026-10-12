/* SPDX-License-Identifier: MIT */
#include "lm_exec_services_cases.h"
#include "lm_restart_routines_cases.h"
#include "p66_native.h"
#include "waitlist.h"
#define AGC_SITES_CALLEES
#include "../src/lm_restart_table_data.inc"
#include "../src/lm_exec_sites.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define ANY_FB 0177777u
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* Site k (0..145): index of its first operand word in lm_exec_sites. The
 * FIXDELAY sites belong to lm-exec-jobs. */
static uint16_t site_entry(uint16_t k){uint16_t i,seen=0;
 for(i=0;i<LM_EXEC_SITES_COUNT;++i)if(lm_exec_sites_callee[i]!=LM_EXEC_FIXDELAY&&(i==0u||lm_exec_sites[i-1u].physical!=lm_exec_sites[i].physical-1u)){if(seen==k)return i;++seen;}
 return 0;
}
/* Canonical Executive and Waitlist erasable: cores busy core sets (core set
 * 0, the running job, always busy), the first vacs VAC areas in use, count
 * Waitlist tasks. */
static void canon(apollo_overlay_state *m,uint8_t cores,uint8_t vacs,uint8_t count,uint32_t *seed){executive e;unsigned i;
 p66_native_import_executive(&e,m);e.alarm=0;e.newjob=0;
 for(i=0;i<EXEC_CORE_COUNT;++i)e.core[i][EXEC_PRIORITY]=i<cores?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<EXEC_VAC_COUNT;++i)e.vac[i][0]=i<vacs?0:(agc_word)(0400u+i*44u);
 p66_native_export_executive(m,&e);
 m->words[026]=(agc_word)(rnd(seed)>>17&037777u);
 for(i=0;i<8u;++i)m->words[01400u+i]=(agc_word)(077777u-(rnd(seed)>>16)%6000u);
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=i<count?(agc_word)(02000u+i):WL_END_LOCATION;m->words[01411u+2u*i]=i<count?(agc_word)(060007u|(i%8u)*020u):WL_END_BANK;}
}
static void random_image(apollo_overlay_state *m,uint32_t *seed){unsigned i;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(seed)>>12)&AGC_MASK);
 m->words[7]=0;for(i=010;i<=061u;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
}
static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){m->words[4]=fb;m->words[3]=eb;m->words[6]=(agc_word)(fb|(eb>>8));}
void lm_exs_input(apollo_overlay_state *m,p66_bias_io *io,lm_exs_case *c,uint16_t n){uint32_t seed=0x5203u+n;
 io->superbank=0;io->interrupt_enabled=0;c->superbank=0;
 if(n<LM_EXS_SITE_FIRST){
  agc_word g,phase;uint16_t k=lm_rsr_select_case(n);unsigned i;
  /* Skip the type-B phase 2 of the groups whose even entries are the
   * source's dummy 1.2SPOT: its second part (DEC 100, 2CADR TASKOVER) is
   * a positive word, so RESTARTS would admit TASKOVER as a job of
   * priority 0144, outside the Executive's priority domain. */
  for(;;){(void)lm_rsselect_input(m,k);
   g=m->words[5]==LM_RS_RESTARTS?m->words[0161]:(agc_word)(m->words[0155]/2u);phase=m->words[0154];
   if(!(phase==2u&&g<6u&&lm_restart_table_data[g*2u]==lm_restart_table_data[0]))break;
   k=(uint16_t)(k+6u);}
  /* The erasable times table entries name (-GENADR SAVET-30, ZOOMTIME,
   * TGO+1 deltas; LONGCALL DP times) hold valid times, as their producers
   * store them: entries are triples after the 12-word SIZETAB. */
  for(i=0;i<40u;++i){agc_word pr=lm_restart_table_data[12u+3u*i],code=lm_restart_table_data[13u+3u*i],bb=lm_restart_table_data[14u+3u*i],at;
   if(!(code&AGC_SIGN))continue;
   /* A Waitlist entry's 2CADR is stored negated (the BBCON shows 01000). */
   if(bb&01000u){if(!(pr&AGC_SIGN))continue;at=agc_negate(pr);bb=agc_negate(bb);}else at=pr;
   if(at<0100u||at>=02000u)continue;
   if(at>=01400u)at=(agc_word)((bb&7u)*0400u+(at&0377u));
   if(pr&AGC_SIGN)m->words[at]=(agc_word)((rnd(&seed)>>17)%8000u+1u);
   else{m->words[at]=(agc_word)((rnd(&seed)>>17)%4u);m->words[at+1u]=(agc_word)((rnd(&seed)>>17)&037777u);}
  }
  /* Phase time bases within 07777 centiseconds before TIME1 (wrapping). */
  for(i=0;i<6u;++i)m->words[01053u+i*2u]=(agc_word)((m->words[025]-(rnd(&seed)>>17)%010000u)&037777u);
  /* A variable job phase's stored priority is a priority (bits 10-14). */
  g=m->words[0161];phase=m->words[0154];
  if(m->words[5]==LM_RS_RESTARTS&&g<6u&&(phase&01400u)&&((phase&01000u)||(phase&7u)==2u))
   m->words[01054u+g*2u]=(agc_word)(((m->words[01054u+g*2u]>>9)%31u+1u)*01000u);
  canon(m,(uint8_t)(1u+(n/7u)%8u),(uint8_t)((n/5u)%6u),(uint8_t)((n/3u)%10u),&seed);
  c->stop_z=LM_RS_SWRETURN;c->stop_fb=ANY_FB;return;
 }
 if(n<LM_EXS_TASK_FIRST){
  uint16_t k=(uint16_t)((n-LM_EXS_SITE_FIRST)%LM_EXS_SITES),v=(uint16_t)((n-LM_EXS_SITE_FIRST)/LM_EXS_SITES),e=site_entry(k);
  uint16_t ph=lm_exec_sites[e].physical,bank=(uint16_t)(ph/1024u),off=(uint16_t)(ph%1024u);agc_word callee=lm_exec_sites_callee[e],q,fb,a,l;
  random_image(m,&seed);
  canon(m,(uint8_t)(1u+v%8u),(uint8_t)((v/2u)%6u),(uint8_t)((v*3u)%10u),&seed);
  if(bank==2u||bank==3u){q=(agc_word)(bank*02000u+off);fb=(agc_word)(((v*5u)%32u)*02000u);c->superbank=(agc_word)((v%4u)*020u);}
  else if(bank>=040u){q=(agc_word)(02000u+off);fb=(agc_word)((bank-8u)*02000u);c->superbank=0100;}
  else{q=(agc_word)(02000u+off);fb=(agc_word)(bank*02000u);c->superbank=(agc_word)((v%4u)*020u);}
  io->superbank=c->superbank;
  set_banks(m,fb,(agc_word)((v%8u)*0400u));
  m->words[2]=q;m->words[5]=callee;a=m->words[0];l=m->words[1];
  if(callee==LM_EXEC_NOVAC||callee==LM_EXEC_FINDVAC)a=(agc_word)((v%31u+1u)*01000u);
  else if(callee==LM_EXEC_LONGCALL){
   /* DP delta: positive (high or low word), zero, or negative. */
   a=(agc_word)(v%7u==0u?0u:v%7u==1u?077777u:v%7u==2u?(rnd(&seed)>>17)%0400u+040000u:(rnd(&seed)>>17)%010u);
   l=(agc_word)(v%7u==1u?(rnd(&seed)>>17)&037777u:v%7u==0u&&v%2u?0u:v%7u==0u?(rnd(&seed)>>17)%040000u|1u:(rnd(&seed)>>17)&037777u);
   if(v==7u)l=077777u;
  }else a=(agc_word)(v%9u==0u?0u:v%9u==1u?077000u:(rnd(&seed)>>17)%16250u+1u);
  m->words[0]=agc_extend(a);m->words[1]=agc_extend(l);
  c->stop_z=(agc_word)(q+(callee==LM_EXEC_TWIDDLE?1u:2u));c->stop_fb=q>=04000u?ANY_FB:fb;return;
 }
 {uint16_t t=(uint16_t)(n-LM_EXS_TASK_FIRST);agc_word high,low;
  random_image(m,&seed);
  canon(m,1,0,(uint8_t)(t%10u),&seed);
  set_banks(m,02000u,01400u);m->words[2]=(agc_word)((rnd(&seed)>>17)&07777u);
  if(t%4u){
   /* LONGCYCL: remaining time positive with either low-word sign, or an
    * exhausted high word; some fall on the constant DPBIT14. */
   high=(agc_word)((rnd(&seed)>>17)%(t%4u==1u?2u:040000u));low=(agc_word)((rnd(&seed)>>17)&077777u);
   if(t%16u==2u)high=077777u;
   m->words[LM_EXEC_LONGTIME]=high;m->words[LM_EXEC_LONGTIME+1u]=low;
   m->words[LM_EXEC_LONGEXIT]=LM_EXEC_TASKOVER;m->words[LM_EXEC_LONGEXIT+1u]=02003;
   m->words[5]=LM_EXEC_LONGCYCL;c->stop_z=LM_EXEC_TASKOVER;c->stop_fb=ANY_FB;
  }else{
   /* GETCADR: the task 2CADR LONGCALL stored, from a genuine LONGCALL/WAITLIST site. */
   uint16_t k=(uint16_t)((t/4u)%LM_EXS_SITES),e=site_entry(k);
   while(lm_exec_sites_callee[e]==LM_EXEC_TWIDDLE||lm_exec_sites[e].word==077777u||lm_exec_sites[e].word==LM_EXEC_GETCADR){k=(uint16_t)((k+1u)%LM_EXS_SITES);e=site_entry(k);}
   m->words[LM_EXEC_LONGCADR]=lm_exec_sites[e].word;m->words[LM_EXEC_LONGCADR+1u]=lm_exec_sites[e+1u].word;
   m->words[5]=LM_EXEC_GETCADR;c->stop_z=lm_exec_sites[e].word;c->stop_fb=lm_exec_sites[e].word>=04000u?ANY_FB:(agc_word)(lm_exec_sites[e+1u].word&076000u);
  }
 }
}
/* The trampoline's stop point for a case. */
static uint16_t stop_point(const lm_exs_case *c){
 return agc_fixed_physical(c->stop_z,c->stop_fb==ANY_FB?0u:c->stop_fb,c->superbank);
}
unsigned lm_exs_run(apollo_overlay_state *s,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};p66_bias_io io;lm_exs_case c;enum exec_status st;
 tables[0]=lm_exec_routines;tables[1]=lm_restart_routines;program.basic=tables;program.basic_count=2;
 program.operands=lm_exec_site_table(&program.operand_count);
 lm_exs_input(s,&io,&c,n);
 st=agc_transfer_run(&program,s,&io,&c.superbank,stop_point(&c),48,steps);
 if(st==EXEC_OK)return LM_EXS_RETURNED;
 if(st==EXEC_UNMAPPED&&(s->words[5]==LM_EXEC_BAILOUT1||s->words[5]==LM_EXEC_POODOO1))return LM_EXS_ALARM;
 if(st==EXEC_OUT_OF_SCOPE)return LM_EXS_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_exs_digest(apollo_overlay_state *s,uint16_t n){uint16_t steps;unsigned i,kind=lm_exs_run(s,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LM_EXS_REFUSED)for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
