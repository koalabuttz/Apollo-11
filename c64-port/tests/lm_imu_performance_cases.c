/* SPDX-License-Identifier: MIT */
#include "lm_imu_performance_cases.h"
#define W m->words
#define NEWOPS 06070u
#define FIXLOC 0120u
#define LOC 0164u
#define BANKSET 0165u
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
static unsigned below(uint32_t *s,unsigned n){return (unsigned)((rnd(s)>>16)%n);}
/* A word of one of four CCS classes. */
static agc_word ccs_class(uint32_t *seed,unsigned k){agc_word v=(agc_word)(below(seed,037776u)+1u);
 return k==0u?v:k==1u?0u:k==2u?(agc_word)(~v&077777u):077777u;
}
/* A small CCS class: 1..n, +0, -1..-n, -0. */
static agc_word small_class(uint32_t *seed,unsigned k,unsigned n){agc_word v=(agc_word)(below(seed,n)+1u);
 return k==0u?v:k==1u?0u:k==2u?(agc_word)(~v&077777u):077777u;
}

int ipt_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){
 uint16_t ph;
 if(z<02000u)return !(z==2u||z==0133u||z==0137u||z==0141u||z==0156u||z==01417u||z==01420u||z==01571u);
 if(z==NEWOPS){ph=agc_fixed_physical(loc,bankset,superbank);return !(ph>=037u*1024u+021u&&ph<=037u*1024u+01032u);}
 if(z>=04102u&&z<=04123u)return 0;
 if(z>=04621u&&z<=04630u)return 0;   /* SWCALL, after RTB */
 if(z>=06037u&&z<=07707u)return 0;
 if(z>=04000u)return 1;
 ph=agc_fixed_physical(z,fb,superbank);
 if(ph>=037u*1024u+2u&&ph<=037u*1024u+01053u)return 0;
 if(ph<=03734u-02000u)return 0;
 if(ph>=1024u+0344u&&ph<=1024u+0600u)return 0;
 return 1;
}

int ipt_idle(agc_word z,agc_word prev){return (z==02335u&&prev==02352u)||(z==04103u&&prev==04117u);}

#if !(defined IPT_C64_PART && IPT_C64_PART>0)
unsigned ipt_part;
int ipt_in(agc_word loc){return IPT_P_IN(ipt_part,loc);}
int ipt_basic_in(agc_word a){return IPT_P_BASIC_IN(ipt_part,ipt_block(a));}
#endif
agc_word ipt_block(agc_word a){
 static const agc_word heads[]={02002u,02004u,02027u,02067u,02101u,02117u,02126u,02151u,02166u,02226u,02270u,02301u,
  02332u,02367u,02447u,02457u,02500u,02536u,02577u,02773u,03025u,03034u,04102u};
 unsigned i=sizeof heads/sizeof *heads;
 while(--i&&heads[i]>a){}
 return heads[i];
}
static int search(const agc_routine_table *t,uint16_t ph){unsigned lo=0,hi=t->count;
 while(lo<hi){unsigned mid=(lo+hi)/2u;if(t->entries[mid].physical<ph)lo=mid+1u;else hi=mid;}
 return lo<t->count&&t->entries[lo].physical==ph;
}
int ipt_is_key(uint16_t ph){return search(&lm_imu_performance_routines,ph)||search(&lm_imu_performance_interpretive,ph);}
int ipt_part_has(uint16_t ph){agc_word a;
 if(search(&lm_imu_performance_interpretive,ph))return IPT_IN((agc_word)(02000u+ph%1024u));
 a=(agc_word)(ph>=2048u&&ph<3072u?04000u+ph-2048u:02000u+ph%1024u);
 a=ipt_block(a);
 return IPT_P_BASIC_IN(ipt_part,a);
}

static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){W[4]=fb;W[3]=eb;W[6]=(agc_word)(fb|(eb>>8));}
/* Physical word of an EBANK 5 window address (the image is indexed
 * physically: E5 is 02400-02777). */
