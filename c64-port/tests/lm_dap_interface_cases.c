/* SPDX-License-Identifier: MIT */
#include "lm_dap_interface_cases.h"
#include "p66_native.h"
#include "waitlist.h"
#define ANY_FB 0177777u
#define SWRETURN 04631u
const agc_word lm_dap_entry_z[LM_DAP_ENTRIES]={LM_DAP_RESTORDB,LM_DAP_SETMAXDB,LM_DAP_SETMINDB,LM_DAP_PFLITEDB,LM_DAP_ZATTEROR,LM_DAP_STOPRATE,LM_DAP_ALLCOAST};
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
/* Canonical Executive and Waitlist erasable (as lm-exec-services): cores
 * busy core sets, vacs VAC areas in use, count Waitlist tasks. */
static void canon(apollo_overlay_state *m,uint8_t cores,uint8_t vacs,uint8_t count,uint32_t *seed){executive e;unsigned i;
 p66_native_import_executive(&e,m);e.alarm=0;e.newjob=0;
 for(i=0;i<EXEC_CORE_COUNT;++i)e.core[i][EXEC_PRIORITY]=i<cores?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<EXEC_VAC_COUNT;++i)e.vac[i][0]=i<vacs?0:(agc_word)(0400u+i*44u);
 p66_native_export_executive(m,&e);
 m->words[026]=(agc_word)(rnd(seed)>>17&037777u);
 for(i=0;i<8u;++i)m->words[01400u+i]=(agc_word)(077777u-(rnd(seed)>>16)%6000u);
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=i<count?(agc_word)(02000u+i):WL_END_LOCATION;m->words[01411u+2u*i]=i<count?(agc_word)(060007u|(i%8u)*020u):WL_END_BANK;}
}
void lm_dap_input(apollo_overlay_state *m,p66_bias_io *io,lm_dap_case *c,uint16_t n){uint32_t seed=0x2123u+n;unsigned i,v=n/LM_DAP_ENTRIES;agc_word q,eb;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 m->words[7]=0;for(i=010;i<=061u;++i)m->words[i]=0;
 /* The CDU counters hold any angle. */
 for(i=032;i<=034u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 canon(m,(uint8_t)(v%8u==7u?8u:1u+v%7u),0,(uint8_t)(v%10u),&seed);
 c->superbank=(agc_word)((v%4u)*020u);io->superbank=c->superbank;io->interrupt_enabled=0;
 /* Return: SWRETURN, fixed-fixed 06000-07777, or bank 20 outside this file. */
 if(v%3u==0u){q=SWRETURN;c->stop_fb=ANY_FB;}
 /* Q+1/Q+2 (06737, 06741), which NOVAC's exit passes through, are excluded. */
 else if(v%3u==1u){do q=(agc_word)(06000u+(rnd(&seed)>>17)%02000u);while(q>=06730u&&q<=06745u);c->stop_fb=ANY_FB;}
 else{do q=(agc_word)(02000u+(rnd(&seed)>>17)%02000u);while(q>=LM_DAP_FIRST&&q<=LM_DAP_LAST);c->stop_fb=LM_DAP_FB;}
 eb=(agc_word)(((rnd(&seed)>>17)%8u)<<8);
 m->words[3]=eb;m->words[4]=LM_DAP_FB;m->words[6]=(agc_word)(LM_DAP_FB|(eb>>8));
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 m->words[2]=q;m->words[5]=lm_dap_entry_z[n%LM_DAP_ENTRIES];c->stop_z=q;
}
static uint16_t stop_point(const lm_dap_case *c){return agc_fixed_physical(c->stop_z,c->stop_fb==ANY_FB?0u:c->stop_fb,c->superbank);}
unsigned lm_dap_run(apollo_overlay_state *s,uint16_t n,uint16_t *steps){
 agc_routine_table tables[2];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};p66_bias_io io;lm_dap_case c;enum exec_status st;
 tables[0]=lm_dap_interface_routines;tables[1]=lm_exec_routines;program.basic=tables;program.basic_count=2;
 program.operands=lm_exec_site_table(&program.operand_count);
 lm_dap_input(s,&io,&c,n);
 st=agc_transfer_run(&program,s,&io,&c.superbank,stop_point(&c),24,steps);
 if(st==EXEC_OK)return LM_DAP_RETURNED;
 if(st==EXEC_UNMAPPED&&(s->words[5]==LM_EXEC_BAILOUT1||s->words[5]==LM_EXEC_POODOO1))return LM_DAP_ALARM;
 return 2u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t lm_dap_digest(apollo_overlay_state *s,uint16_t n){uint16_t steps;unsigned i,kind=lm_dap_run(s,n,&steps);
 sum_a=sum_b=0;mix((uint16_t)kind);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
