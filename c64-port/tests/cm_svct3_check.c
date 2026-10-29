/* SPDX-License-Identifier: MIT */
#include "cm_svct3_cases.h"
#include "cm_svct3_vectors.h"
#include "cm_svct3_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
#ifdef __mos__
#define CM_SOURCE_CHECK_COUNT 192u
#define CM_SOURCE_CHECK_INDEX(run) ((run/64u)*512u+(run%64u)*7u)
#else
#define CM_SOURCE_CHECK_COUNT 1536u
#define CM_SOURCE_CHECK_INDEX(run) (run)
#endif
unsigned cm_svct3_check(void){unsigned n,run,i,j,fail=0;
 for(run=0;run<CM_SOURCE_CHECK_COUNT;++run){uint32_t h;enum cm_svct3_exit status;
  n=CM_SOURCE_CHECK_INDEX(run);cm_svct3_input(&state,&io,(uint16_t)n);status=cm_svct3_phase(&state,&io,(uint8_t)(n/512u));
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
  if(h!=cm_svct3_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_svct3_sample_indices[j]){
   if(cm_svct3_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_svct3_samples[j][i+1u])++fail;
   if(io.superbank!=cm_svct3_samples[j][2049]||io.interrupt_enabled!=cm_svct3_samples[j][2050])++fail;
  }
 }return fail;
}
