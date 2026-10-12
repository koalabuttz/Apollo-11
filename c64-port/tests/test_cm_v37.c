/* SPDX-License-Identifier: MIT */
/* cm-v37: Comanche055 FRESH_START_AND_RESTART bank 04 (V37, R00, P00,
 * INITSUB) and bank 20 (VAC5STOR) cases through the trampoline, against
 * digests frozen from the original program; the C64 rows are host cases;
 * every registered entry is reached and each run ends at one of the 29
 * points outside the milestone it can reach (and each of them is reached);
 * a negative MMNUMBER, an index outside the 33-word tables, a TC MPAC +1
 * word INITSUB never stores and a missing io record are refused. */
#include <stdio.h>
#include "cm_v37_cases.h"
#include "cm_v37_vectors.h"
#include "cm_v37_c64_vectors.h"
#include "cm_interbank_routines.h"
#define ENDS 29u
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned reached[CM_V37_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned i;uint16_t at=agc_transfer_point(p,s,((cm_native_io *)io)->superbank);
 for(i=0;i<CM_V37_ROUTINES;++i)if(cm_v37_routines.entries[i].physical==at){++reached[i];return cm_v37_routines.entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,cm_fresh_io *io){
 agc_native_program p={&cm_v37_routines,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.operands=cm_interbank_site_table(&p.operand_count);call=agc_transfer_resolve(&p,s,io?io->control.superbank:0);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The first case entering at z. */
static void at(apollo_overlay_state *s,cm_fresh_io *io,agc_word z){unsigned k;
 for(k=0;k<CM_V37_ROUTINES&&cm_v37_entries[k].z!=z;++k){}
 cm_v37_input(s,io,(uint16_t)k);
}
/* Where a run may end: Z, and the bank for switched addresses. */
static const struct { agc_word z; uint8_t bank; const char *name; } ends[ENDS]={
 {03732u,0,"DELAYJOB"},{04400u,0,"FALTON"},{04473u,0,"RELDSP"},{04526u,0,"STOPRATE"},{04675u,0,"SWRETURN"},
 {04753u,0,"ISWRETRN"},{05160u,0,"SPVAC"},{05217u,0,"ENDOFJOB"},{05245u,0,"WAITLIST"},{05357u,0,"NEWMODEA"},
 {05360u,0,"MMDSPLAY"},{05412u,0,"PHASCHNG"},{05536u,0,"CLEARMRK"},{05540u,0,"CLEARMRK +2"},{05546u,0,"UPFLAG"},
 {05560u,0,"DOWNFLAG"},{05606u,0,"GENTRAN"},{05650u,0,"ALARM"},{06006u,0,"INTPRET"},{03225u,01u,"DUMMYJOB +2"},
 {03366u,05u,"MASSPROP"},{02720u,010u,"GOFLASH"},{03272u,010u,"PINBRNCH"},{02025u,021u,"RCSDAPON +1"},{02304u,024u,"SETMINDB"},
 {02320u,024u,"SETMAXDB"},{03017u,024u,"SPSOFF"},{03071u,024u,"TVCZAP"},{03757u,027u,"UPACTOFF"}};
int main(void){apollo_overlay_state s;cm_fresh_io io;enum exec_status st;unsigned n,i,ended[ENDS];
 for(i=0;i<ENDS;++i)ended[i]=0;
 check(CM_V37_VECTORS==CM_V37_CASES,"vector count",0);
 /* The C64 rows (chosen by outcome in the freeze) are host cases with their digests. */
 for(n=0;n<CM_V37_C64_COUNT;++n)check(cm_v37_c64_case[n]<CM_V37_CASES&&(n==0u||cm_v37_c64_case[n-1u]<cm_v37_c64_case[n])&&cm_v37_c64[n]==cm_v37_expected[cm_v37_c64_case[n]],"C64 row",n);
 for(n=1;n<CM_V37_ROUTINES;++n)check(cm_v37_routines.entries[n-1u].physical<cm_v37_routines.entries[n].physical,"routines sorted",n);
 for(n=0;n<CM_V37_CASES;++n)check(cm_v37_digest(&s,(uint16_t)n,&st)==cm_v37_expected[n]&&st==EXEC_OK,"oracle",n);
 {agc_routine_entry wrapped[CM_V37_ROUTINES],interbank[16];agc_routine_table tables[3];uint16_t m=0;
  agc_native_program p={0,3,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<CM_V37_ROUTINES;++i){wrapped[i].physical=cm_v37_routines.entries[i].physical;wrapped[i].call=count;}
  for(i=0;i<cm_interbank_routines.count&&m<16u;++i)if(cm_interbank_routines.entries[i].physical!=CM_V37_ISWRETRN_POINT)interbank[m++]=cm_interbank_routines.entries[i];
  tables[0].entries=wrapped;tables[0].count=CM_V37_ROUTINES;tables[1]=cm_fresh_start_routines;tables[2].entries=interbank;tables[2].count=m;
  p.basic=tables;p.operands=cm_interbank_site_table(&p.operand_count);
  for(n=0;n<CM_V37_CASES;++n){unsigned bank;agc_word z;
   cm_v37_input(&s,&io,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.control.superbank,CM_V37_SWRETURN_POINT,32,&steps);
   check(st==EXEC_UNMAPPED||st==EXEC_OK,"coverage run",n);
   z=s.words[5];bank=s.words[4]>>10;if(bank>=030u&&(io.control.superbank&0100u))bank+=010u;
   for(i=0;i<ENDS;++i)if(z==ends[i].z&&(!ends[i].bank||bank==ends[i].bank))break;
   check(i<ENDS,"end point",n);if(i<ENDS)++ended[i];
   /* SWRETURN is VAC5STOR's return to its BANKCALL; ISWRETRN a KLEAN entry's with Q = 3. */
   if(z==04675u)check(st==EXEC_OK&&s.words[2]==02064u,"VAC5STOR returns through SWRETURN",n);
   if(z==04753u)check(st==EXEC_UNMAPPED&&s.words[2]==3u,"KLEAN returns to ISWRETRN with Q = 3",n);}
  for(i=0;i<CM_V37_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<ENDS;++i)check(ended[i]>0u,"end reached",i);}
 /* Refusals, each entered once directly. */
 at(&s,&io,02157u);s.words[CM_V37_MMNUMBER]=077000u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"negative MMNUMBER",0);
 at(&s,&io,02266u);s.words[CM_V37_MINDEX]=041u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"MINDEX past DNLADMM1",0);
 at(&s,&io,02274u);s.words[CM_V37_MINDEX]=077776u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"negative MINDEX into PREMM1",0);
 at(&s,&io,02364u);s.words[0155]=02002u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"TC MPAC +1 word INITSUB never stores",0);
 at(&s,&io,02011u);check(once(&s,0)==EXEC_OUT_OF_SCOPE,"no io",0);
 printf("cm-v37: %u cases, %u entries, ends",CM_V37_CASES,CM_V37_ROUTINES);for(i=0;i<ENDS;++i)printf(" %u",ended[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
