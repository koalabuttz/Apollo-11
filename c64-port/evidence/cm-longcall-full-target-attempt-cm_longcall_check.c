/* SPDX-License-Identifier: MIT */
#include "cm_longcall_cases.h"
#include "cm_longcall_vectors.h"
#include "cm_longcall_check.h"
static apollo_overlay_state state;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_longcall_check(void){unsigned n,i,j,fail=0;
 for(n=0;n<4096u;++n){uint32_t h;enum cm_longcall_status status;exec_address task;
  cm_longcall_input(&state,&task,(uint16_t)n);status=cm_longcall_phase_run(&state,(enum cm_longcall_phase)(n/1024u),task);
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);
  if(h!=cm_longcall_hash[n])++fail;
  for(j=0;j<4u;++j)if(n==cm_longcall_sample_indices[j]){
   if(cm_longcall_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_longcall_samples[j][i+1u])++fail;
  }
 }return fail;
}
