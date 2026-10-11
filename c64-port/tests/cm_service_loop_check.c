/* SPDX-License-Identifier: MIT */
#include "cm_service_loop_cases.h"
#include "cm_service_loop_vectors.h"
#include "cm_service_loop_check.h"
static apollo_overlay_state state;static cm_native_io io;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
#ifdef __mos__
#define SOURCE_CASES 64u
#define SOURCE_INDEX(run) (((run)%6u)*64u+((run)/6u)*5u)
#define HASH_INDEX(run,n) (run)
#else
#define SOURCE_CASES 384u
#define SOURCE_INDEX(run) (run)
#define HASH_INDEX(run,n) (n)
#endif
unsigned cm_service_loop_check(void){unsigned run,n,step,i,fail=0;
 for(run=0;run<SOURCE_CASES;++run){cm_service_loop r={&state,&io,cm_service_builtin_invoke,0,0,0};n=SOURCE_INDEX(run);cm_service_loop_input(&state,&io,(uint16_t)n);
  for(step=0;step<cm_service_loop_count[n];++step){cm_service_call c;enum cm_service_status status;uint32_t h;
   status=cm_service_identify(&state,&io,&c);if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME){++fail;break;}status=cm_service_step(&r);
   h=fold(2166136261u,(uint16_t)c.group);h=fold(h,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
   if(h!=cm_service_loop_hash[HASH_INDEX(run,n)][step])++fail;
   if(n==0&&step+1u==cm_service_loop_count[n]){if(cm_service_loop_sample[0]!=(uint16_t)c.group||cm_service_loop_sample[1]!=(uint16_t)status)++fail;for(i=0;i<2048u;++i)if(cm_service_loop_sample[i+2u]!=state.words[i])++fail;if(cm_service_loop_sample[2050]!=io.superbank||cm_service_loop_sample[2051]!=io.interrupt_enabled)++fail;}
   if(status!=CM_SERVICE_ADVANCED&&status!=CM_SERVICE_RESUME){++fail;break;}
  }
  if(r.in_callback||r.completed_groups+1u!=cm_service_loop_count[n])++fail;
 }return fail;
}
