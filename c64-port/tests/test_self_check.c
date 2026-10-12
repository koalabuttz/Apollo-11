/* SPDX-License-Identifier: MIT */
/* self-check: AGC_BLOCK_TWO_SELF-CHECK (bank 43) of Luminary099 and
 * Comanche055 through the trampoline, against digests frozen from the
 * original programs; the C64 rows are host cases; every registered entry
 * is reached in both missions and each run ends at one of the routines the
 * file calls (with Q at the return point), ENDSUMS, FXADRS or a TC SFAIL /
 * TC SKEEP1 target outside the file (and each named end is reached); a
 * missing io record, another FB bank, a SUPERBNK that does not select bank
 * 43, a TC through SFAIL to an erasable word or into the middle of the file
 * and an ERASCHK pair over the central registers are refused. */
#include <stdio.h>
#include "self_check_cases.h"
#include "lm_self_check_vectors.h"
#include "lm_self_check_c64_vectors.h"
#include "cm_self_check_vectors.h"
#include "cm_self_check_c64_vectors.h"
#define ENDS 8u
#define K(z) ((agc_word)((z)-(mission?0u:2u)))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static unsigned mission,reached[SELF_CHECK_ROUTINES];
static enum exec_status count(const agc_native_program *p,apollo_overlay_state *s,void *io){
 const agc_routine_table *t=self_check_table(mission);unsigned i;uint16_t at=agc_transfer_point(p,s,((p66_bias_io *)io)->superbank);
 for(i=0;i<SELF_CHECK_ROUTINES;++i)if(t->entries[i].physical==at){++reached[i];return t->entries[i].call(p,s,io);}
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status once(apollo_overlay_state *s,p66_bias_io *io){
 agc_native_program p={0,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 p.basic=self_check_table(mission);call=agc_transfer_resolve(&p,s,io?io->superbank:0100u);
 return call?call(&p,s,io):EXEC_UNMAPPED;
}
/* The first case entering at Comanche055 address z. */
static void at(apollo_overlay_state *s,p66_bias_io *io,agc_word z){unsigned k;
 for(k=0;k<SELF_CHECK_ROUTINES&&self_check_entry_list(mission)[k]!=K(z);++k){}
 self_check_input(mission,s,io,(uint16_t)k);
}
/* The named ends (Luminary099, Comanche055) and the return point the TC there leaves in Q (0: none). */
static const struct { agc_word z[2],q; const char *name; } ends[ENDS]={
 {{05571u,05652u},03267u,"ALARM2"},{{04635u,04701u},03337u,"POSTJUMP"},{{04651u,04715u},03542u,"SUPDACAL"},
 {{05122u,05164u},03625u,"CHANG1"},{{04331u,04345u},03675u,"LEFT5"},{{03113u,02773u},03713u,"SDISPLAY"},
 {{03131u,03011u},0u,"ENDSUMS"},{{03557u,03561u},0u,"FXADRS"}};
int main(void){apollo_overlay_state s;p66_bias_io io;enum exec_status st;unsigned n,i,outside=0,ended[ENDS];
 const uint32_t *expected[2]={lm_self_check_expected,cm_self_check_expected};
 check(LM_SELF_CHECK_VECTORS==SELF_CHECK_CASES&&CM_SELF_CHECK_VECTORS==SELF_CHECK_CASES,"vector count",0);
 /* The C64 rows (chosen by outcome in the freeze) are host cases with their digests. */
 for(n=0;n<LM_SELF_CHECK_C64_COUNT;++n)check(lm_self_check_c64_case[n]<SELF_CHECK_CASES&&(n==0u||lm_self_check_c64_case[n-1u]<lm_self_check_c64_case[n])&&lm_self_check_c64[n]==lm_self_check_expected[lm_self_check_c64_case[n]],"LM C64 row",n);
 for(n=0;n<CM_SELF_CHECK_C64_COUNT;++n)check(cm_self_check_c64_case[n]<SELF_CHECK_CASES&&(n==0u||cm_self_check_c64_case[n-1u]<cm_self_check_c64_case[n])&&cm_self_check_c64[n]==cm_self_check_expected[cm_self_check_c64_case[n]],"CM C64 row",n);
 for(mission=0;mission<2u;++mission){agc_routine_entry wrapped[SELF_CHECK_ROUTINES];agc_routine_table table;
  agc_native_program p={0,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};uint16_t steps;
  for(i=0;i<SELF_CHECK_ROUTINES;++i)check(self_check_table(mission)->entries[i].physical==043u*1024u+self_check_entry_list(mission)[i]-02000u&&(i==0u||self_check_table(mission)->entries[i-1u].physical<self_check_table(mission)->entries[i].physical),"routines sorted",i);
  for(n=0;n<SELF_CHECK_CASES;++n)check(self_check_digest(mission,&s,(uint16_t)n,&st)==expected[mission][n]&&st==EXEC_OK,"oracle",n);
  for(i=0;i<SELF_CHECK_ROUTINES;++i){wrapped[i].physical=self_check_table(mission)->entries[i].physical;wrapped[i].call=count;reached[i]=0;}
  for(i=0;i<ENDS;++i)ended[i]=0;
  table.entries=wrapped;table.count=SELF_CHECK_ROUTINES;p.basic=&table;
  for(n=0;n<SELF_CHECK_CASES;++n){agc_word z;
   self_check_input(mission,&s,&io,(uint16_t)n);
   st=agc_transfer_run(&p,&s,&io,&io.superbank,AGC_TRANSFER_NONE,64,&steps);
   check(st==EXEC_UNMAPPED,"coverage run",n);
   z=s.words[5];
   for(i=0;i<ENDS;++i)if(z==ends[i].z[mission])break;
   if(i<ENDS){++ended[i];check(!ends[i].q||s.words[2]==K(ends[i].q),"Q at the return point",n);}
   else{check(z>=04000u||(z>=02000u&&z<K(03234u)),"end point",n);++outside;}}
  for(i=0;i<SELF_CHECK_ROUTINES;++i)check(reached[i]>0u,"entry reached",i);
  for(i=0;i<ENDS;++i)check(ended[i]>0u,"end reached",i);
  printf("self-check %s: %u cases, %u entries, ends",mission?"Comanche055":"Luminary099",SELF_CHECK_CASES,SELF_CHECK_ROUTINES);
  for(i=0;i<ENDS;++i)printf(" %u",ended[i]);
  printf(", %u TC SFAIL/SKEEP1 out of the file\n",outside);
  /* Refusals, each entered once directly. */
  at(&s,&io,03340u);check(once(&s,0)==EXEC_OUT_OF_SCOPE,"no io",mission);
  at(&s,&io,03340u);s.words[4]=032u<<10;s.words[6]=(agc_word)(s.words[4]|(s.words[3]>>8));check(once(&s,&io)!=EXEC_OK,"FB 32",mission);
  at(&s,&io,03340u);io.superbank=0u;check(once(&s,&io)!=EXEC_OK,"SUPERBNK bank 33",mission);
  at(&s,&io,03270u);s.words[01362]=077777u;s.words[01357]=01000u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"TC SFAIL to erasable",mission);
  at(&s,&io,03270u);s.words[01362]=077777u;s.words[01357]=K(03250u);check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"TC SFAIL into the file",mission);
  at(&s,&io,03443u);s.words[01374]=0u;s.words[01377]=01402u;s.words[01373]=01410u;check(once(&s,&io)==EXEC_OUT_OF_SCOPE,"ERASCHK pair over the central registers",mission);}
 printf("self-check: %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
