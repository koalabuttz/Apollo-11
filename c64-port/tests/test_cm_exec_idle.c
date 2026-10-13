/* SPDX-License-Identifier: MIT */
/* cm-exec-idle: Comanche055 DUMMYJOB, ADVAN, NUCHANG2, SUPDXCHZ and SVCT3 as
 * native routines, against digests frozen from the original program; every
 * entry and every kind of continuation is reached; inputs no original
 * producer supplies are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "cm_exec_idle_cases.h"
#include "cm_exec_idle_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static const agc_word entries[9]={CM_EXEC_NUCHANG2,CM_EXEC_DUMMYJOB,CM_EXEC_DUMMYJOB+2u,CM_EXEC_ADVAN,CM_EXEC_SUPDXCHZ,CM_EXEC_SUPDXCHZ+1u,
 CM_EXEC_SVCT3,CM_EXEC_SVCT3_NOVAC_RETURN,CM_EXEC_SVCT3X_RETURN};
/* Continuations: a basic job (or SUPDXCHZ target), NEWOPS (interpretive
 * job), the self-check at SELFRET (bank 033, SUPERBNK 0100), TASKOVER, NOVAC
 * and FIXDELAY. */
enum {TO_BASIC,TO_NEWOPS,TO_SELFCHECK,TO_TASKOVER,TO_NOVAC,TO_FIXDELAY,TO_KINDS};
static unsigned reached[9],to[TO_KINDS];
static enum exec_status run_from(apollo_overlay_state *s,cm_native_io *io){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=&cm_exec_idle_routines;program.basic_count=1;
 call=agc_transfer_resolve(&program,s,io->superbank);
 return call?call(&program,s,io):EXEC_UNMAPPED;
}
/* A refusal leaves the image and io as they were. */
static void refused(apollo_overlay_state *s,cm_native_io *io,const char *what){static apollo_overlay_state before;cm_native_io io0=*io;before=*s;
 check(run_from(s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,0);
}
int main(void){static apollo_overlay_state s;cm_native_io io;unsigned n,i,kind[3]={0,0,0};agc_word z;
 for(n=0;n<CM_EXI_CASES;++n){unsigned k;
  cm_exi_input(&s,&io,(uint16_t)n);
  for(i=0;i<9u;++i)if(s.words[5]==entries[i])++reached[i];
  k=cm_exi_run(&s,&io,(uint16_t)n);
  check(cm_exi_digest(&s,(uint16_t)n)==cm_exi_expected[n],"digest",n);
  check(k<2u,"outcome",n);if(k<3u)++kind[k];
  (void)cm_exi_run(&s,&io,(uint16_t)n);z=s.words[5];
  ++to[z==CM_EXEC_NEWOPS?TO_NEWOPS:io.superbank==0100u&&s.words[4]==066000u&&cm_exi_cohort((uint16_t)n)<CM_EXI_SUPDXCHZ?TO_SELFCHECK:
   z==CM_EXEC_TASKOVER?TO_TASKOVER:z==CM_EXEC_NOVAC?TO_NOVAC:z==CM_EXEC_FIXDELAY?TO_FIXDELAY:TO_BASIC];}
 for(i=0;i<9u;++i)check(reached[i]>0u,"entry reached",i);
 for(i=0;i<TO_KINDS;++i)check(to[i]>0u,"continuation reached",i);
 /* Inputs no producer supplies. */
 cm_exi_input(&s,&io,cm_exi_first(CM_EXI_DUMMYJOB));s.words[CM_EXEC_SELFRET]=0;refused(&s,&io,"DUMMYJOB with no self-check return");
 cm_exi_input(&s,&io,cm_exi_first(CM_EXI_ADVAN));s.words[7]=1;refused(&s,&io,"ADVAN with a nonzero ZERO register");
 cm_exi_input(&s,&io,cm_exi_first(CM_EXI_NUCHANG2));s.words[0067]=1;refused(&s,&io,"NUCHANG2 with NEWJOB not a core-set offset");
 cm_exi_input(&s,&io,cm_exi_first(CM_EXI_SUPDXCHZ));s.words[0]=agc_extend(077u);s.words[1]=agc_extend(077u);refused(&s,&io,"SUPDXCHZ to a central register");
 cm_exi_input(&s,&io,cm_exi_first(CM_EXI_SVCT3));s.words[6]=0;refused(&s,&io,"SVCT3 outside its bank");
 printf("cm-exec-idle: %u cases (%u returned, %u alarm, %u refused); entries",CM_EXI_CASES,kind[0],kind[1],kind[2]);
 for(i=0;i<9u;++i)printf(" %u",reached[i]);
 printf("; to job %u, NEWOPS %u, self-check %u, TASKOVER %u, NOVAC %u, FIXDELAY %u; %u checks, %u failures\n",to[0],to[1],to[2],to[3],to[4],to[5],checks,failures);
 return failures!=0u;
}
