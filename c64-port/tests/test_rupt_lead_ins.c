/* SPDX-License-Identifier: MIT */
/* rupt-lead-ins: INTERRUPT_LEAD_INS.agc of Luminary099 and Comanche055 and
 * Comanche's RESUME family as native routines, against digests frozen from
 * the original programs; every entry and both T5RUPT continuations are
 * reached; inputs no original producer supplies are refused with the image
 * unchanged. */
#include <stdio.h>
#include <string.h>
#include "rupt_lead_ins_cases.h"
#include "rupt_lead_ins_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* Rerun case n's input after `edit` and expect a refusal with nothing changed. */
static void refused(unsigned cm,uint16_t n,unsigned word,agc_word value,int outside,const char *what){
 static apollo_overlay_state s,before;rli_io io,io0;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 rli_input(cm,&s,&io,n);s.words[word]=value;if(outside)io.hw.in_interrupt=0;before=s;io0=io;
 program.basic=cm?&cm_lead_in_routines:&lm_lead_in_routines;program.basic_count=1;program.hardware=&io.hw;
 call=agc_transfer_resolve(&program,&s,cm?io.cm.superbank:io.lm.superbank);
 check(call&&call(&program,&s,cm?(void *)&io.cm:(void *)&io.lm)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),what,n);
}
/* #144: a native RESUME continues at ZRUPT-1, so BRUPT must be the word
 * there. Case n of a CM RESUME entry, interrupted at erasable 01001 or (with
 * a fixed reader) at fixed 04001: BRUPT equal to that word resumes, any
 * other refuses with nothing changed. */
static agc_word fixed_word;
static bool reader(const agc_native_program *p,uint16_t physical,agc_word *w){(void)p;(void)physical;*w=fixed_word;return true;}
static void brupt(uint16_t n,agc_word zrupt,int fixed,int equal,const char *what){
 static apollo_overlay_state s,before;rli_io io,io0;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;
 rli_input(1,&s,&io,n);s.words[015]=zrupt;fixed_word=(agc_word)(s.words[01000]^0123u);
 s.words[017]=(agc_word)((fixed?fixed_word:s.words[01000])^(equal?0u:1u));before=s;io0=io;
 program.basic=&cm_lead_in_routines;program.basic_count=1;program.hardware=&io.hw;if(fixed)program.fixed_data=reader;
 call=agc_transfer_resolve(&program,&s,io.cm.superbank);st=call?call(&program,&s,&io.cm):EXEC_OUT_OF_SCOPE;
 if(equal)check(st==EXEC_OK&&s.words[5]==(agc_word)(zrupt-1u),what,n);
 else check(st==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),what,n);
}
int main(void){static apollo_overlay_state s;unsigned cm,n,e,kind[3]={0,0,0},to_noqbrsm=0,to_t5loc=0;
 for(cm=0;cm<2u;++cm){const uint32_t *expected=cm?rli_cm_expected:rli_lm_expected;unsigned entries=rli_cases(cm)/RLI_PER_ENTRY,seen[15];
  memset(seen,0,sizeof seen);
  for(n=0;n<rli_cases(cm);++n){static rli_io io;unsigned k;agc_word z=rli_entry(cm,(uint16_t)n);
   check(rli_digest(cm,&s,(uint16_t)n)==expected[n],"digest",n);
   /* Only the CM rope-reader RESUME cases (every fourth) may refuse: BRUPT not the word at ZRUPT-1. */
   {bool rope=cm&&z>=05333u&&n%4u==3u;
   k=rli_run(cm,&s,&io,(uint16_t)n);check(k==RLI_RETURNED||(rope&&k==RLI_REFUSED),"outcome",n);if(k<3u)++kind[k];
   if(k==RLI_REFUSED){++seen[n/RLI_PER_ENTRY];continue;}}
   ++seen[n/RLI_PER_ENTRY];
   if(cm&&(z==04010u||z==04065u)){if(s.words[5]==05337u)++to_noqbrsm;else ++to_t5loc;}
   if(z!=04000u&&!(cm&&z>=05333u))check(io.hw.in_interrupt==1u,"interrupt still in progress",n);
   if(cm&&z>=05333u)check(io.hw.in_interrupt==0u&&io.cm.interrupt_enabled==1u,"interrupt ended",n);}
  for(e=0;e<entries;++e)check(seen[e]==RLI_PER_ENTRY,"entry reached",e);}
 check(to_noqbrsm>0u&&to_t5loc>0u,"both T5RUPT continuations",0);
 /* Inputs no producer supplies. */
 refused(0,1u*RLI_PER_ENTRY,01274u,0100u,0,"LM T5RUPT with T5ADR in erasable");
 refused(1,1u*RLI_PER_ENTRY,01310u,0u,0,"CM T6RUPT with T6LOC zero");
 refused(1,10u*RLI_PER_ENTRY,01312u,01000u,0,"CM T5RUPT with T5LOC in erasable");
 refused(0,2u*RLI_PER_ENTRY,0u,0u,1,"LM T4RUPT outside an interrupt");
 refused(1,4u*RLI_PER_ENTRY,0u,0u,1,"CM KEYRUPT1 outside an interrupt");
 refused(1,11u*RLI_PER_ENTRY,015u,0u,0,"CM RESUME with no interrupted address");
 refused(1,12u*RLI_PER_ENTRY,016u,077777u,0,"CM NOQRSM with BANKRUPT not a bank");
 brupt(11u*RLI_PER_ENTRY,01001u,0,1,"CM RESUME, BRUPT the erasable word at ZRUPT-1");
 brupt(11u*RLI_PER_ENTRY,01001u,0,0,"CM RESUME refused, BRUPT not the erasable word at ZRUPT-1");
 brupt(14u*RLI_PER_ENTRY,04001u,1,1,"CM NOQBRSM, BRUPT the fixed word at ZRUPT-1");
 brupt(14u*RLI_PER_ENTRY,04001u,1,0,"CM NOQBRSM refused, BRUPT not the fixed word at ZRUPT-1");
 printf("rupt-lead-ins: %u LM and %u CM cases (%u returned, %u refused); CM T5RUPT to NOQBRSM %u, to T5LOC %u; %u checks, %u failures\n",
  RLI_LM_CASES,RLI_CM_CASES,kind[0],kind[2],to_noqbrsm,to_t5loc,checks,failures);
 return failures!=0u;
}
