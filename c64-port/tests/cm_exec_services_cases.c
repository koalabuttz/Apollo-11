/* SPDX-License-Identifier: MIT */
#include "cm_exec_services_cases.h"
#define AGC_SITES_CALLEES
#include "../src/cm_exec_sites.inc"
static const uint16_t first[CM_EXS_COHORTS+1]={0,4128,5152,5600,6112,6624,7648};
enum cm_exs_cohort cm_exs_cohort(uint16_t n){uint8_t k=0;while(k+1u<CM_EXS_COHORTS&&n>=first[k+1u])++k;return (enum cm_exs_cohort)k;}
uint16_t cm_exs_first(enum cm_exs_cohort k){return first[k];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static uint16_t r17(uint32_t *s){return (uint16_t)(rnd(s)>>17);}
static void stop(cm_exs_case *c,agc_word z,agc_word fb){if(c->stops<CM_EXS_MAX_STOPS){c->stop_z[c->stops]=z;c->stop_fb[c->stops]=z>=04000u?CM_EXS_ANY_FB:fb;++c->stops;}}
/* Site k: index of its first operand word in cm_exec_sites, counting the
 * NOVAC/FINDVAC/WAITLIST/LONGCALL/TWIDDLE sites (fixdelay=0) or the FIXDELAY
 * sites (fixdelay=1). */
static uint16_t site_entry(uint16_t k,int fixdelay){uint16_t i,seen=0;
 for(i=0;i<CM_EXEC_SITES_COUNT;++i)if((cm_exec_sites_callee[i]==CM_EXEC_FIXDELAY)==(fixdelay!=0)&&(i==0u||cm_exec_sites[i-1u].physical!=cm_exec_sites[i].physical-1u||cm_exec_sites_callee[i-1u]==CM_EXEC_TWIDDLE||cm_exec_sites_callee[i-1u]==CM_EXEC_FIXDELAY)){if(seen==k)return i;++seen;}
 return 0;
}
/* Canonical Executive and Waitlist erasable: cores busy core sets (core set
 * 0, the running job, always busy), the first vacs VAC areas in use, count
 * Waitlist tasks. */
static void canon(apollo_overlay_state *m,uint8_t cores,uint8_t vacs,uint8_t count,uint32_t *seed){unsigned i;
 for(i=0;i<7u;++i)m->words[0167u+12u*i]=i<cores?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<5u;++i)m->words[0400u+44u*i]=i<vacs?0:(agc_word)(0400u+i*44u);
 m->words[067]=0;
 m->words[026]=(agc_word)(r17(seed)&037777u);
 for(i=0;i<8u;++i)m->words[01400u+i]=(agc_word)(077777u-r17(seed)%6000u);
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=i<count?(agc_word)(02000u+i):CM_WL_END_LOCATION;m->words[01411u+2u*i]=i<count?(agc_word)(060007u|(i%8u)*020u):CM_WL_END_BANK;}
}
/* Flight-typical edges over a canonical state, by case (n/13)%8: TIME3 one
 * to four centiseconds before its overflow (0), every later task due 1 cs
 * after the one before (1), equal priorities in the busy core sets (2),
 * NEWJOB -0 (3), TIME3 just overflowed with the first task due next (4).
 * They leave the seeded draws as they were. */
static void edges(apollo_overlay_state *m,uint16_t n){unsigned i;
 switch((n/13u)%8u){
 case 0:m->words[026]=(agc_word)(037777u-n%4u);break;
 case 1:m->words[01400]=(agc_word)(077777u-1u-n%3u);for(i=1;i<8u;++i)m->words[01400u+i]=077776u;break;
 case 2:for(i=1;i<8u;++i)if(!(m->words[0167u+12u*i]&AGC_SIGN))m->words[0167u+12u*i]=011110u;break;
 case 3:m->words[067]=AGC_NEG_ZERO;break;
 case 4:m->words[026]=0;m->words[01400]=077776u;break;
 default:break;}
}
/* Tasks due together: LST1+1..+7 hold +0 (a later task due at the same
 * time as the one before it), the state that takes WTLST5's last CCS to +0,
 * with TIME3 close to its next overflow and the first task due soon. */
