/* SPDX-License-Identifier: MIT */
#include "lm_lunar_landing_cases.h"
#define W m->words
#define NEWOPS 06070u
#define FIXLOC 0120u
#define LOC 0164u
#define BANKSET 0165u
typedef char lld_cases_check[LLD_CASES==LLD_ROUTINES*LLD_STATES?1:-1];
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
static unsigned below(uint32_t *s,unsigned n){return (unsigned)((rnd(s)>>16)%n);}

static int in_code(uint16_t ph){
 return (ph>=031u*1024u+0144u&&ph<=031u*1024u+0214u)||(ph>=032u*1024u+0776u&&ph<=032u*1024u+01252u);
}
int lld_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){
 uint16_t ph;
 /* A TC Q on its way, or the interpreter's erasable instructions (the
  * BUF2 / ADDRWD transfers, UNIT's TC A and TPAGREE's TC L). */
 if(z<02000u)return !(z==0u||z==1u||z==2u||z==0133u||z==0137u||z==0141u||z==0156u);
 if(z==NEWOPS)return !in_code(agc_fixed_physical(loc,bankset,superbank));
 if(z>=04621u&&z<=04630u)return 0;   /* SWCALL, after RTB */
 if(z>=06037u&&z<=07707u)return 0;   /* the interpreter */
 if(z>=04000u)return 1;
 ph=agc_fixed_physical(z,fb,superbank);
 if(in_code(ph))return 0;
 if(ph<=03734u-02000u)return 0;          /* the interpreter in bank 0 */
 if(ph>=1024u+0344u&&ph<=1024u+0600u)return 0;   /* and bank 1 */
 return 1;
}

static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){W[4]=fb;W[3]=eb;W[6]=(agc_word)(fb|(eb>>8));}

void lld_input(apollo_overlay_state *m,lld_case *c,uint16_t n){
 uint32_t seed=0x6c4du+(uint32_t)n*29u;unsigned i,v,r=n%LLD_ROUTINES;uint16_t ph=lld_routine_keys[r];
 agc_word eb,vac,fb=ph>=032u*1024u?LM_LLD_FB32:LM_LLD_FB31;
 v=n/LLD_ROUTINES;
 for(i=0;i<2048u;++i)W[i]=r15(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 c->interrupt_enabled=(uint16_t)(v&1u);
 c->superbank=(agc_word)(((v>>1)%4u)<<4);
 c->chan12=r15(&seed);c->chan33=(v>>3)&1u?(agc_word)(r15(&seed)|040u):(agc_word)(r15(&seed)&~040u);
 /* EBANK= E2DPS (E7) as the file runs, another bank in a quarter of the
  * states. */
 eb=(v>>4)%4u==3u?(agc_word)(below(&seed,8u)<<8):03400u;
 set_banks(m,fb,eb);
 /* NEWJOB +0 (a job waiting in a fifth of the states). */
 W[067]=(v>>6)%5u==4u?(agc_word)(below(&seed,0400u)+1u):0;
 /* A work area of one of the five VAC areas, the pushdown pointer low in
  * it, MODE one of the three. */
 vac=(agc_word)(0401u+054u*below(&seed,5u));W[FIXLOC]=vac;W[0166]=(agc_word)(vac+below(&seed,020u));
 W[0163]=(agc_word)(v%3u==0u?0u:v%3u==1u?1u:077776u);
 /* DDUMCALC: in three sevenths of the states the ignition algorithm's
  * guesses converge (gains, RGU, RIGNX/Z and VGU's X and Y zero, so DDUM
  * is +0 and BMN takes DDUMGOOD), V zero in a third of those (UNIT's
  * alarm). P63SPOT2: R60VSAVE zero in a quarter (UNIT's alarm). */
 if(ph==032u*1024u+01102u&&v%7u<3u){
  for(i=0;i<6u;++i)W[02500u+i]=W[02636u+i]=0;
  W[02474u]=W[02475u]=W[02476u]=W[02477u]=0;
  W[03626u]=W[03627u]=W[03630u]=W[03631u]=0;
  if(v%3u==2u)for(i=0;i<6u;++i)W[03526u+i]=0;}
 if(ph==032u*1024u+01215u&&v%4u==1u)for(i=0;i<6u;++i)W[02630u+i]=0;
 /* ABS DSU DDUMCRIT; BMN at 32,3141: |DDUM| exactly DDUMCRIT (2DEC +8 B-28),
  * either sign, in DP mode in a twelfth of the states, for BMN's zero outcome. */
 if(ph==032u*1024u+01141u&&v%12u==0u){int neg=below(&seed,2u)!=0;W[0154]=neg?077777u:0u;W[0155]=neg?077767u:010u;W[0156]=0u;}
 if(r<LLD_BASIC_ROUTINES){W[5]=(agc_word)(02000u+ph%1024u);return;}
 /* Interpretive keys: at NEWOPS with LOC there, BANKSET the key's bank. */
 W[5]=NEWOPS;W[LOC]=(agc_word)(02000u+ph%1024u);W[BANKSET]=W[6];W[0115]=040000u;W[023]=0;
}

#ifndef LLD_UNDO_ADDRESS
static apollo_overlay_state undo_image;
#define LLD_UNDO (&undo_image)
#else
#define LLD_UNDO ((apollo_overlay_state *)LLD_UNDO_ADDRESS)
#endif
unsigned lld_run(apollo_overlay_state *s,uint16_t n,lld_case *out){
 lld_case c;lm_lld_io io;agc_routine_table basic[1],interp[1];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;
 lld_input(s,&c,n);
 io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;io.chan12=c.chan12;io.chan33=c.chan33;io.undo=LLD_UNDO;
 basic[0]=lm_lunar_landing_routines;interp[0]=lm_lunar_landing_interpretive;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+NEWOPS-04000u);program.loc=LOC;program.bankset=BANKSET;
 call=agc_transfer_resolve(&program,s,c.superbank);
 if(!call)return 3u;
 st=call(&program,s,&io);
 if(out){*out=c;out->interrupt_enabled=io.control.interrupt_enabled;out->chan12=io.chan12;out->chan33=io.chan33;}
 if(st==EXEC_OK)return LLD_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LLD_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lld_digest(apollo_overlay_state *s,uint16_t n){lld_case c;unsigned i,kind=lld_run(s,n,&c);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LLD_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
  mix(c.chan12);mix(c.chan33);mix(c.interrupt_enabled);}
 return (uint32_t)sum_b<<16|sum_a;
}
