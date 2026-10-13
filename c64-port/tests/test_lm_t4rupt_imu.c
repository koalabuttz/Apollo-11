/* SPDX-License-Identifier: MIT */
/* lm-t4rupt-imu: the IMU monitor of Luminary099 T4RUPT_PROGRAM.agc (IMUMON
 * through SETISSW, CAGESUB and LAMPTEST) as native routines, against digests
 * frozen from the original program; every entry returns a continuation the
 * original leaves, and every external service is reached; inputs no
 * original producer supplies are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_t4rupt_imu_cases.h"
#include "lm_t4rupt_imu_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* RESUME, ALARM, VARALARM, WAITLIST, VARDELAY, TASKOVER, IBNKCALL, POSTJUMP, ZEROICDU. */
static const agc_word external[9]={05270u,05567u,05744u,05203u,05224u,05261u,04674u,04635u,05457u};
static unsigned reached[10];
static void refused(uint16_t n,unsigned word,agc_word value,const char *what){
 static apollo_overlay_state s,before;lti_io io,io0;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 lti_input(&s,&io,n);if(word<2048u)s.words[word]=value;else io.hw.channel14|=value;before=s;io0=io;
 program.basic=&lm_t4rupt_imu_routines;program.basic_count=1;program.hardware=&io.hw;
 call=agc_transfer_resolve(&program,&s,io.lm.superbank);
 check(call&&call(&program,&s,&io.lm)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),what,n);
}
static uint16_t first(agc_word z){uint16_t n;for(n=0;n<LTI_CASES;++n)if(lti_entry(n)==z)return n;return 0;}
int main(void){static apollo_overlay_state s;unsigned n,i,kind[3]={0,0,0};
 for(n=0;n<LTI_CASES;++n){static lti_io io;unsigned k;agc_word z;
  check(lti_digest(&s,(uint16_t)n)==lti_expected[n],"digest",n);
  k=lti_run(&s,&io,(uint16_t)n);check(k==LTI_RETURNED,"outcome",n);if(k<3u)++kind[k];
  z=s.words[5];for(i=0;i<9u;++i)if(z==external[i])break;
  /* Otherwise SETISSW's TC ITEMP6 back to a caller in fixed-fixed. */
  if(i==9u)check((lti_entry((uint16_t)n)==02703u||lti_entry((uint16_t)n)==02731u)&&z>=04000u&&z<010000u,"continuation",n);
  ++reached[i];}
 for(i=0;i<10u;++i)check(reached[i]>0u,"continuation reached",i);
 /* Inputs no producer supplies. */
 {uint16_t n0=first(02555u);
  refused(n0,071u,1u,"IFAILJMP scan past the table");
  refused(n0,071u,040000u,"bit scan with a negative RUPTREG2");}
 refused(first(02670u),071u,1u,"C33JMP scan past the table");
 refused(first(02611u),2048u,010000u,"RNDREFDR return with the CDU drive on");
 refused(first(02731u),066u,01777u,"SETISSW return to erasable");
 /* Another bank names another routine: nothing resolves. */
 {static lti_io io;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
  lti_input(&s,&io,0);s.words[4]=016000u;s.words[6]=(agc_word)(016000u|(s.words[3]>>8));program.basic=&lm_t4rupt_imu_routines;program.basic_count=1;program.hardware=&io.hw;
  check(!agc_transfer_resolve(&program,&s,io.lm.superbank),"IMUMON address outside bank 06 unresolved",0);}
 printf("lm-t4rupt-imu: %u cases (%u returned, %u refused); to RESUME %u, ALARM %u, VARALARM %u, WAITLIST %u, VARDELAY %u, TASKOVER %u, IBNKCALL %u, POSTJUMP %u, ZEROICDU %u, SETISSW caller %u; %u checks, %u failures\n",
  LTI_CASES,kind[0],kind[2],reached[0],reached[1],reached[2],reached[3],reached[4],reached[5],reached[6],reached[7],reached[8],reached[9],checks,failures);
 return failures!=0u;
}
