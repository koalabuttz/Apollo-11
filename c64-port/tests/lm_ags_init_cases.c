/* SPDX-License-Identifier: MIT */
#include "lm_ags_init_cases.h"
#define W m->words
#define NEWOPS 06070u
#define FIXLOC 0120u
#define LOC 0164u
#define BANKSET 0165u
typedef char ags_cases_check[AGS_CASES==AGS_ROUTINES*AGS_STATES?1:-1];
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
static unsigned below(uint32_t *s,unsigned n){return (unsigned)((rnd(s)>>16)%n);}
/* A word of one of four CCS classes. */
static agc_word ccs_class(uint32_t *seed,unsigned k){agc_word v=(agc_word)(below(seed,037776u)+1u);
 return k==0u?v:k==1u?0u:k==2u?(agc_word)(~v&077777u):077777u;
}

int ags_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){
 uint16_t ph;
 /* A TC Q on its way, or the interpreter's erasable instructions (the
  * BUF2 / ADDRWD transfers). */
 if(z<02000u)return !(z==2u||z==0133u||z==0137u||z==0141u||z==0156u);
 if(z==NEWOPS){ph=agc_fixed_physical(loc,bankset,superbank);
  return !((ph>=032u*1024u+037u&&ph<=032u*1024u+071u)||(ph>=032u*1024u+0132u&&ph<=032u*1024u+0167u));}
 if(z>=04621u&&z<=04630u)return 0;   /* SWCALL, after RTB */
 if(z>=06037u&&z<=07707u)return 0;   /* the interpreter */
 if(z>=04000u)return 1;
 ph=agc_fixed_physical(z,fb,superbank);
 if(ph>=032u*1024u+05u&&ph<=032u*1024u+0131u)return 0;
 if(ph<=03734u-02000u)return 0;          /* the interpreter in bank 0 */
 if(ph>=1024u+0344u&&ph<=1024u+0600u)return 0;   /* and bank 1 */
 return 1;
}

static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){W[4]=fb;W[3]=eb;W[6]=(agc_word)(fb|(eb>>8));}
#define KEY(a) (ph==(uint16_t)(032u*1024u+((a)-02000u)))

void ags_input(apollo_overlay_state *m,ags_case *c,uint16_t n){
 uint32_t seed=0x4a65ul+(uint32_t)n*17u;unsigned i,v,r=n%AGS_ROUTINES;uint16_t ph=ags_routine_keys[r];
 agc_word eb,vac;
 v=n/AGS_ROUTINES;
 for(i=0;i<2048u;++i)W[i]=r15(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 c->interrupt_enabled=(uint16_t)(v&1u);
 c->superbank=(agc_word)(((v>>1)%4u)<<4);
 /* EBANK= AGSBUFF (E4) as the extended verb sets it, another bank in a
  * quarter of the states. */
 eb=(v>>3)%4u==3u?(agc_word)(below(&seed,8u)<<8):02000u;
 set_banks(m,LM_AGS_FB32,eb);
 /* NEWJOB +0 (a job waiting in a fifth of the states). */
 W[067]=(v>>5)%5u==4u?(agc_word)(below(&seed,0400u)+1u):0;
 /* REFSMFLG and IMUSE set or clear; MPAC BIT6 (V32) in half the enter
  * returns; IMUCADR of any CCS class. */
 W[077]=(v>>2)&1u?(agc_word)(W[077]|010000u):(agc_word)(W[077]&~010000u);
 W[074]=(v>>3)&1u?(agc_word)(W[074]|0200u):(agc_word)(W[074]&~0200u);
 if((v>>4)&1u)W[0154]=040u;
 W[01304]=ccs_class(&seed,(v>>6)%4u);
 /* A work area of one of the five VAC areas, the pushdown pointer inside
  * it, MODE one of the three. */
 vac=(agc_word)(0401u+054u*below(&seed,5u));W[FIXLOC]=vac;W[0166]=(agc_word)(vac+below(&seed,040u));
 W[0163]=(agc_word)(v%3u==0u?0u:v%3u==1u?1u:077776u);
 if(r<AGS_BASIC_ROUTINES){W[5]=(agc_word)(02000u+ph%1024u);return;}
 /* Interpretive keys: at NEWOPS with LOC there, BANKSET bank 32; inside
  * SCALEVEC a genuine QPRET, or a CADR elsewhere in a quarter. */
 W[5]=NEWOPS;W[LOC]=(agc_word)(02000u+ph%1024u);W[BANKSET]=W[6];W[0115]=040000u;W[023]=0;
 if(KEY(02144)||KEY(02156)){agc_word q=(v>>7)%2u?AGS_QPRET_2061:AGS_QPRET_2053;
  if((v>>8)%4u==3u)q=(agc_word)(04000u|r15(&seed));
  W[vac+052u]=q;}
}

#ifndef AGS_UNDO_ADDRESS
static apollo_overlay_state undo_image;
#define AGS_UNDO (&undo_image)
#else
#define AGS_UNDO ((apollo_overlay_state *)AGS_UNDO_ADDRESS)
#endif
unsigned ags_run(apollo_overlay_state *s,uint16_t n,uint16_t *ie){
 ags_case c;lm_ags_io io;agc_routine_table basic[1],interp[1];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;
 ags_input(s,&c,n);
 io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;io.undo=AGS_UNDO;
 basic[0]=lm_ags_init_routines;interp[0]=lm_ags_init_interpretive;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+NEWOPS-04000u);program.loc=LOC;program.bankset=BANKSET;
 call=agc_transfer_resolve(&program,s,c.superbank);
 if(!call)return 3u;
 st=call(&program,s,&io);
 if(ie)*ie=io.control.interrupt_enabled;
 if(st==EXEC_OK)return AGS_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return AGS_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t ags_digest(apollo_overlay_state *s,uint16_t n){uint16_t ie=0;unsigned i,kind=ags_run(s,n,&ie);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=AGS_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));mix(ie);}
 return (uint32_t)sum_b<<16|sum_a;
}
