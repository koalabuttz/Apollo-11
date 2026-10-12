/* SPDX-License-Identifier: MIT */
/* lm-phase-table: Luminary099 PHASE_TABLE_MAINTENANCE from every genuine
 * caller site through the trampoline, against digests frozen from the
 * original program; every site and registered entry is reached; Q that is
 * no genuine site, a missing io record, and banks the routines are not
 * entered under are refused. */
#include <stdio.h>
#include "lm_phase_table_cases.h"
#include "lm_phase_table_vectors.h"
#include "lm_interbank_routines.h"
#define LM_PHASE_SITE_LIST
#include "../src/lm_phase_sites.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* One routine call at Z, for refusals (a step-limited run would also end
 * in EXEC_OUT_OF_SCOPE after a routine that succeeded). */
static enum exec_status once(const agc_native_program *p,apollo_overlay_state *s,p66_bias_io *io){
 agc_routine call=agc_transfer_resolve(p,s,io?io->superbank:0u);return call?call(p,s,io):EXEC_UNMAPPED;}
static unsigned reached[LM_PT_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_PT_ROUTINES;++i)if(lm_phase_table_routines.entries[i].physical==at){++reached[i];return lm_phase_table_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status novac(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;s->words[5]=(agc_word)(s->words[2]+2u);return EXEC_OK;}
static const agc_routine_entry novac_stub[1]={{FF(LM_PT_NOVAC),novac}};
int main(void){apollo_overlay_state s;p66_bias_io io;lm_pt_log log;unsigned n,i,checkmm_taken=0,checkmm_not=0;uint16_t steps;enum exec_status st;
 check(LM_PT_VECTORS==LM_PT_CASES,"vector count",0);
 check(LM_PT_OPERAND_COUNT==LM_PHASE_SITE_WORDS,"operand count",0);
 for(n=1;n<LM_PHASE_SITE_WORDS;++n)check(lm_phase_sites[n-1u].physical<lm_phase_sites[n].physical,"operands sorted",n);
 for(n=1;n<LM_PT_ROUTINES;++n)check(lm_phase_table_routines.entries[n-1u].physical<lm_phase_table_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<LM_PT_CASES;++n){
  check(lm_pt_digest(&s,(uint16_t)n,&st)==lm_pt_expected[n]&&st==EXEC_OK,"oracle",n);
  if(s.words[5]==LM_PT_Q_PLUS_2)++checkmm_taken;else if(s.words[5]==LM_PT_Q_PLUS_1)++checkmm_not;}
 check(checkmm_taken>0u&&checkmm_not>0u,"CHECKMM both exits",checkmm_taken);
 /* Every registered entry is reached (counting wrapper over the same table). */
 {agc_routine_entry wrapped[LM_PT_ROUTINES];agc_routine_table tables[3];agc_native_program p={0,0,0,0,lm_phase_sites,LM_PHASE_SITE_WORDS,AGC_TRANSFER_NONE,0,0,0,0,0};
  for(i=0;i<LM_PT_ROUTINES;++i){wrapped[i].physical=lm_phase_table_routines.entries[i].physical;wrapped[i].call=count;}
  tables[0].entries=wrapped;tables[0].count=LM_PT_ROUTINES;tables[1]=lm_interbank_routines;tables[2].entries=novac_stub;tables[2].count=1;p.basic=tables;p.basic_count=3;
  for(n=0;n<LM_PT_CASES;++n){lm_pt_input(&s,&io,(uint16_t)n);(void)agc_transfer_run(&p,&s,&io,&io.superbank,0xfffeu,16,&steps);}
  for(i=0;i<LM_PT_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);}
 /* Refusals. */
 {agc_native_program p={&lm_phase_table_routines,1,0,0,lm_phase_sites,LM_PHASE_SITE_WORDS,AGC_TRANSFER_NONE,0,0,0,0,0};
  for(n=0;n<lm_pt_site_count();++n){lm_pt_input(&s,&io,(uint16_t)n);
   if(n<LM_PHASE_SITE_COUNT&&lm_phase_site_list[n].words){s.words[2]=(agc_word)(s.words[2]+lm_phase_site_list[n].words);
    check(once(&p,&s,&io)==EXEC_OUT_OF_SCOPE,"non-site Q",n);}
   lm_pt_input(&s,&io,(uint16_t)n);
   check(once(&p,&s,0)==EXEC_OUT_OF_SCOPE,"no io",n);}
  lm_pt_input(&s,&io,0);s.words[5]=LM_PT_PHSCHNG2;s.words[4]=024000u;s.words[6]=(agc_word)(024000u|(s.words[3]>>8));
  (void)agc_transfer_run(&p,&s,&io,&io.superbank,0xfffeu,1,&steps);
  check(s.words[5]==LM_PT_PHSCHNG2,"PHSCHNG2 only in bank 10",0);}
 (void)log;
 printf("lm-phase-table: %u cases from %u caller sites (%u operand words), CHECKMM %u/%u, %u checks, %u failures\n",
        LM_PT_CASES,LM_PHASE_SITE_COUNT,LM_PHASE_SITE_WORDS,checkmm_taken,checkmm_not,checks,failures);
 return failures!=0u;
}
