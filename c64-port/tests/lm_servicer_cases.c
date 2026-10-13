/* SPDX-License-Identifier: MIT */
#include "lm_servicer_cases.h"
#include "lm_servicer_chains.h"
#define W m->words
#define NEWOPS 06070u
#define FIXLOC 0120u
#define LOC 0164u
#define BANKSET 0165u
#define PH(b,a) (uint16_t)((b)*1024u+((a)-02000u))
#define KEY(b,a) (ph==PH(b,a))
#define E7(a) (07u*0400u+(a)-01400u)
typedef char lmsv_cases_check[LMSV_BASE_CASES==LMSV_ROUTINES*LMSV_STATES&&
                              LMSV_CASES==LMSV_BASE_CASES+LMSV_CHAINS?1:-1];
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
static unsigned below(uint32_t *s,unsigned n){return (unsigned)((rnd(s)>>16)%n);}

static int in_code(uint16_t ph){
 return (ph>=PH(021u,02130u)&&ph<=PH(021u,02153u))||(ph>=PH(022u,03661u)&&ph<=PH(022u,03710u))||
        (ph>=PH(023u,02461u)&&ph<=PH(023u,02510u))||(ph>=PH(027u,03321u)&&ph<=PH(027u,03357u))||
        (ph>=PH(032u,03770u)&&ph<=PH(032u,03772u))||(ph>=PH(033u,02206u)&&ph<=PH(033u,03776u))||
        (ph>=PH(034u,03716u)&&ph<=PH(034u,03774u))||(ph>=PH(037u,03410u)&&ph<=PH(037u,03654u));
}
int lmsv_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word q,agc_word loc,agc_word bankset){
 uint16_t ph;
 /* A TC Q, TC A or TC LRADRET on its way, or the interpreter's erasable
  * instructions (the BUF2 / ADDRWD transfers, UNIT's TC A and ARGZERO's TC
  * L). */
 if(z<02000u)return !(z==0u||z==1u||z==2u||z==0133u||z==0137u||z==0141u||z==0156u||z==01650u);
 if(z==NEWOPS)return !in_code(agc_fixed_physical(loc,bankset,superbank));
 /* The interpreter's subroutines the file's basic code calls (SHORTMP,
  * SHORTMP2, ALSIGNAG, BRANCH and BRANCH +4), when called from the file. */
 if((z==07307u||z==07313u||z==07544u||z==06723u||z==06727u)&&q>=02000u&&q<=03777u&&
    in_code(agc_fixed_physical(q,fb,superbank)))return 1;
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
/* A word of switched erasable bank e at its 01400-01777 address. */
static void put(apollo_overlay_state *m,unsigned e,unsigned a,agc_word v){W[e*0400u+a-01400u]=v;}
/* A transfer destination the original and the routines agree on: a word of
 * switched fixed or fixed-fixed below the interpreter. */
static agc_word dest(uint32_t *seed){return (agc_word)(02000u+below(seed,04027u));}
/* A return into the file: the word after one of its TCs of a subroutine
 * that returns through Q (GNUFAZE5, QUIKFAZ5, COPYCYC, PIPASR, TMPTOSPT,
 * GNURVST, SETPOS). */
static const uint16_t returns[]={PH(037u,03412u),PH(037u,03417u),PH(037u,03451u),PH(037u,03453u),PH(033u,02261u),
 PH(033u,02275u),PH(033u,02353u),PH(033u,02411u),PH(033u,03053u),PH(033u,03065u),PH(033u,03306u),PH(033u,03311u),
 PH(033u,03512u),PH(033u,03515u),PH(033u,02547u),PH(033u,03700u),PH(033u,03735u),PH(033u,02422u),PH(033u,03310u),
 PH(033u,03514u),PH(033u,02276u)};
/* The EBANK each key runs under: E7 (EBANK= DVCNTR) but RDGIMS (E4), the
 * radar's VELUPDAT after QUICTRIG (E4) and USEVF/WSTOR (E5). */
static unsigned ebank_of(uint16_t ph){
 if(ph==PH(034u,03757u)||ph==PH(033u,03336u)||ph==PH(033u,03344u))return 4u;
 if(ph==PH(033u,03466u)||ph==PH(033u,03472u))return 5u;
 return 7u;
}

unsigned lmsv_slot(uint16_t n){return n<LMSV_BASE_CASES?n%LMSV_ROUTINES:lmsv_chain_slot[n-LMSV_BASE_CASES];}

void lmsv_input(apollo_overlay_state *m,lmsv_case *c,uint16_t n){
 uint32_t seed=0x5356u+(uint32_t)n*2654435761u;unsigned i,v,r=n%LMSV_ROUTINES,bank;uint16_t ph=lmsv_routine_keys[r];
 agc_word eb,vac,q;
 if(n>=LMSV_BASE_CASES){unsigned j=n-LMSV_BASE_CASES;
  lmsv_input(m,c,lmsv_chain_base[j]);
  if(lmsv_chain_edits[j])for(i=0;i<lmsv_chain_count[j];++i)W[lmsv_chain_edits[j][i][0]]=lmsv_chain_edits[j][i][1];
  c->chan11=lmsv_chain_io[j][0];c->chan12=lmsv_chain_io[j][1];c->chan32=lmsv_chain_io[j][2];
  c->chan33=lmsv_chain_io[j][3];c->interrupt_enabled=lmsv_chain_io[j][4];return;}
 v=n/LMSV_ROUTINES;
 for(i=0;i<2048u;++i)W[i]=r15(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 /* The drive counters yaAGC empties as it runs (CDU and optics drives,
  * gyro, thrust and altimeter outputs) start +0. */
 W[035]=W[036]=W[047]=0;for(i=050u;i<=054u;++i)W[i]=0;
 /* Overflow in A at the entry in an eighth of the states. */
 if(v%8u==5u)W[0]=(agc_register)(below(&seed,2u)?040000u|r15(&seed):0100000u|(r15(&seed)&037777u));
 c->interrupt_enabled=(uint16_t)(v&1u);
 c->superbank=(agc_word)(((v>>1)%4u)<<4);
 c->chan11=r15(&seed);c->chan12=r15(&seed);c->chan32=r15(&seed);c->chan33=r15(&seed);
 bank=ph/1024u;
 /* Q a return into the file in half the states, else any fixed word. */
 if(v&4u){uint16_t p=returns[below(&seed,sizeof returns/sizeof returns[0])];q=(agc_word)(02000u+p%1024u);
  if(p/1024u!=bank)q=dest(&seed);}
 else q=dest(&seed);
 eb=(agc_word)(ebank_of(ph)<<8);
 if(v%16u==9u)eb=(agc_word)(below(&seed,8u)<<8);
 set_banks(m,(agc_word)(bank<<10),eb);
 W[2]=q;
 /* NEWJOB +0 (a job waiting in a fifth of the states). */
 W[067]=(v>>4)%5u==4u?(agc_word)(below(&seed,0400u)+1u):0;
 /* A work area of one of the five VAC areas, the pushdown pointer low in
  * it, MODE one of the three, EDOP +0. */
 vac=(agc_word)(0401u+054u*below(&seed,5u));W[FIXLOC]=vac;W[0166]=(agc_word)(vac+below(&seed,020u));
 W[0163]=(agc_word)(v%3u==0u?0u:v%3u==1u?1u:077776u);W[023]=0;
 /* The indexes (fifteen sixteenths): ITEMP1 an even count up to 20,
  * VSELECT and VSELECT* 0, 1 or 2, BUF 0 or 6, LRADRET a return. */
 if(v%16u!=3u){W[061]=(agc_word)(2u*below(&seed,11u));W[0131]=(agc_word)below(&seed,3u);
  put(m,7u,01651u,(agc_word)below(&seed,3u));W[0130]=(agc_word)(6u*below(&seed,2u));
  put(m,7u,01650u,(agc_word)(02000u+returns[below(&seed,sizeof returns/sizeof returns[0])]%1024u));}
 /* X1 and X2 of the work area as their producers leave them (fifteen
  * sixteenths): VELUPDAT's -6 VSELECT and -2 VSELECT past its BANKCALL and
  * CALL (-0 for VSELECT +0, as CS VSELECT; ADS L leaves it), and
  * CALCGRAV's LXC,1 RTX2 (-0 or -2) at CALCGRV1. */
 if(v%16u!=3u){
  if(KEY(033u,03336u)||KEY(033u,03344u)){W[vac+046u]=(agc_word)(~(6u*W[0131])&AGC_MASK);
   W[vac+047u]=(agc_word)(~(2u*W[0131])&AGC_MASK);}
  if(KEY(033u,02765u))W[vac+046u]=(agc_word)(~(2u*below(&seed,2u))&AGC_MASK);}
 /* A at ALTCHK (2 or 6), at SETPOS (0 or 2) and at GNURVST and its two
  * callers (0 or 6). */
 if(v%8u!=5u){
  if(KEY(033u,02664u))W[0]=(agc_word)(2u+4u*below(&seed,2u));
  if(KEY(033u,03740u))W[0]=(agc_word)(2u*below(&seed,2u));
  if(KEY(033u,03307u)||KEY(033u,03513u)||KEY(033u,03534u))W[0]=(agc_word)(6u*below(&seed,2u));}
 /* The PHASE words +0 or positive (three quarters each). */
 for(i=0751u;i<=0765u;i+=2u)if(below(&seed,4u))W[i]=below(&seed,2u)?r15(&seed)&037777u:0u;
 /* The values the file's CCS and BZF tests single out, each in a quarter
  * of the states: PIPAGE, DELVZ, DELVY, TEMZ and TEMX +0 or -0 (REREADAC),
  * DVCNTR +0 (LOTHRUST), STILBADH and STILBADV +0 (the radar jobs), PHASE2
  * and PHASE4 -0, TIME5 at READACCS's +0 and -0, the radar counters LRMCTR and LRLCTR
  * at most 3 past LRSCTR and LRRCTR and those +0 (VFAIL, HFAIL), RTX2 and
  * the word SLOAD RTX2 reads +0 (CALCGRAV in earth orbit; 2, the moon, in
  * half the states). */
 {static const unsigned zeros[]={01257u,01254u,01255u,01256u,0330u,0326u},e7zeros[]={01515u,01674u,01675u};
  for(i=0;i<sizeof zeros/sizeof zeros[0];++i)if(!below(&seed,4u))W[zeros[i]]=below(&seed,2u)?0u:077777u;
  for(i=0;i<sizeof e7zeros/sizeof e7zeros[0];++i)if(!below(&seed,4u))put(m,7u,e7zeros[i],below(&seed,2u)?0u:077777u);
  if(!below(&seed,4u))W[0755]=077777u;
  if(!below(&seed,4u))W[0761]=077777u;
  if(!below(&seed,4u))W[030]=(agc_word)(037770u+below(&seed,3u));
  if(!below(&seed,4u)){put(m,7u,01671u,0u);put(m,7u,01673u,0u);}
  {unsigned t=below(&seed,4u);if(t<3u)W[02777]=W[03000]=(agc_word)(t==0u?0u:2u);}
  if(!below(&seed,4u))put(m,7u,01672u,(agc_word)((W[E7(01673u)]+below(&seed,4u))&AGC_MASK));
  if(!below(&seed,4u))put(m,7u,01670u,(agc_word)((W[E7(01671u)]+below(&seed,4u))&AGC_MASK));}
 /* REREADAC's chain of tests (PIPAGE, then DELVZ, DELVY and TEMZ): each
  * +0, -0, positive or negative by bits of the state number (0-1, 2-3, 4-5
  * and 5-6). */
 if(KEY(037u,03614u)){static const unsigned at[]={01257u,0330u,0326u,01256u};
  for(i=0;i<4u;++i){unsigned k=(v>>(i<3u?2u*i:5u))%4u;
   W[at[i]]=k==0u?0u:k==1u?077777u:k==2u?(agc_word)((r15(&seed)&037777u)|1u):(agc_word)(040000u|(r15(&seed)&037776u));}}
 /* ALSIGNAG's return: A +0 in a quarter of the states, L below LRHMAX in
  * half. */
 if(KEY(033u,03257u)&&v%8u!=5u){if(!below(&seed,4u))W[0]=0;if(below(&seed,2u))W[1]=(agc_word)below(&seed,0100u);}
 if(r<LMSV_BASIC_ROUTINES){W[5]=(agc_word)(02000u+ph%1024u);return;}
 /* UNIT of a zero vector at CALCGRAV and MUNGRAV in an eighth. */
 if((KEY(033u,02730u)||KEY(033u,03130u))&&v%8u==6u)for(i=0154u;i<=0161u;++i)W[i]=0;
 /* Interpretive keys: at NEWOPS with LOC there, BANKSET the key's bank. */
 W[5]=NEWOPS;W[LOC]=(agc_word)(02000u+ph%1024u);W[BANKSET]=W[6];W[0115]=040000u;
}

#ifndef LMSV_UNDO_ADDRESS
static apollo_overlay_state undo_image;
#define LMSV_UNDO (&undo_image)
#else
#define LMSV_UNDO ((apollo_overlay_state *)LMSV_UNDO_ADDRESS)
#endif
unsigned lmsv_run(apollo_overlay_state *s,uint16_t n,lmsv_case *out){
 lmsv_case c;lm_sv_io io;agc_routine_table basic[1],interp[1];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;
 /* A continuation another C64 group runs: this build lacks its image. */
 if(n>=LMSV_BASE_CASES&&!lmsv_chain_edits[n-LMSV_BASE_CASES])return 4u;
 lmsv_input(s,&c,n);
 io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;
 io.chan11=c.chan11;io.chan12=c.chan12;io.chan32=c.chan32;io.chan33=c.chan33;io.undo=LMSV_UNDO;
 basic[0]=lm_servicer_routines;interp[0]=lm_servicer_interpretive;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+NEWOPS-04000u);program.loc=LOC;program.bankset=BANKSET;
 call=agc_transfer_resolve(&program,s,c.superbank);
 if(!call)return 4u;
 st=call(&program,s,&io);
 if(out){*out=c;out->interrupt_enabled=io.control.interrupt_enabled;out->chan11=io.chan11;out->chan12=io.chan12;
  out->chan32=io.chan32;out->chan33=io.chan33;}
 if(st==EXEC_OK)return LMSV_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return LMSV_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lmsv_digest(apollo_overlay_state *s,uint16_t n){lmsv_case c;unsigned i,kind=lmsv_run(s,n,&c);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=LMSV_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
  mix(c.chan11);mix(c.chan12);mix(c.chan32);mix(c.chan33);mix(c.interrupt_enabled);}
 return (uint32_t)sum_b<<16|sum_a;
}
