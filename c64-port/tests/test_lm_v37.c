/* SPDX-License-Identifier: MIT */
/* lm-v37: Luminary099 FRESH_START_AND_RESTART bank 04 (verb 37, R00, POOH,
 * V37XEQ, SETUP70/71) cases through the trampoline, with bank 05 and the
 * interbank routines, against digests frozen from the original program;
 * every registered entry is reached and each run ends at a routine outside
 * the file (or P70/P71, or ISWRETRN after an IBNKCALL'd KLEAN); the bank's
 * compiled CADRs equal Lane A's interbank site table, and the operand table
 * holds every word bank 05 reads; a
 * MINDEX past the major-mode tables, a negative MMNUMBER, an overflowed
 * request and a missing io record are refused. */
#include <stdio.h>
#include "lm_v37_cases.h"
#include "lm_v37_vectors.h"
#include "lm_interbank_routines.h"
#include "../src/lm_v37_operands.inc"
#include "../src/lm_fresh_start_operands.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[LM_V37_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<LM_V37_ROUTINES;++i)if(lm_v37_routines.entries[i].physical==at){++reached[i];return lm_v37_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,lm_fresh_io *io){
 agc_native_program p={&lm_v37_routines,1,0,0,lm_v37_operands,LM_V37_OPERAND_COUNT,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 call=agc_transfer_resolve(&p,s,io?io->control.superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
static void at(apollo_overlay_state *s,lm_fresh_io *io,agc_hardware *hw,agc_word z){unsigned k;
 for(k=0;k<LM_V37_ROUTINES&&lm_v37_entries[k]!=z;++k){}
 lm_v37_input(s,io,hw,(uint16_t)k);
}
/* Where a run may end: Z, bank (99 any), Q = 3 required. */
static const struct { uint16_t z,bank; uint8_t q3; const char *name; } ends[]={
 {05516u,99u,0,"DOWNFLAG"},{02476u,010u,0,"GOFLASH"},{04457u,99u,0,"RELDSP"},{03555u,036u,0,"ENGINOF1"},
 {05315u,99u,0,"MMDSPLAY"},{05155u,99u,0,"ENDOFJOB"},{06011u,99u,0,"CLRADMOD"},{05353u,99u,0,"PHASCHNG"},
 {04707u,99u,1,"ISWRETRN"},{06037u,99u,0,"INTPRET"},{05476u,99u,0,"CLEARMRK +2"},{05357u,99u,0,"PHSCHNGA"},
 {05314u,99u,0,"NEWMODEA"},{05116u,99u,0,"SPVAC"},{02204u,020u,0,"ALLCOAST"},{03050u,010u,0,"PINBRNCH"},
 {03210u,01u,0,"DUMMYJOB +2"},{02075u,021u,1,"P71"},{02072u,021u,0,"P70"},{04364u,99u,0,"FALTON"},{05567u,99u,0,"ALARM"}};
#define ENDS (sizeof ends/sizeof ends[0])
int main(void){apollo_overlay_state s;lm_fresh_io io;agc_hardware hw;enum exec_status st;unsigned n,i,ended[ENDS];uint16_t sites=0;
 const agc_site_operand *table=lm_interbank_site_table(&sites);
 for(i=0;i<ENDS;++i)ended[i]=0;
 check(LM_V37_VECTORS==LM_V37_CASES,"vector count",0);
 for(n=1;n<LM_V37_OPERAND_COUNT;++n)check(lm_v37_operands[n-1u].physical<lm_v37_operands[n].physical,"operands sorted",n);
 for(n=1;n<LM_V37_ROUTINES;++n)check(lm_v37_routines.entries[n-1u].physical<lm_v37_routines.entries[n].physical,"routines sorted",n);
 /* The CADRs after the bank's TC BANKCALL/POSTJUMP/IBNKCALL (bank 04, 2024-2377) are the site table's. */
 {unsigned found=0;
  for(n=0;n<LM_V37_OPERAND_COUNT;++n){uint16_t ph=lm_v37_operands[n].physical;unsigned k;
   if(ph<04u*1024u||ph>=05u*1024u)continue;
   for(k=0;k<sites&&table[k].physical!=ph;++k){}
   if(k<sites){++found;check(table[k].word==lm_v37_operands[n].word,"site table CADR",n);}}
  check(found==7u,"seven interbank CADRs",found);}
 /* lm_v37_operands covers the whole file: every word bank 05 reads (lm_fresh_operands) is in it. */
 for(n=0;n<LM_FRESH_OPERAND_COUNT;++n){unsigned k;
  for(k=0;k<LM_V37_OPERAND_COUNT&&lm_v37_operands[k].physical!=lm_fresh_operands[n].physical;++k){}
  check(k<LM_V37_OPERAND_COUNT&&lm_v37_operands[k].word==lm_fresh_operands[n].word,"bank 05 operand",n);}
 for(n=0;n<LM_V37_CASES;++n)check(lm_v37_digest(&s,(uint16_t)n,&st)==lm_v37_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[LM_V37_ROUTINES],interbank[16];agc_routine_table tables[3];uint16_t steps,k=0;
  agc_native_program p={0,3,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
  for(i=0;i<LM_V37_ROUTINES;++i){wrapped[i].physical=lm_v37_routines.entries[i].physical;wrapped[i].call=count;}
  for(i=0;i<lm_interbank_routines.count;++i)if(lm_interbank_routines.entries[i].physical!=LM_V37_ISWRETRN_POINT)interbank[k++]=lm_interbank_routines.entries[i];
  tables[0].entries=wrapped;tables[0].count=LM_V37_ROUTINES;tables[1]=lm_fresh_start_routines;tables[2].entries=interbank;tables[2].count=k;
  p.basic=tables;p.hardware=&hw;p.operands=lm_v37_operands;p.operand_count=LM_V37_OPERAND_COUNT;
  for(n=0;n<LM_V37_CASES;++n){unsigned bank;agc_word z;
   lm_v37_input(&s,&io,&hw,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.control.superbank,LM_V37_SWRETURN_POINT,32,&steps);
   check(st==EXEC_UNMAPPED,"coverage run",n);
   z=s.words[5];bank=s.words[4]>>10;if(bank>=030u&&(io.control.superbank&0100u))bank+=010u;
   for(i=0;i<ENDS;++i)if(ends[i].z==z&&(ends[i].bank==99u||ends[i].bank==bank)&&(!ends[i].q3||s.words[2]==3u))break;
   if(i<ENDS)++ended[i];else check(0,"end point",n);}
  for(i=0;i<LM_V37_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<ENDS;++i)check(ended[i]>0u,ends[i].name,i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,&hw,02330u);s.words[LM_V37_MINDEX]=036u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"MINDEX past PREMM1",0);
 at(&s,&io,&hw,02316u);s.words[LM_V37_MINDEX]=077777u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"MINDEX negative",0);
 at(&s,&io,&hw,02161u);s.words[LM_V37_MMNUMBER]=077770u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"negative MMNUMBER",0);
 at(&s,&io,&hw,02040u);s.words[0]=040000u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"overflowed request",0);
 at(&s,&io,&hw,02040u);check(once(&s,0)==EXEC_OUT_OF_SCOPE,"no io",0);
 printf("lm-v37: %u cases, %u entries, ends",LM_V37_CASES,LM_V37_ROUTINES);for(i=0;i<ENDS;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
