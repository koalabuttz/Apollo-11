/* SPDX-License-Identifier: MIT */
#include "cm_executive_check.h"
#include "cm_executive_cases.h"
#include "cm_executive_vectors.h"
static uint32_t add(uint32_t h,uint16_t w) {
 h=(h^(uint8_t)w)*UINT32_C(16777619);
 return (h^(uint8_t)(w>>8))*UINT32_C(16777619);
}
int cm_case_check(unsigned n) {
 cm_scenario c; int result; uint8_t i,j; unsigned k=0,t;
 uint32_t h; uint16_t words[CM_EXEC_STATE_WORDS];
 if(n>=CM_EXEC_CASE_COUNT) return 0;
 cm_case_prepare(&c,(uint16_t)n);
 result=c.complete?cm_exec_endjob_select(&c.state):(int)cm_exec_admit(&c.state,c.priority,c.location,c.bank,c.needs_vac);
 for(i=0;i<CM_EXEC_CORE_COUNT;++i) for(j=0;j<CM_EXEC_CORE_WORDS;++j) words[k++]=c.state.core[i][j];
 for(i=0;i<CM_EXEC_VAC_COUNT;++i) for(j=0;j<CM_EXEC_VAC_WORDS;++j) words[k++]=c.state.vac[i][j];
 words[k++]=c.state.newjob; words[k++]=c.state.newprio;
 words[k++]=c.state.newloc[0]; words[k++]=c.state.newloc[1];
 words[k++]=c.state.locctr; words[k++]=c.state.fixloc;
 words[k++]=c.state.ovfind; words[k++]=c.state.alarm;
 if(k!=CM_EXEC_STATE_WORDS) return 0;
 h=add(UINT32_C(2166136261),(uint16_t)result);
 for(k=0;k<CM_EXEC_STATE_WORDS;++k) h=add(h,words[k]);
 if(h!=cm_case_hashes[n]) return 0;
 for(t=0;t<CM_EXEC_TARGET_COUNT;++t) if(cm_full_indices[t]==n) {
  if(result!=cm_full_results[t]) return 0;
  for(k=0;k<CM_EXEC_STATE_WORDS;++k) if(words[k]!=cm_full_words[t][k]) return 0;
 }
 return 1;
}
