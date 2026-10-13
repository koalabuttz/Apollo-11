/* SPDX-License-Identifier: MIT */
/* t4rupt-core: the display and dispatch core of T4RUPT_PROGRAM.agc in
 * Luminary099 and Comanche055 as native routines, against digests frozen
 * from the original programs; every entry, continuation and display path
 * is reached; inputs no original producer supplies are refused with the
 * image unchanged. */
#include <stdio.h>
#include <string.h>
#include "t4rupt_core_cases.h"
#include "t4rupt_core_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* Continuations: RESUME, NOVAC (PROCKEY), the four T4JUMP targets. Display
 * paths of the 120 ms phase: a relay word written (HANG20, 20 ms) or none
 * (NODSPOUT, 120 ms); of the 20 ms phase: QUIKOFF, or the quick display. */
enum {TO_RESUME,TO_NOVAC,TO_J0,TO_J1,TO_J2,TO_J3,TO_KINDS};
enum {P_HANG20,P_NODSPOUT,P_QUIKOFF,P_QUICK,PATHS};
static unsigned to[2][TO_KINDS],path[2][PATHS];
static const agc_word resume[2]={05270u,05333u},novac[2]={05072u,05134u},jump[2][4]={{03156u,03006u,02172u,03132u},{04740u,02765u,02174u,05333u}};
static void refused(unsigned cm,uint16_t n,unsigned word,agc_word value,const char *what){
 static apollo_overlay_state s,before;t4_io io,io0;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 t4_input(cm,&s,&io,n);if(word<2048u)s.words[word]=value;else io.hw.in_interrupt=0;before=s;io0=io;
 program.basic=cm?&cm_t4rupt_core_routines:&lm_t4rupt_core_routines;program.basic_count=1;program.hardware=&io.hw;
 call=agc_transfer_resolve(&program,&s,cm?io.cm.superbank:io.lm.superbank);
 check(call&&call(&program,&s,cm?(void *)&io.cm:(void *)&io.lm)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),what,n);
}
int main(void){static apollo_overlay_state s,in;unsigned cm,n,i,kind[3]={0,0,0};
 for(cm=0;cm<2u;++cm){const uint32_t *expected=cm?t4_cm_expected:t4_lm_expected;agc_word dsruptsw=cm?01302u:01313u;
  for(n=0;n<t4_cases(cm);++n){static t4_io io;unsigned k;agc_word z;
   t4_input(cm,&in,&io,(uint16_t)n);
   check(t4_digest(cm,&s,(uint16_t)n)==expected[n],"digest",n);
   k=t4_run(cm,&s,&io,(uint16_t)n);check(k==T4_RETURNED,"outcome",n);if(k<3u)++kind[k];
   z=s.words[5];
   {agc_word d=in.words[dsruptsw];int t4=t4_entry(cm,(uint16_t)n)==02000u,quick=t4&&(d&040000u)&&d!=077777u;
    if(quick){check(z==resume[cm],"quick display ends at RESUME",n);++to[cm][TO_RESUME];++path[cm][(d&020000u)?P_QUICK:P_QUIKOFF];}
    else{for(i=0;i<4u;++i)if(z==jump[cm][i])break;
     if(i<4u)++to[cm][TO_J0+i];else{check(z==novac[cm],"continuation",n);++to[cm][TO_NOVAC];}
     if(t4)++path[cm][s.words[027]==037776u?P_HANG20:P_NODSPOUT];}}}}
 for(cm=0;cm<2u;++cm){for(i=0;i<TO_KINDS;++i)check(to[cm][i]>0u,"continuation reached",i);for(i=0;i<PATHS;++i)check(path[cm][i]>0u,"path reached",i);}
 /* Inputs no producer supplies. */
 refused(0,0,01313u,9u,"LM DSRUPTSW past the T4JUMP table");
 refused(1,1,01023u,0u,"CM display word +0");
 refused(0,2,01030u,077777u,"LM display word -0");
 refused(1,3,0776u,013u,"CM DSPCNT past the display table");
 refused(0,(uint16_t)T4_T4RUPT_CASES,070u,8u,"LM T4JUMP with RUPTREG1 past the table");
 refused(1,4,2048u,0,"CM T4RUPT outside the interrupt");
 printf("t4rupt-core: %u LM and %u CM cases (%u returned, %u refused); LM to RESUME %u, NOVAC %u, RCSMONIT %u, RRAUTCHK %u, IMUMON %u, DAPT4S %u; CM to RESUME %u, NOVAC %u, OPTTEST %u, OPTMON %u, IMUMON %u, RESUME (T4JUMP) %u; "
  "LM HANG20 %u, NODSPOUT %u, QUIKOFF %u, quick display %u; CM %u, %u, %u, %u; %u checks, %u failures\n",T4_LM_CASES,T4_CM_CASES,kind[0],kind[2],
  to[0][0],to[0][1],to[0][2],to[0][3],to[0][4],to[0][5],to[1][0],to[1][1],to[1][2],to[1][3],to[1][4],to[1][5],
  path[0][0],path[0][1],path[0][2],path[0][3],path[1][0],path[1][1],path[1][2],path[1][3],checks,failures);
 return failures!=0u;
}
