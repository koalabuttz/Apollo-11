/* SPDX-License-Identifier: MIT */
#include "lm_burn_baby_cases.h"
#define W m->words
#define NEWOPS 06070u
#define FIXLOC 0120u
#define LOC 0164u
#define BANKSET 0165u
#define PH(b,a) (uint16_t)((b)*1024u+((a)-02000u))
typedef char lmbb_cases_check[LMBB_CASES==LMBB_ROUTINES*LMBB_STATES?1:-1];
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
static unsigned below(uint32_t *s,unsigned n){return (unsigned)((rnd(s)>>16)%n);}

static int in_code(uint16_t ph){
 return (ph>=PH(036u,02022u)&&ph<=PH(036u,03146u))||(ph>=PH(035u,03747u)&&ph<=PH(035u,03775u))||
        (ph>=PH(017u,02000u)&&ph<=PH(017u,02026u))||(ph>=PH(027u,02200u)&&ph<=PH(027u,02246u))||
        (ph>=06027u&&ph<=06036u);
}
int lmbb_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){
 uint16_t ph;
 /* A TC Q or TC P40/RET on its way, or the interpreter's erasable
  * instructions (the BUF2 / ADDRWD transfers, and UNIT's TC A). */
 if(z<02000u)return !(z==0u||z==2u||z==0133u||z==0137u||z==0141u||z==0156u||z==01142u);
 if(z==NEWOPS)return !in_code(agc_fixed_physical(loc,bankset,superbank));
 if(z>=06027u&&z<=06036u)return 0;   /* KILLTASK */
 if(z==07257u)return 1;               /* TPAGREE (STCLOK3's TC) */
 if(z>=06037u&&z<=07707u)return 0;   /* the interpreter */
 if(z>=04000u)return 1;
 ph=agc_fixed_physical(z,fb,superbank);
 if(in_code(ph))return 0;
 if(ph<=03734u-02000u)return 0;          /* the interpreter in bank 0 */
 if(ph>=1024u+0344u&&ph<=1024u+0600u)return 0;   /* and bank 1 */
 return 1;
}

static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){W[4]=fb;W[3]=eb;W[6]=(agc_word)(fb|(eb>>8));}
/* A word of switched erasable bank e at its 01400-01777 address. */
static void put(apollo_overlay_state *m,unsigned e,unsigned a,agc_word v){W[e*0400u+a-01400u]=v;}
/* A transfer destination the original and the routines agree on: a word of
 * switched fixed or fixed-fixed below the interpreter. */
static agc_word dest(uint32_t *seed){return (agc_word)(02000u+below(seed,04027u));}
/* A negative word, -0 in a quarter. */
static agc_word neg(uint32_t *seed){return below(seed,4u)?(agc_word)(040000u|r15(seed)):077777u;}
static const agc_word tables[6]={02022u,02036u,02046u,02061u,02076u,02112u};
/* DISPDEX: +0, a count, or minus a display index (-CNTDNDEX, -VB97DEX,
 * -VB99DEX, -BLANKDEX and the -25 V06N61 entry) or -0. */
static const agc_word dispdex[8]={0u,3u,077777u-035u,077777u-025u,077777u-017u,077777u-013u,077777u-2u,077777u};
static const agc_word p40ret[3]={03014u,03057u,03074u};

