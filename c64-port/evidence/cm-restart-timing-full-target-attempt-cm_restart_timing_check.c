/* SPDX-License-Identifier: MIT */
#include "cm_restart_timing_cases.h"
#include "cm_restart_timing_vectors.h"
#include "cm_restart_timing_check.h"
static apollo_overlay_state state;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_rstime_check(void){unsigned n,i,j,fail=0;
 for(n=0;n<4096u;++n){uint32_t h;enum cm_restart_time_exit status;
  cm_rstime_input(&state,(uint16_t)n);status=n<2048u?cm_restart_wait_delay(&state,(uint8_t)(n%6u)):cm_restart_long_delay(&state);
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);
  if(h!=cm_rstime_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_rstime_sample_indices[j]){
   if(cm_rstime_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_rstime_samples[j][i+1u])++fail;
  }
 }return fail;
}