static void together(apollo_overlay_state *m,uint32_t *seed){unsigned i;
 m->words[026]=(agc_word)(037740u+r17(seed)%040u);
 m->words[01400]=(agc_word)(077777u-r17(seed)%8u-1u);for(i=1;i<8u;++i)m->words[01400u+i]=0;
}
static void random_image(apollo_overlay_state *m,uint32_t *seed){unsigned i;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(seed)>>12)&AGC_MASK);
 m->words[7]=0;for(i=010;i<=061u;++i)m->words[i]=0;
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
}
static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){m->words[4]=fb;m->words[3]=eb;m->words[6]=(agc_word)(fb|(eb>>8));}
/* Q at the operand word ph (physical) with the caller's FB and SUPERBNK. */
static agc_word place(apollo_overlay_state *m,cm_native_io *io,uint16_t ph,uint16_t v){uint16_t bank=(uint16_t)(ph/1024u),off=(uint16_t)(ph%1024u);agc_word q,fb;
 if(bank==2u||bank==3u){q=(agc_word)(bank*02000u+off);fb=(agc_word)(((v*5u)%32u)*02000u);io->superbank=(agc_word)((v%4u)*020u);}
 else if(bank>=040u){q=(agc_word)(02000u+off);fb=(agc_word)((bank-8u)*02000u);io->superbank=0100;}
 else{q=(agc_word)(02000u+off);fb=(agc_word)(bank*02000u);io->superbank=(agc_word)((v%4u)*020u);}
 set_banks(m,fb,(agc_word)((v%8u)*0400u));m->words[2]=q;return fb;
}
void cm_exs_input(apollo_overlay_state *m,cm_native_io *io,cm_exs_case *c,uint16_t n){
 enum cm_exs_cohort k=cm_exs_cohort(n);uint16_t j=(uint16_t)(n-first[k]);uint32_t seed=0x5245u+n;
 io->channel11=io->channel12=io->channel13=io->channel14=0;io->main_keys=io->navigation_keys=0;
 io->superbank=0;io->interrupt_enabled=0;io->trap31a=io->trap31b=io->trap32=0;io->executive_alarm=io->waitlist_alarm=0;
 c->stops=0;c->resume=0;c->hw.channel11=0;c->hw.in_interrupt=0;
 random_image(m,&seed);
 switch(k){
 case CM_EXS_SITE:{
  uint16_t s=(uint16_t)(j%CM_EXS_SITES),v=(uint16_t)(j/CM_EXS_SITES),e=site_entry(s,0);agc_word callee=cm_exec_sites_callee[e],fb,a,l;
  canon(m,(uint8_t)(1u+v%7u),(uint8_t)((v/2u)%6u),(uint8_t)((v*3u)%10u),&seed);edges(m,n);
  if(v%8u==5u)together(m,&seed);
  fb=place(m,io,cm_exec_sites[e].physical,v);m->words[5]=callee;a=m->words[0];l=m->words[1];
  io->interrupt_enabled=(agc_word)(v/16u);
  if(callee==CM_EXEC_NOVAC||callee==CM_EXEC_FINDVAC)a=(agc_word)((v%31u+1u)*01000u);
  else if(callee==CM_EXEC_LONGCALL){
   /* DP delta: positive (high or low word), zero, or negative. */
   a=(agc_word)(v%7u==0u?0u:v%7u==1u?077777u:v%7u==2u?r17(&seed)%0400u+040000u:r17(&seed)%010u);
   l=(agc_word)(v%7u==1u?r17(&seed)&037777u:v%7u==0u&&v%2u?0u:v%7u==0u?(r17(&seed)%040000u)|1u:r17(&seed)&037777u);
   if(v==7u)l=077777u;
  }else a=(agc_word)(v%8u==5u?r17(&seed)%48u+1u:v%9u==0u?0u:v%9u==1u?077000u:r17(&seed)%16250u+1u);
  m->words[0]=agc_extend(a);m->words[1]=agc_extend(l);
  stop(c,(agc_word)(m->words[2]+(callee==CM_EXEC_TWIDDLE?1u:2u)),fb);break;}
 case CM_EXS_LONGTASK:{agc_word high,low;
  canon(m,1,0,(uint8_t)(j%10u),&seed);edges(m,n);
  set_banks(m,02000u,01400u);m->words[2]=(agc_word)(r17(&seed)&07777u);
  if(j%4u){
   /* LONGCYCL: remaining time positive with either low-word sign, or an
    * exhausted high word; some fall on the constant DPBIT14. */
   high=(agc_word)(r17(&seed)%(j%4u==1u?2u:040000u));low=(agc_word)(r17(&seed)&077777u);
   if(j%16u==2u)high=077777u;
   m->words[CM_EXEC_LONGTIME]=high;m->words[CM_EXEC_LONGTIME+1u]=low;
   m->words[CM_EXEC_LONGEXIT]=CM_EXEC_TASKOVER;m->words[CM_EXEC_LONGEXIT+1u]=02003;
   m->words[5]=CM_EXEC_LONGCYCL;stop(c,CM_EXEC_TASKOVER,0);
  }else{
   /* GETCADR: the task 2CADR LONGCALL stored, from a genuine LONGCALL/WAITLIST site. */
   uint16_t s=(uint16_t)((j/4u)%CM_EXS_SITES),e=site_entry(s,0);
   while(cm_exec_sites_callee[e]==CM_EXEC_TWIDDLE||cm_exec_sites[e].word==077777u||cm_exec_sites[e].word==CM_EXEC_GETCADR||cm_exec_sites[e].word==CM_EXEC_LONGCYCL){s=(uint16_t)((s+1u)%CM_EXS_SITES);e=site_entry(s,0);}
   m->words[CM_EXEC_LONGCADR]=cm_exec_sites[e].word;m->words[CM_EXEC_LONGCADR+1u]=cm_exec_sites[e+1u].word;
   io->superbank=(agc_word)(cm_exec_sites[e+1u].word&0160u);
   m->words[5]=CM_EXEC_GETCADR;stop(c,cm_exec_sites[e].word,(agc_word)(cm_exec_sites[e+1u].word&076000u));
  }
  break;}
 case CM_EXS_FIXDELAY:case CM_EXS_VARDELAY:{uint16_t v;
  /* A task (in T3RUPT, interrupts inhibited) requeues its remainder. */
  if(k==CM_EXS_FIXDELAY){uint16_t e=site_entry((uint16_t)(j%CM_EXS_FIXDELAY_SITES),1);v=(uint16_t)(j/CM_EXS_FIXDELAY_SITES);
   (void)place(m,io,cm_exec_sites[e].physical,v);m->words[5]=CM_EXEC_FIXDELAY;}
  else{v=(uint16_t)(j%32u);set_banks(m,(agc_word)(((j*7u)%32u)*02000u),(agc_word)((j%8u)*0400u));
   m->words[2]=(agc_word)(0100u+r17(&seed)%07677u);io->superbank=(agc_word)((j%4u)*020u);
   m->words[0]=(agc_word)(v%8u==0u?r17(&seed)%037777u+1u:v%8u==1u?r17(&seed)%16u+1u:r17(&seed)%16250u+1u);m->words[5]=CM_EXEC_VARDELAY;}
  canon(m,1,0,(uint8_t)((v*3u+j)%10u),&seed);edges(m,n);
  if(v%4u==3u){together(m,&seed);if(k==CM_EXS_VARDELAY)m->words[0]=(agc_word)(r17(&seed)%48u+1u);}
  c->hw.in_interrupt=1;stop(c,CM_EXEC_TASKOVER,0);break;}
 case CM_EXS_RESUME:{agc_word z;
  /* The interrupted program: ZRUPT, BANKRUPT (with SUPERBNK), ARUPT, LRUPT, QRUPT. */
  canon(m,1,0,(uint8_t)(j%10u),&seed);edges(m,n);
  set_banks(m,(agc_word)((r17(&seed)%32u)*02000u),(agc_word)((j%8u)*0400u));
  m->words[0734]=AGC_NEG_ZERO;z=(agc_word)(02001u+r17(&seed)%06000u);m->words[015]=z;
  m->words[016]=(agc_word)(((r17(&seed)%32u)*02000u)|(j%8u)|((j/8u)%4u)*020u);
  m->words[010]=(agc_word)(r17(&seed)&077777u);m->words[011]=(agc_word)(r17(&seed)&077777u);m->words[012]=(agc_word)(r17(&seed)&07777u);
  m->words[2]=(agc_word)(r17(&seed)&07777u);io->superbank=(agc_word)((j%4u)*020u);
  m->words[5]=CM_EXEC_TASKOVER;c->hw.in_interrupt=1;c->resume=1;
  stop(c,(agc_word)(z-1u),(agc_word)(m->words[016]&076000u));break;}
 case CM_EXS_DISPATCH:{uint8_t i;
  /* The T3 vector (the interrupted program's A, L, Q and BB) or TASKOVER
   * with RUPTAGN +1 (bank 01); either interrupt-enable state. */
  canon(m,1,0,(uint8_t)(j%10u),&seed);edges(m,n);
  io->interrupt_enabled=(agc_word)((j/2u)%2u);io->superbank=(agc_word)(((j/4u)%4u)*020u);
  m->words[2]=(agc_word)(r17(&seed)&07777u);m->words[012]=(agc_word)(r17(&seed)&07777u);
  if(j%4u<2u){set_banks(m,(agc_word)((r17(&seed)%32u)*02000u),(agc_word)((j%8u)*0400u));m->words[5]=CM_EXEC_T3RUPT_VECTOR;
   m->words[015]=(agc_word)(02001u+r17(&seed)%06000u);}
  else{set_banks(m,02000u,(agc_word)((j%8u)*0400u));m->words[0734]=1;m->words[5]=CM_EXEC_TASKOVER;
   m->words[016]=(agc_word)(((r17(&seed)%32u)*02000u)|(j%8u));}
  c->hw.in_interrupt=1;
  for(i=0;i<9u;++i)stop(c,m->words[01410u+2u*i],(agc_word)(m->words[01411u+2u*i]&076000u));
  break;}
 default:break;
 }
}
/* The trampoline's stop point for a case: its first stop. */
static uint16_t stop_point(const cm_exs_case *c,agc_word superbank){
 return agc_fixed_physical(c->stop_z[0],c->stop_fb[0]==CM_EXS_ANY_FB?0u:c->stop_fb[0],superbank);
}
unsigned cm_exs_run(apollo_overlay_state *s,cm_native_io *io,cm_exs_case *c,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;uint8_t i;agc_word z,fb;
 program.basic=&cm_exec_routines;program.basic_count=1;
 program.operands=cm_exec_site_table(&program.operand_count);
 cm_exs_input(s,io,c,n);program.hardware=&c->hw;
 st=agc_transfer_run(&program,s,io,&io->superbank,c->resume?stop_point(c,io->superbank):c->stops==1u?stop_point(c,io->superbank):AGC_TRANSFER_NONE,48,steps);
 if(st==EXEC_OUT_OF_SCOPE)return CM_EXS_REFUSED;
 if(st==EXEC_OK)return CM_EXS_RETURNED;
 if(st!=EXEC_UNMAPPED)return 3u;
 z=s->words[5];fb=s->words[4];
 if(z==CM_EXEC_BAILOUT||z==CM_EXEC_POODOO)return CM_EXS_ALARM;
 for(i=0;i<c->stops;++i)if(z==c->stop_z[i]&&(c->stop_fb[i]==CM_EXS_ANY_FB||fb==c->stop_fb[i]))return CM_EXS_RETURNED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_exs_digest(apollo_overlay_state *s,uint16_t n){cm_native_io io;cm_exs_case c;uint16_t steps;unsigned i,kind=cm_exs_run(s,&io,&c,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=CM_EXS_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(io.superbank);mix(io.interrupt_enabled);mix(c.hw.in_interrupt);}
 return (uint32_t)sum_b<<16|sum_a;
}
