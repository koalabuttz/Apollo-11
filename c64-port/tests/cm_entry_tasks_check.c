/* SPDX-License-Identifier: MIT */
#include "cm_entry_tasks_cases.h"
#include "cm_entry_tasks_vectors.h"
#include "cm_entry_tasks_check.h"
static apollo_overlay_state state;static cm_native_io io;static cm_waitlist queue;static cm_wl_native runtime;
static unsigned depth,callbacks,bad;
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
static enum cm_wl_status checked(cm_wl_native *r,void *u){
 volatile uint32_t frame=0x71324658u;enum cm_wl_status status;
 if(++depth!=1u||!r->in_callback)++bad;
 if(cm_wl_native_start(r)!=CM_WL_REENTER)++bad;
 status=cm_setjtag_callback(r,u);
 if(frame!=0x71324658u||depth!=1u)++bad;
 --depth;++callbacks;return status;
}
unsigned cm_entrytask_check(void){unsigned n,i,j,fail=0;
 static const cm_wl_mapping mapping={ {03207,032066},checked};cm_entry_task_binding binding={&state,&io};
 depth=callbacks=bad=0;
 for(n=0;n<1024u;++n){uint32_t h;enum cm_wl_status status;
  cm_entrytask_input(&state,&io,(uint16_t)n);
  /* Ephemeral dispatcher harness. Original-body image starts at actual entry;
   * separate source-proven WAITLIST views exercise the real C trampoline. */
  cm_wl_init(&queue);queue.time3=020000;status=cm_wl_native_init(&runtime,&queue,&mapping,1,&binding,0);
  if(status!=CM_WL_OK||cm_wl_native_schedule(&runtime,1,mapping.address)!=CM_WL_OK||cm_wl_native_advance(&runtime,2)!=CM_WL_OK||cm_wl_native_start(&runtime)!=CM_WL_OK){++fail;continue;}
  status=cm_wl_native_step(&runtime);
  if(status!=CM_WL_IDLE||runtime.running||runtime.in_callback||depth)++fail;
  h=fold(2166136261u,0);for(i=0;i<2048u;++i)h=fold(h,state.words[i]);h=fold(h,io.superbank);h=fold(h,io.interrupt_enabled);
  if(h!=cm_entrytask_hash[n])++fail;
  for(j=0;j<2u;++j)if(n==cm_entrytask_sample_indices[j]){
   if(cm_entrytask_samples[j][0]!=0)++fail;
   for(i=0;i<2048u;++i)if(state.words[i]!=cm_entrytask_samples[j][i+1u])++fail;
   if(io.superbank!=cm_entrytask_samples[j][2049]||io.interrupt_enabled!=cm_entrytask_samples[j][2050])++fail;
  }
 }
 return fail+bad+(callbacks==1024u?0u:1u);
}
