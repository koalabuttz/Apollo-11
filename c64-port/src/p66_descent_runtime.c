/* SPDX-License-Identifier: MIT */
#include "p66_descent_runtime.h"
#include <stddef.h>
#include "p66_descent_prefix_control.h"
#include "guidance_control_observer.h"
typedef char arithmetic_binding_first[(offsetof(p66_descent_binding,basic)==0&&offsetof(p66_basic_binding,arithmetic)==0)?1:-1];
static const agc_word feedback_entries[20]={03434,03437,03441,03444,03445,03447,03452,03455,03460,03462,03465,03470,03473,03475,03477,03500,03503,03504,03505,03507};
static const agc_word horizontal_entries[8]={03510,03517,03521,03523,03525,03526,03527,03530};
static const uint8_t horizontal_selectors[8]={220,221,222,223,224,225,226,227};
static void observe_source(void *user,const apollo_overlay_state *image,agc_word loc){p66_descent_binding *b=user;
 if(image!=b->basic.arithmetic.image){b->basic.arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;return;}
 (void)p66_descent_prefix_control(b->basic.arithmetic.image,loc,b->prefix_ddv_q);
 if(b->observe_source)b->observe_source(b->source_observer,image,loc);
}
static void bind_observer(p66_descent_binding *b){p66_native_binding *a=&b->basic.arithmetic;
 if(a->observe!=observe_source){b->observe_source=a->observe;b->source_observer=a->observer;a->observe=observe_source;a->observer=b;}
}
static uint16_t control(p66_descent_binding *b,uint16_t selector){
 if(b->invoke_control==0)return 0xffff;
 return b->invoke_control(b->control_modules,b->basic.arithmetic.image,&b->basic.control,&b->channel14,selector);
}
static uint16_t arithmetic(void *user,apollo_overlay_state *image,uint16_t selector){
 p66_descent_binding *b=user;
 if(image!=b->basic.arithmetic.image)return P66_NATIVE_UNSUPPORTED;
 /* RODCOMP explicitly INHINTs; its observation is before TC INTPRET.
  * The first compiled interpretive pair follows INTPRET's unconditional
  * RELINT. Source hardware metadata remains separate from native IRQ guards. */
 if(selector==1u)b->basic.control.interrupt_enabled=0;
 if(selector==2u)b->basic.control.interrupt_enabled=1;
 if(selector==4u){agc_dp numerator={image->words[0154],image->words[0155]},divisor={0,400};b->prefix_ddv_q=p66_ddv_return_q(image->words[2],numerator,divisor);}
 if(selector==6u)image->words[020]=5;
 if(selector==220u){uint16_t status;
  /* Last DANZIG poll already completed. This is a compiled fixed EXIT fetch,
   * not a second job check; pending requests survive all basic throttle code. */
  image->words[5]=06070;
  status=control(b,P66_DESCENT_THROTTLE_FETCH);if(status)return status;
  status=control(b,P66_DESCENT_THROTTLE_SERVICE);if(status)return status;
  status=control(b,P66_DESCENT_THROTTLE_RETURN);if(status)return status;
 }
 return p66_descent_invoke(b->math_modules,image,selector);
}
static enum exec_action continuation(executive *state,void *user,exec_address *resume){
 p66_descent_binding *b=user;p66_native_binding *a=&b->basic.arithmetic;
 agc_word loc=a->runtime->current.location;uint8_t i;
 if(loc==03531u){uint16_t result;uint8_t token=a->producers->enter(a->producers->user);
  p66_native_export_executive(a->image,state);result=control(b,P66_DESCENT_DISPLAY_EXIT);
  if(result==0)result=control(b,P66_DESCENT_REGODSPR);
  else if(result==1)result=0;
  p66_native_import_executive(state,a->image);a->status=result;
  if(result)a->runtime->fault=EXEC_OUT_OF_SCOPE;
  if(!result&&a->observe)a->observe(a->observer,a->image,05155);
  a->producers->leave(a->producers->user,token);
  if(!result&&b->finish_source)return b->finish_source(state,user,resume);
  return EXEC_FINISH;
 }
 for(i=0;i<20u;++i)if(loc==feedback_entries[i])
  return p66_native_run_group(state,a,resume,(uint16_t)(190u+i),0,0);
 for(i=0;i<8u;++i)if(loc==horizontal_entries[i]){
  enum exec_action action=p66_native_run_group(state,a,resume,horizontal_selectors[i],0,0);
  if(i==7u)return EXEC_EXIT_INTERPRETER;
  return action;
 }
 a->status=P66_NATIVE_BOUNDARY;a->runtime->fault=EXEC_OUT_OF_SCOPE;return EXEC_YIELD;
}
exec_continuation p66_descent_resolve(void *user,exec_address address,enum exec_context_kind kind){
 p66_descent_binding *b=user;uint8_t i;
 if(address.location==0 || address.location>07777u ||
    (address.bank&(agc_word)~UINT16_C(076167))!=0)return 0;
 for(i=0;i<P66_GUIDANCE_MAPPING_COUNT;++i){const exec_context_mapping *m=&p66_native_guidance_mappings[i];
  if(m->address.location==address.location&&m->address.bank==address.bank&&m->kind==kind)
   return address.location==03434u?continuation:m->call;
 }
 if(address.bank==064007u||address.bank==064067u){
  if(kind==EXEC_CONTEXT_INTERPRETIVE){
   for(i=1;i<20u;++i)if(address.location==feedback_entries[i])return continuation;
   for(i=0;i<8u;++i)if(address.location==horizontal_entries[i])return continuation;
  }else if(kind==EXEC_CONTEXT_BASIC&&address.location==03531u)return continuation;
 }
 for(i=0;i<P66_BASIC_MAPPING_COUNT;++i){const exec_context_mapping *m=&p66_basic_mappings[i];
  if(m->address.location==address.location&&m->address.bank==address.bank&&m->kind==kind)return m->call;
 }
 return b->resolve_extra?b->resolve_extra(b->extra_registry,address,kind):0;
}
enum exec_status p66_descent_init_resolved(exec_native *runtime,executive *state,p66_descent_binding *b){
 if(!b||!b->basic.arithmetic.image||!b->basic.arithmetic.producers||!b->math_modules||!b->invoke_control)return EXEC_OUT_OF_SCOPE;
 bind_observer(b);b->basic.arithmetic.runtime=runtime;b->basic.arithmetic.invoke=arithmetic;b->basic.arithmetic.modules=b;
 return exec_native_resolver_init(runtime,state,p66_descent_resolve,b,b);
}
__attribute__((noinline)) static void copy_mappings(exec_context_mapping *out,const exec_context_mapping *in,uint8_t count){
 volatile unsigned char *d=(volatile unsigned char *)out;const unsigned char *s=(const unsigned char *)in;
 unsigned bytes=(unsigned)count*(unsigned)sizeof(*in),i;for(i=0;i<bytes;++i)d[i]=s[i];
}
enum exec_status p66_descent_init(exec_native *runtime,executive *state,p66_descent_binding *b,
 exec_context_mapping *table,uint8_t capacity,const exec_context_mapping *extra,uint8_t extra_count){
 uint8_t bank,i,n=P66_GUIDANCE_MAPPING_COUNT;
 if(!b||!table||!b->basic.arithmetic.image||!b->basic.arithmetic.producers||!b->math_modules||
    !b->invoke_control||capacity<P66_DESCENT_MAPPING_COUNT||extra_count>capacity-P66_DESCENT_MAPPING_COUNT||
    (extra_count&&extra==0))return EXEC_OUT_OF_SCOPE;
 copy_mappings(table,p66_native_guidance_mappings,P66_GUIDANCE_MAPPING_COUNT);
 for(i=0;i<P66_GUIDANCE_MAPPING_COUNT;++i)if(table[i].address.location==03434)table[i].call=continuation;
 for(bank=0;bank<2u;++bank){agc_word bb=bank?064067:064007;
  for(i=1;i<20u;++i)table[n++]=(exec_context_mapping){{feedback_entries[i],bb},EXEC_CONTEXT_INTERPRETIVE,continuation};
  for(i=0;i<8u;++i)table[n++]=(exec_context_mapping){{horizontal_entries[i],bb},EXEC_CONTEXT_INTERPRETIVE,continuation};
  table[n++]=(exec_context_mapping){{03531,bb},EXEC_CONTEXT_BASIC,continuation};
 }
 copy_mappings(table+n,p66_basic_mappings,P66_BASIC_MAPPING_COUNT);n=(uint8_t)(n+P66_BASIC_MAPPING_COUNT);
 copy_mappings(table+n,extra,extra_count);n=(uint8_t)(n+extra_count);
 bind_observer(b);b->basic.arithmetic.runtime=runtime;b->basic.arithmetic.invoke=arithmetic;b->basic.arithmetic.modules=b;
 return exec_native_context_init(runtime,state,table,n,b);
}