#define E5(a) ((a)+01000u)
#define KEY(a) (ph==(uint16_t)(037u*1024u+((a)-02000u)))
#define FFKEY(a) (ph==(uint16_t)(2048u+((a)-04000u)))

void ipt_input(apollo_overlay_state *m,ipt_case *c,uint16_t n){
 uint32_t seed=0x1d07ul+(uint32_t)n*13u;unsigned i,v,r=n%IPT_ROUTINES;uint16_t ph=ipt_routine_keys[r];
 agc_word eb,vac;
 v=n/IPT_ROUTINES;
 for(i=0;i<2048u;++i)W[i]=r15(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 c->interrupt_enabled=(uint16_t)(v&1u);
 c->superbank=(agc_word)(((v>>1)%4u)<<4);
 /* LOSCALAR 37777 (FINETIME's retry) in a quarter of the states. */
 c->hiscalar=r15(&seed);c->loscalar=below(&seed,4u)?r15(&seed):037777u;
 eb=(v>>3)%4u==3u?(agc_word)(below(&seed,8u)<<8):02400u;
 set_banks(m,076000u,eb);
 /* NEWJOB +0, or a job waiting in a fifth of the states (always at
  * CHECKG's CHANG1 return and the PIPA tasks; half the CHECKG1 states). */
 W[067]=below(&seed,5u)==4u||KEY(02342)||KEY(02166)||KEY(02151)||(KEY(02335)&&below(&seed,2u))?(agc_word)(below(&seed,0400u)+1u):0;
 /* The erasable returns: genuine call sites or a caller outside (04000). */
 {static const agc_word qplace[7]={02144u,02200u,02126u,02231u,02270u,04000u,04000u};
  static const agc_word qplaces[5]={02131u,02171u,02252u,03037u,04000u};
  static const agc_word zerondx[7]={02005u,02101u,02104u,02230u,02456u,03025u,04000u};
  W[E5(01417)]=qplace[below(&seed,7u)];W[E5(01420)]=qplaces[below(&seed,5u)];
  if(eb==02400u)W[E5(01571)]=zerondx[below(&seed,7u)];
 }
 /* Small indexes: POSITON (SOUTHDR), PIPINDEX (PIPAX), RESULTCT (DATAPL). */
 if(eb==02400u){
  W[E5(01416)]=(agc_word)below(&seed,8u);
  W[E5(01415)]=below(&seed,8u)==7u?small_class(&seed,2u+below(&seed,2u),2u):(agc_word)below(&seed,3u);
  W[E5(01537)]=(agc_word)below(&seed,6u);
  /* CCS words, each class drawn on its own: NDXCTR, LENGTHOT, TORQNDX,
   * ALTIM, GEOCOMPS, DATAPL +1, OVFLOWCK. */
  W[E5(01414)]=below(&seed,2u)?small_class(&seed,below(&seed,4u),3u):(agc_word)below(&seed,2u);
  W[E5(01412)]=ccs_class(&seed,below(&seed,4u));
  W[E5(01440)]=ccs_class(&seed,below(&seed,4u));
  W[E5(01530)]=ccs_class(&seed,below(&seed,4u));
  W[E5(01562)]=small_class(&seed,below(&seed,3u),2u);
  W[E5(01473)]=ccs_class(&seed,below(&seed,4u));
  if(below(&seed,2u))W[E5(01576)]=0;
  /* ALX1S +0 or -(12..132), CMPX1 +-0 (flight: ALX1S 144, then 132 down to
   * 0 by 12, CMPX1 +-1; the review's probe with those gave the same outcomes). */
  {agc_word x=(agc_word)(12u*below(&seed,12u));W[E5(01444)]=x?(agc_word)(~x&077777u):0u;}
  W[E5(01445)]=(rnd(&seed)>>17)&1u?077777u:0u;
  /* CHECKG1: PIPAX unchanged since CHECKG's first read (ZERONDX holds
   * its complement) in half the states. */
  if(KEY(02335)&&below(&seed,2u)){agc_word x=W[E5(01415)]==077777u?0u:W[E5(01415)];
   if(x<3u)W[E5(01571)]=(agc_word)(~W[037u+x]&077777u);}
 }
 /* A work area of one of the five VAC areas, the pushdown pointer inside
  * it, small index registers and steps (the basic code reaches the
  * interpreter through TC INTPRET). */
 vac=(agc_word)(0401u+054u*below(&seed,5u));W[FIXLOC]=vac;W[0166]=(agc_word)(vac+below(&seed,040u));
 W[0163]=(agc_word)(v%3u==0u?0u:v%3u==1u?1u:077776u);
 for(i=0;i<2u;++i){agc_word x=(agc_word)below(&seed,040u);W[vac+046u+i]=(rnd(&seed)>>17)&1u?x:(agc_word)(~x&077777u);}
 W[vac+050u]=(agc_word)(below(&seed,6u)+1u);W[vac+051u]=(agc_word)(below(&seed,6u)+1u);
 if(r<IPT_BASIC_ROUTINES){
  /* FINETIME keys return to CHECKG (02361) or, at FINETIME +1, also to
   * SCALPREP (bank 37, 03717). */
  if(ph>=2048u&&ph<3072u){W[5]=(agc_word)(04000u+ph-2048u);W[2]=FFKEY(04103)&&below(&seed,2u)?03717u:02361u;}
  else W[5]=(agc_word)(02000u+ph%1024u);
  return;
 }
 /* Interpretive keys: at NEWOPS with LOC there, BANKSET bank 37 and
  * genuine return CADRs. */
 W[5]=NEWOPS;W[LOC]=(agc_word)(02000u+ph%1024u);W[BANKSET]=W[6];W[0115]=040000u;W[023]=0;
 {agc_word genuine[3];
  genuine[0]=IPT_QPRET_ERTHRVSE_2532;genuine[1]=IPT_QPRET_ERTHRVSE_3030;genuine[2]=IPT_QPRET_EARTHR_2452;
  if(KEY(02414)||KEY(02416)||KEY(02417))W[vac+052u]=(v>>2)%4u==3u?(agc_word)(04000u|r15(&seed)):genuine[(v>>2)%2u];
  if(KEY(02423)||KEY(02424)||KEY(02427)||KEY(02431)||KEY(02434)||KEY(02437)||KEY(02441)||KEY(02442)||KEY(02445)){
   W[vac+051u]=(v>>2)%4u==3u?(agc_word)(04000u|r15(&seed)):genuine[2];W[vac+052u]=genuine[2];}
  /* OVERFFIX's RVQ: CALL OVERFFIX at 02214 (back to AINGOTN) or 02427. */
  if(KEY(02275)||KEY(02300))W[vac+052u]=(v>>2)%4u==3u?(agc_word)(04000u|r15(&seed)):(v>>2)%2u?076431u:076217u;
 }
}

#ifndef IPT_UNDO_ADDRESS
static apollo_overlay_state undo_image;
#define IPT_UNDO (&undo_image)
#else
#define IPT_UNDO ((apollo_overlay_state *)IPT_UNDO_ADDRESS)
#endif
unsigned ipt_run(apollo_overlay_state *s,uint16_t n,uint16_t *ie){
 ipt_case c;lm_ipt_io io;agc_routine_table basic[1],interp[1];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;
 ipt_input(s,&c,n);
 io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;io.hiscalar=c.hiscalar;io.loscalar=c.loscalar;io.undo=IPT_UNDO;
 basic[0]=lm_imu_performance_routines;interp[0]=lm_imu_performance_interpretive;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+NEWOPS-04000u);program.loc=LOC;program.bankset=BANKSET;
 call=agc_transfer_resolve(&program,s,c.superbank);
 if(!call)return 3u;
 st=call(&program,s,&io);
 if(ie)*ie=io.control.interrupt_enabled;
 if(st==EXEC_OK)return IPT_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return IPT_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t ipt_digest(apollo_overlay_state *s,uint16_t n){uint16_t ie=0;unsigned i,kind=ipt_run(s,n,&ie);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=IPT_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));mix(ie);}
 return (uint32_t)sum_b<<16|sum_a;
}
