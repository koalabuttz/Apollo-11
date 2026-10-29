/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cm_executive_cases.h"
#include "cm_executive_check.h"
int main(void) {
 unsigned n; cm_executive state,before;
 for(n=0;n<CM_EXEC_CASE_COUNT;++n) assert(cm_case_check(n));
 cm_exec_init(&state); before=state;
 assert(cm_exec_admit(&state,0,02000,0,0)==EXEC_OUT_OF_SCOPE);
 assert(memcmp(&state,&before,sizeof state)==0);
 state.newjob=84; before=state;
 assert(cm_exec_admit(&state,02000,02000,0,0)==EXEC_OUT_OF_SCOPE);
 assert(memcmp(&state,&before,sizeof state)==0);
 state.alarm=01201; before=state;
 assert(cm_exec_admit(&state,02000,02000,0,1)==EXEC_BAILOUT_PENDING);
 assert(memcmp(&state,&before,sizeof state)==0);
 puts("PASS: 1536 CM state fingerprints, 40 complete-state samples, unsupported/pending boundaries");
 return 0;
}
