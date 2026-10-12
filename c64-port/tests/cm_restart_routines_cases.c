/* SPDX-License-Identifier: MIT */
#include "cm_restart_routines_cases.h"
#define CM_RESTART_TABLE
#define CM_RESTART_TABLE_PHASES
#include "../src/cm_restart_table_data.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define TIME2 024u
#define TIME1 025u
#define LONGBASE 01135u
uint16_t cm_rr_pairs(void){return (uint16_t)CM_RESTART_TABLE_PHASE_COUNT;}
void cm_rr_pair(uint16_t k,uint8_t *group,agc_word *phase){*group=cm_restart_table_phases[k][0];*phase=cm_restart_table_phases[k][1];}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static void image(apollo_overlay_state *s,uint32_t seed){unsigned i;
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 s->words[7]=0;for(i=010;i<=061;++i)s->words[i]=0;
 s->words[0]=agc_extend(s->words[0]);s->words[1]=agc_extend(s->words[1]);
}
/* Cohort 6: the signed-zero and overflow branches. Sub-cohort (j/16)%8, so
 * every sixteenth case (the C64 sample) visits each:
 *   0    CHKNOVAC with priority +0 or -0 (job phases 00402, 01002)
 *   1-2  FINDTIME: TBASE1 the complement of TIME1, both zero, both -0, sums
 *        that overflow positive or negative; direct or indirect deltas that
 *        make the second CCS +0 or -0
 *   3-7  ITSLGCL2: LONGTIME/TIME2/LONGBASE leaving a high word of +0 or -0
 *        with a low word of +, +0, - or -0. */
