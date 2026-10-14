/* SPDX-License-Identifier: MIT */
#include "lm_dapidler_cases.h"
#include "lm_exec_routines.h"
#include "lm_interbank_routines.h"
#include "p66_native.h"
#include "waitlist.h"
#define ANY_FB 0177777u
#define SWRETURN 04631u
#define NO_STOP 0177776u
/* Physical word of an EBANK6 address. */
#define EB6(a) (03000u+((a)-01400u))
const agc_word lm_di_entry_z[LM_DI_ENTRIES]={LM_DI_CHEKBITS,LM_DI_CHEKMORE,LM_DI_DAPIDLER,LM_DI_CHECKUP,02042u,02050u,02161u,02163u,
 LM_DI_ALTDSPLY};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&077777u);}
/* Canonical Executive and Waitlist erasable (as lm-dap-interface): cores
 * busy core sets, count Waitlist tasks. */
static void canon(apollo_overlay_state *m,uint8_t cores,uint8_t count,uint32_t *seed){executive e;unsigned i;
 p66_native_import_executive(&e,m);e.alarm=0;e.newjob=0;
 for(i=0;i<EXEC_CORE_COUNT;++i)e.core[i][EXEC_PRIORITY]=i<cores?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<EXEC_VAC_COUNT;++i)e.vac[i][0]=(agc_word)(0400u+i*44u);
 p66_native_export_executive(m,&e);
 m->words[026]=(agc_word)(rnd(seed)>>17&037777u);
 for(i=0;i<8u;++i)m->words[01400u+i]=(agc_word)(077777u-(rnd(seed)>>16)%6000u);
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=i<count?(agc_word)(02000u+i):WL_END_LOCATION;m->words[01411u+2u*i]=i<count?(agc_word)(060007u|(i%8u)*020u):WL_END_BANK;}
}
static int inside16(agc_word q){return (q>=LM_DI_FIRST16&&q<=LM_DI_LAST16)||q==LM_DI_CALCPERR_Z;}
void lm_di_input(apollo_overlay_state *m,lm_flight_io *io,lm_di_case *c,uint16_t n){uint32_t seed=0x2024u+n;unsigned i,v=n/LM_DI_ENTRIES,e=n%LM_DI_ENTRIES;agc_word q,fb;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 /* Counters and the interrupt registers are zero, except the CDU angles,
  * the CDU command counters and BANKRUPT/QRUPT, which DAPIDLER reads. */
 m->words[7]=0;for(i=010;i<=061u;++i)if(!(i>=032u&&i<=034u)&&!(i>=050u&&i<=052u)&&i!=012u&&i!=016u)m->words[i]=0;
 canon(m,(uint8_t)(v%8u==7u?8u:1u+v%7u),(uint8_t)(v%10u),&seed);
 c->superbank=0;io->bias.superbank=0;io->bias.interrupt_enabled=0;
 /* Channel 14 bits 1-5 only: the CDU, gyro and optics drive bits would
  * start yaAGC pulse output. */
 io->hw.channel14=(agc_word)(r15(&seed)&037u);io->hw.channel5=r15(&seed);io->hw.channel6=r15(&seed);io->hw.channel12=r15(&seed);
 io->hw.channel30=r15(&seed);io->hw.channel31=r15(&seed);
 /* Return: SWRETURN, fixed-fixed 06000-07777 away from NOVAC's Q+1/Q+2
  * exit (06730-06745), or bank 16 outside this file and CALCPERR. */
 if(v%3u==0u){q=SWRETURN;c->stop_fb=ANY_FB;}
 else if(v%3u==1u){do q=(agc_word)(06000u+(rnd(&seed)>>17)%02000u);while(q>=06730u&&q<=06745u);c->stop_fb=ANY_FB;}
 else{do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(inside16(q));c->stop_fb=LM_DI_FB16;}
 /* CHEKBITS, CHEKMORE and ALTDSPLY (through CHEKMORE) return to Q. */
 c->stop_z=e<=1u||e==8u?q:NO_STOP;
 fb=e==8u?LM_DI_FB20:LM_DI_FB16;
 m->words[3]=LM_DI_EB;m->words[4]=fb;m->words[6]=(agc_word)(fb|(LM_DI_EB>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=q;m->words[5]=lm_di_entry_z[e];
 /* ALTDSPLY: a small attitude-error input (|AK| < 16, or -0) in a quarter
  * of the cases, so the DAC loop's 0.1 scaling leaves +0 and -0. */
 if(e==8u&&v%4u==1u){unsigned k=(rnd(&seed)>>20)%3u,a=(rnd(&seed)>>20)%16u;
  m->words[EB6(01761u+k)]=(agc_word)((rnd(&seed)>>24)&1u?a:(~a&077777u));}
}
/* This file's compiled site operands: the NOVAC 2CADR 1/ACCSET and the
 * IBNKCALL CADRs of ZATTEROR, QERRCALC and CALCPERR, as in
 * src/lm_exec_sites.inc and src/lm_interbank_sites.inc (the host test
 * checks each against lm_exec_site_table and lm_interbank_site_table; the
 * whole tables do not fit the C64 diagnostic). */
const agc_site_operand lm_di_sites[LM_DI_SITES]={
 {14367u,002447},{14368u,040106},{14375u,040153},{14448u,036626},{14450u,035446}};
unsigned lm_di_run(apollo_overlay_state *s,lm_flight_io *io,uint16_t n,uint16_t *steps){
 agc_routine_table tables[3];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};lm_di_case c;enum exec_status st;uint16_t stop;agc_word z,fb;
 program.hardware=&io->hw;
 tables[0]=lm_dapidler_routines;tables[1]=lm_interbank_routines;tables[2]=lm_exec_routines;
 program.basic=tables;program.basic_count=3;program.operands=lm_di_sites;program.operand_count=LM_DI_SITES;
 lm_di_input(s,io,&c,n);
 stop=c.stop_z==NO_STOP?(uint16_t)(AGC_TRANSFER_NONE-1u):agc_fixed_physical(c.stop_z,c.stop_fb==ANY_FB?0u:c.stop_fb,c.superbank);
 st=agc_transfer_run(&program,s,io,&c.superbank,stop,32,steps);
 if(st==EXEC_OK)return LM_DI_RETURNED;
 if(st!=EXEC_UNMAPPED)return LM_DI_NONE;
 z=s->words[5];fb=(agc_word)(s->words[4]&076000u);
 if(z==LM_EXEC_BAILOUT1||z==LM_EXEC_POODOO1)return LM_DI_ALARM;
 if(z==LM_DI_RESUME)return LM_DI_RESUMED;
 if(z==LM_DI_QERRCALC_Z&&fb==036000u)return LM_DI_QERRCALC;
 if(z==LM_DI_CALCPERR_Z&&fb==LM_DI_FB16)return LM_DI_CALCPERR;
 if(z==LM_DI_ZATTEROR_Z&&fb==LM_DI_FB20)return LM_DI_ZATTEROR;
 return LM_DI_NONE;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_di_digest(apollo_overlay_state *s,uint16_t n){lm_flight_io io;uint16_t steps;unsigned i,kind=lm_di_run(s,&io,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 mix(io.hw.channel5);mix(io.hw.channel6);mix(io.hw.channel12);mix(io.hw.channel14);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