void lmbb_input(apollo_overlay_state *m,lmbb_case *c,uint16_t n){
 uint32_t seed=0x6262u+(uint32_t)n*31u;unsigned i,v,r=n%LMBB_ROUTINES,bank;uint16_t ph=lmbb_routine_keys[r];
 agc_word eb,vac,fb,q;
 v=n/LMBB_ROUTINES;
 for(i=0;i<2048u;++i)W[i]=r15(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 /* The drive counters yaAGC empties as it runs (CDU and optics drives,
  * gyro, thrust and altimeter outputs) start +0. */
 W[035]=W[036]=W[047]=0;for(i=050u;i<=054u;++i)W[i]=0;
 /* Overflow in A at the entry in an eighth of the states. */
 if(v%8u==5u)W[0]=(agc_register)(below(&seed,2u)?040000u|r15(&seed):0100000u|(r15(&seed)&037777u));
 c->interrupt_enabled=(uint16_t)(v&1u);
 c->superbank=(agc_word)(((v>>1)%4u)<<4);
 c->chan11=r15(&seed);c->chan14=r15(&seed);c->chan30=r15(&seed);
 bank=ph==06027u?0u:ph/1024u;
 /* KILLTASK: FB and Q of a TC KILLTASK (seven eighths), else any word. */
 if(ph==06027u){unsigned k=below(&seed,LMBB_SITES);uint16_t p=lm_burn_baby_sites[k].physical;
  fb=(agc_word)((p/1024u)<<10);q=(agc_word)(02000u+p%1024u);
  if(v%8u==7u){fb=(agc_word)(below(&seed,044u)<<10);q=dest(&seed);}
  /* The CADR in A at KILLTSK2 for the planted list entry below. */
  W[0]=lm_burn_baby_sites[k].word;}
 else{fb=(agc_word)(bank<<10);q=dest(&seed);}
 /* EBANK= WHICH (E7) as the file runs, E3 at KILLTSK2 (KILLBB); another
  * bank in a sixteenth of the states. */
 eb=bank==027u?01400u:03400u;
 if(v%16u==9u)eb=(agc_word)(below(&seed,8u)<<8);
 set_banks(m,fb,eb);
 W[2]=q;
 /* NEWJOB +0 (a job waiting in a fifth of the states). */
 W[067]=(v>>4)%5u==4u?(agc_word)(below(&seed,0400u)+1u):0;
 /* A work area of one of the five VAC areas, the pushdown pointer low in
  * it, MODE one of the three, EDOP +0. */
 vac=(agc_word)(0401u+054u*below(&seed,5u));W[FIXLOC]=vac;W[0166]=(agc_word)(vac+below(&seed,020u));
 W[0163]=(agc_word)(v%3u==0u?0u:v%3u==1u?1u:077776u);W[023]=0;
 /* WHICH one of the six tables (fifteen sixteenths). */
 put(m,7u,01455u,v%16u==3u?r15(&seed):tables[below(&seed,6u)]);
 /* PHASE3, PHASE5, PHASE6 +0 or positive (seven eighths each). */
 W[0757]=v%8u==2u?neg(&seed):below(&seed,2u)?r15(&seed)&037777u:0u;
 W[0763]=v%8u==4u?neg(&seed):below(&seed,2u)?r15(&seed)&037777u:0u;
 W[0765]=v%8u==6u?neg(&seed):below(&seed,2u)?r15(&seed)&037777u:0u;
 /* DISPDEX one of the indexes (seven eighths), P40/RET one of NULLCLOK's
  * returns (fifteen sixteenths). */
 if(v%8u!=1u)W[01163]=dispdex[below(&seed,8u)];
 if(v%16u!=11u)W[01142]=p40ret[below(&seed,3u)];
 /* KILLTSK2: L counts the list pairs (0, 2, .. 16), ITEMP1 the return,
  * and in half the states the list holds the CADR's task. In a sixteenth
  * L is any word, as a DTCB into the file can leave it (#147). */
 W[061]=dest(&seed);
 if(bank==027u&&ph!=PH(027u,02200u))W[1]=v%16u==13u?agc_extend(r15(&seed)):(agc_word)(2u*below(&seed,9u));
 /* The caller's BBANK (L at KILLTSK2, ITEMP2 after it): a bank of
  * switched fixed (FB 2 .. 37, any EB). */
 {agc_word bb=(agc_word)(((2u+below(&seed,036u))<<10)|below(&seed,8u));
  if(ph==PH(027u,02200u))W[1]=agc_extend(bb);else W[062]=bb;}
 if((ph==06027u||bank==027u)&&(v&2u)){agc_word cadr=ph==06027u?W[0]:ph==PH(027u,02200u)?(agc_word)(W[0]&077777u):(agc_word)(W[063]|(W[064]&01777u));
  unsigned k=2u*below(&seed,9u);
  if(ph!=06027u&&ph!=PH(027u,02200u)){W[064]=(agc_word)(02000u|(W[064]&01777u));W[063]&=076000u;cadr=(agc_word)(W[063]|(W[064]&01777u));}
  W[01410u+k]=(agc_word)(02000u|(cadr&01777u));W[01411u+k]=(agc_word)((cadr&076000u)|(r15(&seed)&01777u));}
 /* P40A/P's return from G+N,AUTO: A +0, -0, positive or negative, and
  * APSFLAG clear in half the states. */
 if(ph==PH(035u,03753u)){static const agc_register a[4]={0u,0177777u,1u,0177770u};
  W[0]=below(&seed,2u)?a[below(&seed,4u)]:W[0];if(v&4u)W[0106]&=(agc_word)~010000u;}
 if(r<LMBB_BASIC_ROUTINES){W[5]=ph==06027u?ph:(agc_word)(02000u+ph%1024u);return;}
 /* Interpretive keys: at NEWOPS with LOC there, BANKSET the key's bank. */
 W[5]=NEWOPS;W[LOC]=(agc_word)(02000u+ph%1024u);W[BANKSET]=W[6];W[0115]=040000u;
}

#ifndef LMBB_UNDO_ADDRESS
static apollo_overlay_state undo_image;
#define LMBB_UNDO (&undo_image)
#else
#define LMBB_UNDO ((apollo_overlay_state *)LMBB_UNDO_ADDRESS)
#endif
unsigned lmbb_run(apollo_overlay_state *s,uint16_t n,lmbb_case *out){
 lmbb_case c;lm_bb_io io;agc_routine_table basic[1],interp[1];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;
 lmbb_input(s,&c,n);
 io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;
 io.chan11=c.chan11;io.chan14=c.chan14;io.chan30=c.chan30;io.undo=LMBB_UNDO;
 basic[0]=lm_burn_baby_routines;interp[0]=lm_burn_baby_interpretive;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+NEWOPS-04000u);program.loc=LOC;program.bankset=BANKSET;
 call=agc_transfer_resolve(&program,s,c.superbank);
 if(!call)return 3u;
 st=call(&program,s,&io);
 if(out){*out=c;out->interrupt_enabled=io.control.interrupt_enabled;out->chan11=io.chan11;out->chan14=io.chan14;out->chan30=io.chan30;}
 if(st==EXEC_OK)return LMBB_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LMBB_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lmbb_digest(apollo_overlay_state *s,uint16_t n){lmbb_case c;unsigned i,kind=lmbb_run(s,n,&c);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LMBB_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
  mix(c.chan11);mix(c.chan14);mix(c.chan30);mix(c.interrupt_enabled);}
 return (uint32_t)sum_b<<16|sum_a;
}
