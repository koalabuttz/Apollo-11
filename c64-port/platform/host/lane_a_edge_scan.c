/* SPDX-License-Identifier: MIT */
/* Counts the flight-typical edges in a kernel milestone's case inputs (the
 * state each case starts from), for tools/lane_a_edge_survey.py. One
 * milestone per build (-DM_LM_EXJ, ...). Prints one JSON object. */
#include <stdio.h>
#include <stdlib.h>
#include "agc_word.h"
#include "waitlist.h"
#include "cm_waitlist.h"
#if defined(M_LM_EXJ)
#include "lm_exec_jobs_cases.h"
#define N LM_EXJ_CASES
#define IN(m,n) {p66_bias_io io;lm_exj_case c;lm_exj_input(m,&io,&c,n);}
#elif defined(M_LM_EXS)
#include "lm_exec_services_cases.h"
#define N LM_EXS_CASES
#define IN(m,n) {p66_bias_io io;lm_exs_case c;lm_exs_input(m,&io,&c,n);}
#elif defined(M_LM_EXI)
#include "lm_exec_idle_cases.h"
#define N LM_EXI_CASES
#define IN(m,n) {p66_bias_io io;agc_hardware h;lm_exi_input(m,&io,&h,n);}
#elif defined(M_CM_EXS)
#include "cm_exec_services_cases.h"
#define N CM_EXS_CASES
#define IN(m,n) {cm_native_io io;cm_exs_case c;cm_exs_input(m,&io,&c,n);}
#elif defined(M_CM_EXJ)
#include "cm_exec_jobs_cases.h"
#define N CM_EXJ_CASES
#define IN(m,n) {cm_native_io io;cm_exj_input(m,&io,n);}
#elif defined(M_CM_EXI)
#include "cm_exec_idle_cases.h"
#define N CM_EXI_CASES
#define IN(m,n) {cm_native_io io;cm_exi_input(m,&io,n);}
#elif defined(M_LM_T6)
#include "lm_t6rupt_cases.h"
#define N LM_T6_CASES
#define IN(m,n) {p66_bias_io io;agc_hardware h;lm_t6_input(m,&io,&h,n);}
#endif
static apollo_overlay_state m;
#if defined(M_CM_EXS)||defined(M_CM_EXJ)||defined(M_CM_EXI)
#define END CM_WL_END_LOCATION
#else
#define END WL_END_LOCATION
#endif
int main(void){unsigned n,i,j,c[12]={0};
 for(n=0;n<N;++n){IN(&m,n)
  {agc_word t3=m.words[026],t6=m.words[031];
   if(t3>=037770u)++c[0];
   if(t6>=1u&&t6<=3u)++c[1];
   for(i=1;i<8u;++i)if(m.words[01400u+i]==0){++c[2];break;}
   for(i=1;i<8u;++i)if(m.words[01400u+i]==077776u){++c[3];break;}
   if(m.words[01410u+16u]!=END)++c[4];
   if(m.words[067]==0)++c[5];if(m.words[067]==AGC_NEG_ZERO)++c[6];
   {int busy=0,t=0;for(i=0;i<8u;++i){agc_word p=m.words[0167u+12u*i];if(!(p&AGC_SIGN)){++busy;for(j=i+1;j<8u;++j)if(m.words[0167u+12u*j]==p)t=1;}}if(t)++c[7];if(busy==8)++c[8];}
   {int f=0;for(i=0;i<5u;++i)if(m.words[0400u+44u*i]!=0)f=1;if(!f)++c[9];}}
 }
 printf("{\"cases\": %u, \"TIME3 037770-037777\": %u, \"TIME6 1-3\": %u, \"LST1 tasks due together (+0)\": %u, "
  "\"LST1 tasks 1 cs apart\": %u, \"Waitlist full (9 tasks)\": %u, \"NEWJOB +0\": %u, \"NEWJOB -0\": %u, "
  "\"equal busy priorities\": %u, \"no free core set\": %u, \"no free VAC area\": %u}\n",N,c[0],c[1],c[2],c[3],c[4],c[5],c[6],c[7],c[8],c[9]);
 return 0;}
