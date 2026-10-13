/* SPDX-License-Identifier: MIT */
/* cm-exec-jobs: Comanche055 SPVACIN, SPVAC, CHANG1, CHANG2, JOBSLEEP,
 * JOBWAKE, PRIOCHNG and ENDOFJOB as native routines, against digests frozen
 * from the original program; every routine and every kind of continuation is
 * reached; inputs no original producer supplies are refused with the image
 * unchanged. */
#include <stdio.h>
#include <string.h>
#include "cm_exec_jobs_cases.h"
#include "cm_exec_jobs_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static const agc_word entries[8]={CM_EXEC_SPVACIN,CM_EXEC_SPVAC,CM_EXEC_CHANG1,CM_EXEC_CHANG2,CM_EXEC_JOBSLEEP,CM_EXEC_JOBWAKE,CM_EXEC_PRIOCHNG,CM_EXEC_ENDOFJOB};
/* Continuations: a basic job or caller, NEWOPS (interpretive job), ADVAN
 * (idle), TC 014, CCSHOLE, BAILOUT. */
enum {TO_BASIC,TO_NEWOPS,TO_ADVAN,TO_014,TO_CCSHOLE,TO_BAILOUT,TO_KINDS};
static unsigned reached[8],to[TO_KINDS];
static enum exec_status run_from(apollo_overlay_state *s,cm_native_io *io){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&cm_exec_job_routines;program.basic_count=1;
 call=agc_transfer_resolve(&program,s,io->superbank);
 return call?call(&program,s,io):EXEC_UNMAPPED;
}
/* A refusal leaves the image and io as they were. */
static void refused(apollo_overlay_state *s,cm_native_io *io,const char *what){apollo_overlay_state before=*s;cm_native_io io0=*io;
 check(run_from(s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,0);
}
int main(void){apollo_overlay_state s;cm_native_io io;unsigned n,i,kind[3]={0,0,0};agc_word z;
 for(n=0;n<CM_EXJ_CASES;++n){unsigned k;
  cm_exj_input(&s,&io,(uint16_t)n);
  for(i=0;i<8u;++i)if(s.words[5]==entries[i])++reached[i];
  k=cm_exj_run(&s,&io,(uint16_t)n);
  check(cm_exj_digest(&s,(uint16_t)n)==cm_exj_expected[n],"digest",n);
  check(k<2u,"outcome",n);if(k<3u)++kind[k];
  (void)cm_exj_run(&s,&io,(uint16_t)n);z=s.words[5];
  ++to[z==CM_EXEC_NEWOPS?TO_NEWOPS:z==CM_EXEC_ADVAN&&s.words[4]==02000u?TO_ADVAN:z==014u?TO_014:z==05747u?TO_CCSHOLE:z==CM_EXEC_BAILOUT?TO_BAILOUT:TO_BASIC];}
 for(i=0;i<8u;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<TO_KINDS;++i)check(to[i]>0u,"continuation reached",i);
 /* Inputs no producer supplies. */
 cm_exj_input(&s,&io,(uint16_t)cm_exj_first(CM_EXJ_SPVAC));io.interrupt_enabled=1;refused(&s,&io,"SPVAC with interrupts enabled");
 cm_exj_input(&s,&io,(uint16_t)cm_exj_first(CM_EXJ_JOBSLEEP));s.words[0167]=0;refused(&s,&io,"JOBSLEEP with no current priority");
 cm_exj_input(&s,&io,(uint16_t)cm_exj_first(CM_EXJ_ENDOFJOB));s.words[3]=01001u;refused(&s,&io,"ENDOFJOB with an incoherent EB");
 cm_exj_input(&s,&io,(uint16_t)cm_exj_first(CM_EXJ_PRIOCHNG));s.words[0]=0177777u;refused(&s,&io,"PRIOCHNG with a negative priority");
 printf("cm-exec-jobs: %u cases (%u returned, %u alarm, %u refused); routines",CM_EXJ_CASES,kind[0],kind[1],kind[2]);
 for(i=0;i<8u;++i)printf(" %u",reached[i]);
 printf("; to job/caller %u, NEWOPS %u, ADVAN %u, TC 014 %u, CCSHOLE %u, BAILOUT %u; %u checks, %u failures\n",to[0],to[1],to[2],to[3],to[4],to[5],checks,failures);
 return failures!=0u;
}
