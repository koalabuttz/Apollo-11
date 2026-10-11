/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "p66_direct_launch_check.h"
#include "p66_direct_launch_vectors.h"
static executive state;
static exec_native runtime;
static p66_basic_binding binding;
static exec_irq_queue queue;
static uint8_t excluded,inject;
static uint16_t errors;
static uint8_t enter(void *u){uint8_t old=excluded;(void)u;excluded=1;return old;}
static void leave(void *u,uint8_t old){(void)u;excluded=old;}
static const exec_exclusion guard={enter,leave,0};
/* Deliberate test arrival at the completed compiled-service boundary. No
 * source CPU or Executive words are altered; producer tickets are external. */
static uint16_t invoke(void *u,apollo_overlay_state *s,p66_bias_io *io,uint16_t selector){
 uint16_t status=p66_direct_launch_entry(s,io,selector);exec_request request={037000,{03275,064007},0};(void)u;
 if(excluded!=1)++errors;
 if(inject && status==0 && exec_irq_post(&queue,&request)!=EXEC_OK)++errors;
 return status;
}
static enum exec_action boundary(executive *s,void *u,exec_address *a){(void)s;(void)u;(void)a;return EXEC_DISPATCHED_BASIC;}
static const exec_context_mapping mapping[]={
 {{02477,062007},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02477,062027},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02477,062047},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02477,062067},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062007},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062027},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062047},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{02536,062067},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},
 {{03275,064007},EXEC_CONTEXT_BASIC,boundary},
 {{03275,064027},EXEC_CONTEXT_BASIC,boundary},
 {{03275,064047},EXEC_CONTEXT_BASIC,boundary},
 {{03275,064067},EXEC_CONTEXT_BASIC,boundary}
};
uint16_t p66_direct_launch_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *expected){
 uint16_t f,p,end,number=p66_direct_launch_numbers[n],status;uint8_t trial;
 p66_bias_io old={(uint16_t)((number/5u)%4u*020u),(uint16_t)(number%2u)},want=old;
 p66_direct_launch_prepare(expected,number);
 p=p66_direct_launch_offsets[n];end=p66_direct_launch_offsets[n+1u];
 for(f=0;f<APOLLO_SHARED_WORDS+2u;++f){
  if(p<end && p66_direct_launch_patch[p][0]==f){uint16_t v=p66_direct_launch_patch[p++][1];if(f<APOLLO_SHARED_WORDS)expected->words[f]=v;else if(f==APOLLO_SHARED_WORDS)want.superbank=v;else want.interrupt_enabled=v;}
 }
 errors=0;if(p!=end)++errors;
 /* Compare linked source body, then callback under both empty and arriving IRQ.
  * Initial nonzero NEWJOB appears only after the last interpreter boundary. */
 for(trial=0;trial<3;++trial){
  p66_direct_launch_prepare(s,number);binding.control=old;
  if(trial==0)status=p66_direct_launch_entry(s,&binding.control,number<1024u ? P66_DIRECT_LAUNCH_SERVICE : P66_DIRECT_VERTICAL_SERVICE);
  else if(p66_direct_launch_results[n]==0){
   memset(&binding,0,sizeof binding);binding.control=old;
   state.alarm=0;p66_native_import_executive(&state,s);
   binding.arithmetic.image=s;binding.arithmetic.runtime=&runtime;binding.arithmetic.producers=&guard;binding.invoke_basic=invoke;
   if(exec_native_context_init(&runtime,&state,mapping,12,&binding)!=EXEC_OK)++errors;
   exec_irq_init(&queue);if(exec_native_bind_irq(&runtime,&queue,&guard)!=EXEC_OK)++errors;
   runtime.running=1;runtime.current.location=s->words[5];runtime.current.bank=(uint16_t)(s->words[6]|old.superbank);excluded=0;inject=(uint8_t)(trial==2);
   if(exec_native_step(&runtime)!=EXEC_OK)++errors;
   status=binding.status;
   if(binding.control.superbank!=want.superbank || binding.control.interrupt_enabled!=want.interrupt_enabled)++errors;
   if(runtime.current.location!=03275 || runtime.current.bank!=(uint16_t)(064007|want.superbank) || runtime.current_kind!=EXEC_CONTEXT_BASIC || excluded!=0 || queue.read!=0 || queue.write!=inject || runtime.fault!=EXEC_OK)++errors;
   p66_native_export_executive(s,&state);
   for(f=0;f<APOLLO_SHARED_WORDS;++f)if(s->words[f]!=expected->words[f])++errors;
   continue;
  }else continue;
  if(status!=p66_direct_launch_results[n] || binding.control.superbank!=want.superbank || binding.control.interrupt_enabled!=want.interrupt_enabled)++errors;
  for(f=0;f<APOLLO_SHARED_WORDS;++f)if(s->words[f]!=expected->words[f])++errors;
 }
 return errors;
}