static enum cm_rr_entry targeted(apollo_overlay_state *s,uint16_t j){
 static const agc_word low[4]={5,0,077772,077777};
 unsigned sub=(j>>4)&7u,k=(j&15u)|((j>>7)<<4),g=k%6u;uint32_t seed=0x2900u+j;agc_word t1,base,phase;
 s->words[0161]=(agc_word)g;s->words[01436u+g*2u]=(agc_word)(01200u+(j&0377u));s->words[01437u+g*2u]=02003;
 s->words[TIME2]=(agc_word)((rnd(&seed)>>17)&3u);t1=(agc_word)((rnd(&seed)>>17)&037777u);s->words[TIME1]=t1;
 if(sub==0u){phase=(k&2u)?01002:00402;s->words[01054u+g*2u]=(k&1u)?077777u:0;}
 else if(sub<3u){
  phase=00401;
  switch(k%6u){
  case 0:base=agc_negate(t1);break;
  case 1:s->words[TIME1]=0;base=0;break;
  case 2:s->words[TIME1]=077777;base=077777;break;
  case 3:s->words[TIME1]=(agc_word)(040001u+(k&0377u));base=(agc_word)(040001u+((k*7u)&0377u));break;
  case 4:base=(agc_word)(037777u-(k&7u));s->words[TIME1]=037777;break;
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
  unsigned v=(k>>3)&3u;agc_word t2=s->words[TIME2];
  phase=00403;s->words[01054u+g*2u]=agc_negate(01110u);
  s->words[01110]=(agc_word)((sub&1u)?077777u:0);s->words[01111]=low[(k+sub)&3u];
  if(v==3u){const agc_word *x=left[(k&7u)|((sub&1u)<<3)];
   s->words[01110]=x[0];s->words[01111]=x[1];s->words[TIME2]=x[2];s->words[TIME1]=x[3];s->words[LONGBASE]=x[4];s->words[LONGBASE+1u]=x[5];}
  else if(v==0u){s->words[TIME2]=0;s->words[TIME1]=0;s->words[LONGBASE]=0;s->words[LONGBASE+1u]=0;}
  else if(v==1u){s->words[LONGBASE]=t2;s->words[LONGBASE+1u]=t1;}
  else{s->words[LONGBASE]=(agc_word)((rnd(&seed)>>17)&3u);s->words[LONGBASE+1u]=(agc_word)((rnd(&seed)>>17)&037777u);}
 }
 s->words[0154]=phase;
 return CM_RR_GROUP;
}
enum cm_rr_entry cm_rr_select_input(apollo_overlay_state *s,uint16_t n){
 static const agc_word vars[8]={00402,00401,00403,01003,01403,00400,00407,01002};
 unsigned cohort=n/1024u,g=n%6u,pairs=cm_rr_pairs(),k;agc_word phase=1,v;
 uint8_t pg;enum cm_rr_entry entry=CM_RR_GROUP;
 image(s,0x93520u+n);
 s->words[3]=01400;s->words[4]=02000;s->words[6]=02003;s->words[5]=CM_RR_RESTARTS;
 if(cohort==6u)return targeted(s,(uint16_t)(n-6144u));
 if(cohort==0u){phase=vars[(n/6u)%8u];}
 else if(cohort==2u){phase=1;}
 else{
  k=(n/6u)%pairs;
  if(cohort==3u){do{cm_rr_pair((uint16_t)k,&pg,&phase);k=(k+1u)%pairs;}while(phase&1u);}
  else cm_rr_pair((uint16_t)k,&pg,&phase);
  g=pg;
 }
 s->words[0161]=(agc_word)g;s->words[0154]=phase;
 if(cohort==3u||cohort==4u){
  s->words[0155]=(agc_word)(g*2u);s->words[0157]=CM_RR_SWRETURN;
  if(cohort==3u){s->words[0156]=(agc_word)(cm_restart_table_words[g*2u]+3u*phase);s->words[5]=CM_RR_PHSPART2;entry=CM_RR_PART2;}
  else{s->words[5]=CM_RR_GETPART2;entry=CM_RR_TYPEB_TABLE;}
 }
 /* Variable phases: the group's stored 2CADR, priority or delta. Waitlist
  * deltas are direct, indirect through 01112 or immediate; longcall deltas
  * name the DP time at 01110. */
 s->words[01436u+g*2u]=(agc_word)(01200u+(n&0377u));s->words[01437u+g*2u]=02003;
 v=(agc_word)(phase&7u);
 s->words[01054u+g*2u]=v>=3u?agc_negate(01110u):v==2u?((n&1u)?010000u:067777u):(n%3u)==0u?(agc_word)(n%300u):(n%3u)==1u?agc_negate(01112u):077777u;
 s->words[01110]=(agc_word)((n>>3)&3u);s->words[01111]=(agc_word)(n%12000u);s->words[01112]=(agc_word)(n%400u);
 if(cohort==5u){s->words[TIME2]=(agc_word)(037770u+(n&7u));s->words[TIME1]=(agc_word)(037700u+(n&077u));s->words[01053u+g*2u]=(agc_word)(040010u-(n&017u));}
 else{s->words[TIME2]=(agc_word)((n>>4)&3u);s->words[TIME1]=(agc_word)(n%10000u);}
 return entry;
}

static enum exec_status service(const agc_native_program *p,apollo_overlay_state *s,void *io){cm_rr_log *log=io;cm_rr_call *c;agc_word q=s->words[2];
 if(log->calls>=CM_RR_MAX_CALLS)return EXEC_OUT_OF_SCOPE;
 c=&log->call[log->calls++];c->service=s->words[5];c->a=s->words[0];c->l=s->words[1];c->q=q;
 if(!agc_transfer_operand(p,s,0,q,&c->cadr)||!agc_transfer_operand(p,s,0,(agc_word)(q+1u),&c->bbcon))return EXEC_OUT_OF_SCOPE;
 s->words[5]=(agc_word)(q+2u);return EXEC_OK;
}
static const agc_routine_entry services[4]={{FF(CM_RR_NOVAC),service},{FF(CM_RR_FINDVAC),service},{FF(CM_RR_WAITLIST),service},{FF(CM_RR_LONGCALL),service}};
enum exec_status cm_rr_compose(apollo_overlay_state *s,uint16_t n,cm_rr_log *log,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,cm_restart_operands,CM_RESTART_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_word superbank=0;
 tables[0]=cm_restart_routines;tables[1].entries=services;tables[1].count=4;program.basic=tables;program.basic_count=2;
 (void)cm_rr_select_input(s,cm_rr_compose_case(n));log->calls=0;
 return agc_transfer_run(&program,s,log,&superbank,FF(CM_RR_SWRETURN),32,steps);
}

/* The group operand's logical address under FB for NEWPHASE site k. */
static void newphase_site(unsigned k,agc_word *fb,agc_word *q){agc_site_operand o={0,0};unsigned i,found=0;
 for(i=0;i<CM_RESTART_OPERAND_COUNT;++i)if(cm_restart_operands[i].physical>=4096u&&found++==k){o=cm_restart_operands[i];break;}
 *fb=(agc_word)((o.physical/1024u)<<10);*q=(agc_word)(02000u+o.physical%1024u);
}
void cm_rr_newphase_input(apollo_overlay_state *s,uint16_t n){
 static const agc_word edge[8]={0,077777,1,077776,037777,040000,7,077770};
 agc_word fb,q,phase;unsigned kind=(n>>4)&3u;
 image(s,0x4114u+n);
 newphase_site(n%CM_NEWPHASE_SITE_COUNT,&fb,&q);
 s->words[3]=01400;s->words[4]=fb;s->words[6]=(agc_word)(fb|3u);s->words[5]=CM_RR_NEWPHASE;s->words[2]=q;
 /* Every sixteenth case is an edge phase; otherwise sign by kind. */
 if((n&15u)==0u)phase=edge[(n>>4)&7u];
 else{phase=(agc_word)(s->words[0]&037777u);if(kind==1u)phase=(agc_word)(phase&07u);if(kind>=2u)phase=agc_negate(phase);}
 s->words[0]=agc_extend(phase);
}
uint16_t cm_rr_newphase_return(const apollo_overlay_state *s){return agc_fixed_physical((agc_word)(s->words[2]+1u),s->words[4],0);}
enum exec_status cm_rr_newphase(apollo_overlay_state *s,uint16_t n,uint16_t *steps){
 agc_native_program program={&cm_restart_routines,1,0,0,cm_restart_operands,CM_RESTART_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_word superbank=0;uint16_t stop;
 cm_rr_newphase_input(s,n);stop=cm_rr_newphase_return(s);
 return agc_transfer_run(&program,s,0,&superbank,stop,4,steps);
}

static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
static void mix_image(const apollo_overlay_state *s){unsigned i;for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));}
uint32_t cm_rr_select_digest(apollo_overlay_state *s,uint16_t n){enum cm_rr_exit e=cm_restart_routines_select(s,cm_rr_select_input(s,n));
 sum_a=sum_b=0;mix((uint16_t)(e==CM_RR_SEL_TASK?1u:e==CM_RR_SEL_LONGCALL?2u:e==CM_RR_SEL_DISPLAY?3u:e==CM_RR_INVALID?4u:0u));mix_image(s);
 return (uint32_t)sum_b<<16|sum_a;
}
uint32_t cm_rr_compose_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){cm_rr_log log;uint16_t steps=0;unsigned i;
 *status=cm_rr_compose(s,n,&log,&steps);sum_a=sum_b=0;mix(log.calls);
 for(i=0;i<log.calls;++i){mix(log.call[i].service);mix(log.call[i].a);mix(log.call[i].l);mix(log.call[i].q);mix(log.call[i].cadr);mix(log.call[i].bbcon);}
 mix_image(s);return (uint32_t)sum_b<<16|sum_a;
}
uint32_t cm_rr_newphase_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){uint16_t steps=0;
 *status=cm_rr_newphase(s,n,&steps);sum_a=sum_b=0;mix_image(s);return (uint32_t)sum_b<<16|sum_a;
}
