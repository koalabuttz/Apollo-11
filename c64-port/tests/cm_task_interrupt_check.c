/* SPDX-License-Identifier: MIT */
#include "cm_task_interrupt_cases.h"
#include "cm_task_interrupt_vectors.h"
#include "cm_task_interrupt_check.h"
static apollo_overlay_state state;static cm_native_io io;static cm_task_interrupt_state irq;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
unsigned cm_task_interrupt_check(void){unsigned n,run,i,j,fail=0;
 for(run=0;run<
#ifdef __mos__
 128u
#else
 1024u
#endif
 ;++run){uint32_t h;enum cm_task_interrupt_status status;
#ifdef __mos__
  n=(run/64u)*512u+((run%64u)*17u)%512u;
#else
  n=run;
#endif
  cm_task_interrupt_input(&state,&io,&irq,(uint16_t)n);status=n<512u?cm_task_interrupt_enter(&state,&io,&irq):cm_task_interrupt_resume(&state,&io,&irq);
  h=fold(2166136261u,(uint16_t)status);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);h=fold(h,irq.in_isr);h=fold(h,irq.pending_danzig);
  if(h!=cm_task_interrupt_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_task_interrupt_sample_indices[j]){
   if(cm_task_interrupt_samples[j][0]!=(uint16_t)status)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_task_interrupt_samples[j][i+1u])++fail;
   if(io.superbank!=cm_task_interrupt_samples[j][2049]||io.interrupt_enabled!=cm_task_interrupt_samples[j][2050]||irq.in_isr!=cm_task_interrupt_samples[j][2051]||irq.pending_danzig!=cm_task_interrupt_samples[j][2052])++fail;
  }
 }return fail;
}
