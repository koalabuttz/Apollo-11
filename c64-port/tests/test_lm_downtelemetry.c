/* SPDX-License-Identifier: MIT */
/* lm-downtelemetry: Luminary099 DOWN-TELEMETRY_PROGRAM downrupt sequences
 * through the trampoline, against digests frozen from the original program;
 * every registered entry is reached, every word of all six downlists is
 * sent, every dump case runs its dumps to completion and returns to the
 * downlist; a DNTMGOTO word no producer stores, a DNLSTCOD with no downlist,
 * a missing io record and a bank other than 05 are refused. */
#include <stdio.h>
#include "lm_downtelemetry_cases.h"
#include "lm_downtelemetry_vectors.h"
#include "../src/lm_downtelemetry_data.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[LM_DNTM_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((lm_dntm_io *)io)->control.superbank);
 for(i=0;i<LM_DNTM_ROUTINES;++i)if(lm_downtelemetry_routines.entries[i].physical==at){++reached[i];return lm_downtelemetry_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static agc_word word(uint16_t physical){agc_word w=0;(void)agc_site_lookup(lm_dntm_data,LM_DNTM_DATA_COUNT,physical,&w);return w;}
#define B5(z) (uint16_t)(LM_DNTM_BANK*1024u+((z)-02000u))
int main(void){apollo_overlay_state s;lm_dntm_io io;enum exec_status st;unsigned n,i,k;
 static uint8_t seen[02000];unsigned list_words=0,sent=0;
 check(LM_DNTM_VECTORS==LM_DNTM_CASES,"vector count",0);
 for(n=1;n<LM_DNTM_DATA_COUNT;++n)check(lm_dntm_data[n-1u].physical<lm_dntm_data[n].physical,"data sorted",n);
 for(n=1;n<LM_DNTM_ROUTINES;++n)check(lm_downtelemetry_routines.entries[n-1u].physical<lm_downtelemetry_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<LM_DNTM_CASES;++n)check(lm_dntm_digest(&s,(uint16_t)n,&st)==lm_dntm_expected[n]&&st==EXEC_OK,"oracle",n);
 /* Coverage over the same sequences, with a counting wrapper. */
 {agc_routine_entry wrapped[LM_DNTM_ROUTINES];agc_routine_table table;agc_native_program p={0,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<LM_DNTM_ROUTINES;++i){wrapped[i].physical=lm_downtelemetry_routines.entries[i].physical;wrapped[i].call=count;}
  table.entries=wrapped;table.count=LM_DNTM_ROUTINES;p.basic=&table;
  for(n=0;n<LM_DNTM_CASES;++n){lm_dntm_begin(&s,&io,(uint16_t)n);
   for(k=0;k<lm_dntm_steps((uint16_t)n);++k){agc_word ct;lm_dntm_entry(&s,&io,(uint16_t)n,(uint16_t)k);
    check(agc_transfer_run(&p,&s,&io,&io.control.superbank,LM_DNTM_RESUME,8,&steps)==EXEC_OK,"coverage run",n);
    ct=s.words[0334];if(ct>=02000u&&ct<04000u)seen[ct-02000u]=1;}
   if(n>=LM_DNTM_CASES-LM_DNTM_DUMP_CASES)
    check(s.words[LM_DNTM_DNTMGOTO]==LM_DNTM_DNPHASE2,"dump completed",n);}
  for(i=0;i<LM_DNTM_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);}
 /* Every control word of the six downlists (DNTABLE start to the final,
  * negative word) is the next word CTLIST points to at some downrupt. */
 for(i=0;i<6u;++i){agc_word at=word(B5(02441u+i));
  for(;;){++list_words;if(at>=02000u&&at<04000u&&seen[at-02000u])++sent;if(word(B5(at))&040000u)break;++at;}}
 check(list_words>0u&&sent==list_words,"every downlist word sent",sent);
 /* Refusals. */
 {agc_native_program p={&lm_downtelemetry_routines,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
  lm_dntm_begin(&s,&io,0);lm_dntm_entry(&s,&io,0,0);s.words[LM_DNTM_DNTMGOTO]=03516u;s.words[5]=LM_DNTM_DNTMGOTO;
  call=agc_transfer_resolve(&p,&s,0);check(call&&call(&p,&s,&io)==EXEC_OUT_OF_SCOPE,"unknown DNTMGOTO word",0);
  lm_dntm_begin(&s,&io,0);lm_dntm_entry(&s,&io,0,0);s.words[LM_DNTM_DNLSTCOD]=6u;
  check(lm_dntm_downrupt(&s,&io)==EXEC_OUT_OF_SCOPE,"DNLSTCOD with no downlist",0);
  lm_dntm_begin(&s,&io,0);lm_dntm_entry(&s,&io,0,0);
  call=agc_transfer_resolve(&p,&s,0);check(call&&call(&p,&s,0)==EXEC_OUT_OF_SCOPE,"no io",0);
  lm_dntm_begin(&s,&io,0);lm_dntm_entry(&s,&io,0,0);s.words[LM_DNTM_DNTMGOTO]=LM_DNTM_DNPHASE1;s.words[5]=LM_DNTM_DNTMGOTO;s.words[4]=s.words[6]=014000u;
  call=agc_transfer_resolve(&p,&s,0);check(call&&call(&p,&s,&io)==EXEC_OUT_OF_SCOPE,"bank other than 05",0);}
 printf("lm-downtelemetry: %u cases, %u downlist words all sent, %u checks, %u failures\n",LM_DNTM_CASES,list_words,checks,failures);
 return failures!=0u;
}
