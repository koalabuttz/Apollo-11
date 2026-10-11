/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_extended_cases.h"
#include "cm_waitlist_extended_vectors.h"
#include "cm_waitlist_extended_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_waitlist_extended_check(void){unsigned n,run,i,j,fail=0;
 for(run=0;run<
#ifdef __mos__
 128u
#else
 2048u
#endif
 ;++run){uint32_t h;enum cm_wl_status status;
#ifdef __mos__
  n=(run/64u)*1024u+((run%64u)*17u)%1024u;
#else
  n=run;
#endif
  cm_waitlist_extended_input(&state,&io,(uint16_t)n);status=cm_waitlist_extended(&state,&io,(exec_address){03207,032066},(uint8_t)(n>=1024u));
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
  if(h!=cm_waitlist_extended_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_waitlist_extended_sample_indices[j]){
   if(cm_waitlist_extended_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_waitlist_extended_samples[j][i+1u])++fail;
   if(io.superbank!=cm_waitlist_extended_samples[j][2049]||io.interrupt_enabled!=cm_waitlist_extended_samples[j][2050])++fail;
  }
 }return fail;
}
