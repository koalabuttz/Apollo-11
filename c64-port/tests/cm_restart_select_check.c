/* SPDX-License-Identifier: MIT */
#include "cm_restart_select_cases.h"
#include "cm_restart_select_vectors.h"
#include "cm_restart_select_check.h"
static apollo_overlay_state state;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
#ifdef __mos__
#define CM_SOURCE_CHECK_COUNT 192u
#define CM_SOURCE_CHECK_INDEX(run) ((run/32u)*1024u+(run%32u)*17u)
#else
#define CM_SOURCE_CHECK_COUNT 6144u
#define CM_SOURCE_CHECK_INDEX(run) (run)
#endif
unsigned cm_rsselect_check(void){unsigned n,run,i,j,fail=0;
 for(run=0;run<CM_SOURCE_CHECK_COUNT;++run){uint32_t h;enum cm_restart_select_exit status;
  n=CM_SOURCE_CHECK_INDEX(run);cm_rsselect_input(&state,(uint16_t)n);status=cm_restart_select(&state,n<4096u?CM_RS_SELECT_GROUP:n<5120u?CM_RS_SELECT_PART2:CM_RS_SELECT_TYPEB_TABLE);
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);
  if(h!=cm_rsselect_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_rsselect_sample_indices[j]){
   if(cm_rsselect_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_rsselect_samples[j][i+1u])++fail;
  }
 }return fail;
}
