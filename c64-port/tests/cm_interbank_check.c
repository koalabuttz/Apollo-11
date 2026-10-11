/* SPDX-License-Identifier: MIT */
#include "cm_interbank_cases.h"
#include "cm_interbank_vectors.h"
#include "cm_interbank_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_interbank_check(void){unsigned n,run,i,j,fail=0;
 for(run=0;run<
#ifdef __mos__
 112u
#else
 7168u
#endif
 ;++run){uint32_t h;agc_word literal;enum exec_status status;
#ifdef __mos__
  n=cm_interbank_indices[run];
#else
  n=run;
#endif
  literal=cm_interbank_input(&state,&io,(uint16_t)n);status=cm_interbank(&state,&io,(enum cm_interbank_entry)(n/512u),literal);
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
  if(h!=cm_interbank_hash[run])++fail;
  for(j=0;j<2u;++j)if(n==cm_interbank_sample_indices[j]){
   if(cm_interbank_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_interbank_samples[j][i+1u])++fail;
   if(io.superbank!=cm_interbank_samples[j][2049]||io.interrupt_enabled!=cm_interbank_samples[j][2050])++fail;
  }
 }return fail;
}
