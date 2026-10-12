/* SPDX-License-Identifier: MIT */
#include "cm_stateup_cases.h"
#define W m->words
#define NEWOPS CM_STATEUP_NEWOPS
#define LOC CM_STATEUP_LOC
#define BANKSET CM_STATEUP_BANKSET
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
static unsigned below(uint32_t *s,unsigned n){return (unsigned)((rnd(s)>>16)%n);}
/* The original's interpreter: its fixed-fixed code (06006-07656), bank 0
 * (00,2000-3731) and bank 1 (01,2371-2625) bodies, and Z = 2 (TC Q). */
int cm_stateup_leaves(agc_word z,agc_word fb,agc_word superbank,agc_word loc,agc_word bankset){
 uint16_t ph;
 if(z<02000u)return z!=2u;
 if(z==NEWOPS){ph=agc_fixed_physical(loc,bankset,superbank);
  return !((ph<=013u*1024u+035u&&ph>=013u*1024u)||(ph>=04u*1024u+0131u&&ph<=04u*1024u+0133u));}
 if(z>=06006u&&z<=07656u)return 0;
 if(z>=04000u)return 1;
 ph=agc_fixed_physical(z,fb,superbank);
 if(ph<=03731u-02000u)return 0;
 if(ph>=1024u+0371u&&ph<=1024u+0625u)return 0;
 return 1;
}
void cm_stateup_input(apollo_overlay_state *m,p66_bias_io *io,uint16_t n){
 uint32_t seed=UINT32_C(0x2055)+(uint32_t)n*UINT32_C(7919);unsigned i,k=n%CM_STATEUP_ROUTINES;agc_word eb,vac;
 for(i=0;i<2048u;++i)W[i]=r15(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 io->interrupt_enabled=(uint16_t)below(&seed,2u);io->superbank=(agc_word)(below(&seed,4u)<<4);
 eb=(agc_word)(below(&seed,8u)<<8);
 W[4]=(agc_word)((cm_stateup_entries[k]>=02131u?04u:013u)<<10);W[3]=eb;W[6]=(agc_word)(W[4]|(eb>>8));
 W[5]=NEWOPS;W[LOC]=cm_stateup_entries[k];W[BANKSET]=W[6];W[023]=0;W[0115]=040000u;
 W[067]=below(&seed,5u)==4u?(agc_word)(below(&seed,0400u)+1u):0u;
 vac=(agc_word)(0401u+054u*below(&seed,5u));W[0120]=vac;W[0166]=(agc_word)(vac+below(&seed,043u));
 {unsigned m3=below(&seed,3u);W[0163]=(agc_word)(m3==0u?0u:m3==1u?1u:077776u);}
 /* QPRET as CALL leaves it (074000 + the return address in BANKSET's
  * bank): THISVINT's caller at 13,3574, or another word of bank 13
  * outside the file. */
 if(cm_stateup_entries[k]==02034u)W[vac+052u]=below(&seed,4u)?077576u:(agc_word)(074000u+02100u+below(&seed,01700u));
}
enum exec_status cm_stateup_run(apollo_overlay_state *s,p66_bias_io *io,uint16_t n,uint16_t *steps){
 agc_routine_table interp[1];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 interp[0]=cm_stateup_interpretive;
 program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+NEWOPS-04000u);program.loc=LOC;program.bankset=BANKSET;
 cm_stateup_input(s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,AGC_TRANSFER_NONE,32,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t cm_stateup_digest(apollo_overlay_state *s,uint16_t n,enum exec_status *status){p66_bias_io io;uint16_t steps=0;unsigned i;
 *status=cm_stateup_run(s,&io,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
