/* SPDX-License-Identifier: MIT */
/* cm-update: Comanche055 UPDATE_PROGRAM cases through the trampoline, against
 * digests frozen from the original program; every registered entry is
 * reached and each run ends at an entry outside the file (through both
 * CKMDMORE outcomes) or, for UPACTOFF from either BANKCALL, at SWRETURN; a TC UPTEMP or
 * INDEX UPTEMP; TC 1 word TIMEDIDL never stores, an UPVERB with no branch,
 * an update that would write a register or counter, a missing io record and
 * UPACTOFF without the hardware record are refused. */
#include <stdio.h>
#include "cm_update_cases.h"
#include "cm_update_vectors.h"
#include "cm_update_c64_vectors.h"
#include "cm_interbank_routines.h"
#include "../src/cm_update_operands.inc"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[CM_UP_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((cm_native_io *)io)->superbank);
 for(i=0;i<CM_UP_ROUTINES;++i)if(cm_update_routines.entries[i].physical==at){++reached[i];return cm_update_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw){
 agc_native_program p={&cm_update_routines,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.hardware=hw;p.operands=cm_interbank_site_table(&p.operand_count);call=agc_transfer_resolve(&p,s,io?io->superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The first case entering at z. */
static void at(apollo_overlay_state *s,cm_native_io *io,agc_hardware *hw,agc_word z){unsigned k;
 for(k=0;k<CM_UP_ROUTINES&&cm_up_entries[k].z!=z;++k){}
 cm_up_input(s,io,hw,(uint16_t)k);
}
/* External entries a run may end at (Z, and the bank for switched ones). */
static const struct { agc_word z; uint8_t bank; } ends[14]={
 {02076u,043u},{05412u,0},{05354u,0},{02561u,010u},{04400u,0},{05147u,0},{05217u,0},{06006u,0},{05546u,0},{07226u,0},
 {02671u,027u},{05357u,0},{05534u,0},{CM_UP_SWRETURN,0}};
int main(void){apollo_overlay_state s;cm_native_io io;agc_hardware hw;enum exec_status st;unsigned n,i,ended[14]={0},callers[2]={0};
 check(CM_UP_VECTORS==CM_UP_CASES,"vector count",0);
 /* The C64 rows (chosen by outcome in the freeze) are host cases with their digests. */
 for(n=0;n<CM_UP_C64_COUNT;++n)check(cm_up_c64_case[n]<CM_UP_CASES&&(n==0u||cm_up_c64_case[n-1u]<cm_up_c64_case[n])&&cm_up_c64[n]==cm_up_expected[cm_up_c64_case[n]],"C64 row",n);
 for(n=1;n<CM_UPDATE_OPERAND_COUNT;++n)check(cm_update_operands[n-1u].physical<cm_update_operands[n].physical,"operands sorted",n);
 /* The CADRs after the file's BANKCALL GOXDSPF and POSTJUMP come from the interbank site table. */
 {static const struct { uint16_t physical; agc_word word; } cadrs[5]={
   {027u*1024u+(03420u-02000u),020561u},{027u*1024u+(03444u-02000u),020561u},{027u*1024u+(03461u-02000u),020561u},
   {043u*1024u+(03752u-02000u),057373u},{043u*1024u+(03765u-02000u),057754u}};uint16_t count;const agc_site_operand *t=cm_interbank_site_table(&count);
  for(n=0;n<5u;++n){agc_word w=0;check(agc_site_lookup(t,count,cadrs[n].physical,&w)&&w==cadrs[n].word,"interbank CADR",n);}}
 for(n=1;n<CM_UP_ROUTINES;++n)check(cm_update_routines.entries[n-1u].physical<cm_update_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<CM_UP_CASES;++n)check(cm_up_digest(&s,(uint16_t)n,&st)==cm_up_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[CM_UP_ROUTINES];agc_routine_table tables[2];
  agc_native_program p={0,2,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<CM_UP_ROUTINES;++i){wrapped[i].physical=cm_update_routines.entries[i].physical;wrapped[i].call=count;}
  tables[0].entries=wrapped;tables[0].count=CM_UP_ROUTINES;tables[1]=cm_interbank_routines;p.basic=tables;p.hardware=&hw;p.operands=cm_interbank_site_table(&p.operand_count);
  for(n=0;n<CM_UP_CASES;++n){unsigned bank;
   cm_up_input(&s,&io,&hw,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.superbank,CM_UP_SWRETURN_POINT,24,&steps);
   check(st==EXEC_UNMAPPED||(st==EXEC_OK&&s.words[5]==CM_UP_SWRETURN),"coverage run",n);
   bank=s.words[4]>>10;if(bank>=030u&&(io.superbank&0100u))bank+=010u;
   for(i=0;i<14u;++i)if(s.words[5]==ends[i].z&&(!ends[i].bank||bank==ends[i].bank))break;
   check(i<14u,"end point",n);if(i<14u)++ended[i];
   /* UPACTOFF from a BANKCALL: back at SWRETURN with Q = 3. */
   if(cm_up_entries[n%CM_UP_ROUTINES].z==03757u){unsigned c=(n/CM_UP_ROUTINES)&1u;
    check(s.words[5]==CM_UP_SWRETURN&&s.words[2]==3u,"UPACTOFF returns to its BANKCALL",n);++callers[c];}}
  check(callers[0]>0u&&callers[1]>0u,"both UPACTOFF callers",0);
  for(i=0;i<CM_UP_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<14u;++i)check(ended[i]>0u,"end reached",i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,&hw,03613u);s.words[CM_UP_UPTEMP]=03535u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"TC UPTEMP word",0);
 at(&s,&io,&hw,03621u);s.words[CM_UP_UPTEMP]=03536u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"INDEX UPTEMP; TC 1 word",0);
 at(&s,&io,&hw,03404u);s.words[CM_UP_UPVERB]=4u;check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"UPVERB with no branch",0);
 at(&s,&io,&hw,03553u);s.words[CM_UP_UPVERB]=1u;s.words[CM_UP_COMPNUMB]=3u;s.words[CM_UP_UPBUFF+1u]=040u; /* EB 0, S 40: TIME2 */
 check(once(&s,&io,&hw)==EXEC_OUT_OF_SCOPE,"update into a register or counter",0);
 at(&s,&io,&hw,03373u);check(once(&s,0,&hw)==EXEC_OUT_OF_SCOPE,"no io",0);
 at(&s,&io,&hw,03706u);check(once(&s,&io,0)==EXEC_OUT_OF_SCOPE,"UPACTOFF without hardware",0);
 printf("cm-update: %u cases, %u entries, ends",CM_UP_CASES,CM_UP_ROUTINES);for(i=0;i<14u;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
