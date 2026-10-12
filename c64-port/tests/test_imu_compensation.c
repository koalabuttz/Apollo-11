/* SPDX-License-Identifier: MIT */
/* imu-compensation: Luminary099 and Comanche055 IMU_COMPENSATION_PACKAGE as
 * native routines, against digests frozen from the original programs; every
 * routine reaches each exit the source gives it, every round trip returns
 * to its caller, and the one refused input leaves the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "imu_compensation_cases.h"
#include "imu_compensation_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
/* Exits each routine must reach (fixed-fixed Z), in lm_points/cm_points order. */
static const agc_word exits[ICP_ROUTINES][4]={
 {LM_ICP_SWRETURN,LM_ICP_NOVAC},{LM_ICP_SWRETURN},{LM_ICP_BANKCALL},{LM_ICP_BANKCALL},{LM_ICP_ENDOFJOB},{LM_ICP_ENDOFJOB},
 {LM_ICP_BANKCALL,LM_ICP_IBNKCALL,LM_ICP_ENDOFJOB},{LM_ICP_BANKCALL,LM_ICP_ENDOFJOB},{LM_ICP_BANKCALL},
 {LM_ICP_BANKCALL,LM_ICP_ENDOFJOB},{LM_ICP_SWRETURN},
 {CM_ICP_SWRETURN,CM_ICP_NOVAC},{CM_ICP_SWRETURN},{CM_ICP_BANKCALL},{CM_ICP_BANKCALL},{CM_ICP_ENDOFJOB},{CM_ICP_ENDOFJOB},
 {CM_ICP_BANKCALL,CM_ICP_ENDOFJOB},{CM_ICP_BANKCALL},{CM_ICP_BANKCALL,CM_ICP_ENDOFJOB}};
static unsigned quiet[2],reached[ICP_ROUTINES][4],returned[ICP_SITES+ICP_LM_SITES],ended[ICP_SITES+ICP_LM_SITES];
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
int main(void){apollo_overlay_state s,before;icp_case c;unsigned n,i,k;uint16_t ie;
 check(sorted(&lm_imu_compensation_routines)&&lm_imu_compensation_routines.count==ICP_LM_ROUTINES,"LM table",0);
 check(sorted(&cm_imu_compensation_routines)&&cm_imu_compensation_routines.count==ICP_ROUTINES-ICP_LM_ROUTINES,"CM table",0);
 check(ICP_VECTORS==ICP_CASES,"vector count",ICP_VECTORS);
 for(n=0;n<ICP_CASES;++n){unsigned kind=icp_run(&s,(uint16_t)n,&ie);
  check(kind==ICP_STOPPED,"stopped",n);
  check(icp_digest(&s,(uint16_t)n)==icp_expected[n],"digest",n);
  if(n<ICP_ENTRY_CASES){unsigned r=n%ICP_ROUTINES;int known=0;
   for(i=0;i<4u&&exits[r][i];++i)if(s.words[5]==exits[r][i]){++reached[r][i];known=1;}
   check(known,"exit",n);
   /* 1/PIPA with GCOMPSW not negative: full compensation, commands of two
    * pulses or less, back to SWRETURN without NOVAC. */
   if((r==0u||r==ICP_LM_ROUTINES)&&(n/ICP_ROUTINES)%4u!=2u&&s.words[5]==exits[r][0])++quiet[r!=0u];
  }else{k=n<ICP_PLAIN_FIRST?(n-ICP_TRIP_FIRST)%ICP_SITES:ICP_SITES+(n-ICP_PLAIN_FIRST)%ICP_LM_SITES;icp_input(&before,&c,(uint16_t)n);
   if(s.words[5]==c.stop_z&&(s.words[4]&076000u)==c.stop_fb)++returned[k];else ++ended[k];}
 }
 for(n=0;n<ICP_ROUTINES;++n)for(i=0;i<4u&&exits[n][i];++i)check(reached[n][i]>0u,"exit reached",n*10u+i);
 check(quiet[0]>0u&&quiet[1]>0u,"two pulses or less",0);
 /* Every round trip returns to its caller. Through the Executive, LM sites
  * 1-3 (1/PIPA) also reach BAILOUT1; CM sites and the plain LM 1/PIPA trips
  * also stop at NOVAC. Site 0 (GCOMPZER) never calls NOVAC. */
 for(k=0;k<ICP_SITES+ICP_LM_SITES;++k){check(returned[k]>0u,"round trip returned",k);if(k%ICP_SITES&&k!=ICP_SITES)check(ended[k]>0u,"round trip ended",k);}
 /* TEM1 other than +0 or BIT8 after IBNKCALL PIPASR +3: no producer. */
 for(n=7u;n<ICP_ENTRY_CASES;n+=ICP_ROUTINES){agc_native_program program={&lm_imu_compensation_routines,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0};p66_bias_io io={0,1};agc_word sb=0;uint16_t steps;
  icp_input(&s,&c,(uint16_t)n);s.words[0141]=(agc_word)(n&1u?1u:077577u);before=s;
  check(agc_transfer_run(&program,&s,&io,&sb,0xfffeu,4,&steps)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&io.interrupt_enabled==1u,"TEM1 refused",n);}
 printf("imu-compensation: %u cases;",ICP_CASES);
 for(n=0;n<ICP_ROUTINES;++n){printf(" [");for(i=0;i<4u&&exits[n][i];++i)printf("%s%u",i?" ":"",reached[n][i]);printf("]");}
 printf("; round trips");for(k=0;k<ICP_SITES+ICP_LM_SITES;++k)printf(" %u/%u",returned[k],ended[k]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
