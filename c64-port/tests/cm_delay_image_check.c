/* SPDX-License-Identifier: MIT */
#include "cm_delay_image_cases.h"
#include "cm_delay_image_vectors.h"
#include "cm_delay_image_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_delay_image_check(void){unsigned n,run,i,j,fail=0;
 for(run=0;run<
#ifdef __mos__
 128u
#else
 1024u
#endif
 ;++run){uint32_t h;enum cm_wl_status status;
#ifdef __mos__
  n=(run/64u)*512u+((run%64u)*17u)%512u;
#else
  n=run;
#endif
  cm_delay_image_input(&state,&io,(uint16_t)n);status=cm_delay_image(&state,&io,500,(uint8_t)(n<512u));
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
  if(h!=cm_delay_image_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_delay_image_sample_indices[j]){
   if(cm_delay_image_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_delay_image_samples[j][i+1u])++fail;
   if(io.superbank!=cm_delay_image_samples[j][2049]||io.interrupt_enabled!=cm_delay_image_samples[j][2050])++fail;
  }
 }return fail;
}
