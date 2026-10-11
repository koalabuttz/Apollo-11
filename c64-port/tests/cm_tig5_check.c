/* SPDX-License-Identifier: MIT */
#include "cm_tig5_cases.h"
#include "cm_tig5_vectors.h"
#include "cm_tig5_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_tig5_check(void){unsigned n,i,j,fail=0;
 for(n=0;n<1536u;
#ifdef __mos__
 n+=8u
#else
 ++n
#endif
 ){uint32_t h;enum cm_tig5_exit status;
  cm_tig5_input(&state,&io,(uint16_t)n);status=cm_tig5_phase(&state,&io,(uint8_t)(n/512u));
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
  if(h!=cm_tig5_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_tig5_sample_indices[j]){
   if(cm_tig5_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_tig5_samples[j][i+1u])++fail;
   if(io.superbank!=cm_tig5_samples[j][2049]||io.interrupt_enabled!=cm_tig5_samples[j][2050])++fail;
  }
 }return fail;
}
