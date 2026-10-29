/* SPDX-License-Identifier: MIT */
#include "p66_application_pending.h"
#include <stddef.h>
typedef char pending_application_first[(offsetof(p66_application_pending,recurrence)==0 &&
 offsetof(p66_application_recurrence,application)==0)?1:-1];
#ifndef P66_PENDING_DISPATCH_ONLY
uint16_t p66_application_pending_entry(p66_application_pending *b,exec_address *resume,uint16_t selector){
 p66_descent_binding *d; p66_native_binding *a;enum exec_status status;exec_transfer selected;
 if(!b)return 0xffff;
 d=&b->recurrence.application.display_job.descent;a=&d->basic.arithmetic;
 if(!a->image||!a->runtime||!a->runtime->state)return 0xffff;
#ifdef P66_PENDING_ONLY_SERVICE
 if(selector!=P66_PENDING_ONLY_SERVICE)return 0xffff;
 selector=P66_PENDING_ONLY_SERVICE;
#endif
 if(selector==P66_PENDING_STEP_IMPORT){b->previous_kind=a->runtime->current_kind;
  p66_native_import_executive(a->runtime->state,a->image);return 0;}
 if(selector==P66_PENDING_STEP_EXPORT){exec_native*r=a->runtime;
  p66_native_export_executive(a->image,r->state);
  if(b->step_status==EXEC_OK){uint16_t bank=(uint16_t)(r->current.bank&076007u);
   a->image->words[6]=bank;a->image->words[3]=(uint16_t)((bank&7u)<<8);
   a->image->words[4]=(uint16_t)(bank&076000u);
   d->basic.control.superbank=(uint16_t)(r->current.bank&0160u);}
  if(b->previous_kind==EXEC_CONTEXT_INTERPRETIVE||
     (b->step_status==EXEC_OK&&r->current_kind==EXEC_CONTEXT_INTERPRETIVE)){
   a->image->words[0115]=r->interpreter_control.intbit15;
   a->image->words[023]=r->interpreter_control.edop;}
  return 0;}
 if(selector==P66_PENDING_ELAPSED_RETURN)return p66_elapsed_return_control(
  a->image,b->elapsed_q,b->elapsed_before,b->elapsed_piptime);
 if(selector!=P66_PENDING_EXCHANGE||!resume)return 0xffff;
 status=lm_danzig_change(a->image,&d->basic.control,&selected);
 if(status!=EXEC_OK){a->runtime->fault=status;return EXEC_YIELD;}
 p66_native_import_executive(a->runtime->state,a->image);
 a->runtime->interpreter_control.intbit15=a->image->words[0115];
 a->runtime->interpreter_control.edop=a->image->words[023];
 *resume=selected.address;
 return selected.interpretive?EXEC_EXCHANGED_INTERPRETER:EXEC_EXCHANGED_BASIC;
}
#endif
#ifndef P66_PENDING_KERNEL_ONLY
static p66_application_pending *resident_binding(void *user){
#ifdef P66_PENDING_FIXED_BINDING
 (void)user;
 return (p66_application_pending *)(uintptr_t)(0xc400u+sizeof(exec_native));
#else
 return user;
#endif
}
static __attribute__((noinline)) uint16_t service(p66_application_pending *b,exec_address *resume,uint16_t selector){
 if(b->invoke_pending)return b->invoke_pending(b->pending_modules,b,resume,selector);
#ifndef P66_PENDING_DISPATCH_ONLY
 return p66_application_pending_entry(b,resume,selector);
#else
 return 0xffff;
#endif
}
static uint16_t math(void *user,apollo_overlay_state *image,uint16_t selector){
 p66_application_pending *b=resident_binding(user);uint16_t status;
 if(selector==103u){b->elapsed_q=image->words[2];
  b->elapsed_before.high=image->words[0154];b->elapsed_before.low=image->words[0155];
  b->elapsed_piptime.high=image->words[01234];b->elapsed_piptime.low=image->words[01235];}
 status=b->original_math.call(b->original_math.user,image,selector);
 if(status==0&&selector==103u)status=service(b,0,P66_PENDING_ELAPSED_RETURN);
 return status;
}
enum exec_status p66_application_pending_step(p66_application_pending *b){
 p66_native_binding *a;uint8_t token;uint16_t status;
 if(!b)return EXEC_OUT_OF_SCOPE;
 b=resident_binding(b);
 a=&b->recurrence.application.display_job.descent.basic.arithmetic;
 if(!a->image||!a->runtime||!a->producers||!a->producers->enter||!a->producers->leave)return EXEC_OUT_OF_SCOPE;
 token=a->producers->enter(a->producers->user);
 status=service(b,0,P66_PENDING_STEP_IMPORT);
 a->producers->leave(a->producers->user,token);
 if(status)return EXEC_OUT_OF_SCOPE;
 b->step_status=exec_native_step(a->runtime);
 token=a->producers->enter(a->producers->user);
 status=service(b,0,P66_PENDING_STEP_EXPORT);
 a->producers->leave(a->producers->user,token);
 return status?EXEC_OUT_OF_SCOPE:b->step_status;
}
static enum exec_action continuation(executive *state,void *user,exec_address *resume){
 p66_application_pending *b=resident_binding(user);p66_application_binding *app=&b->recurrence.application;
 p66_native_binding *a=&app->display_job.descent.basic.arithmetic;exec_native *r=a->runtime;
 /* Prepare's final resolver lookup publishes this resident callback. Native
  * address validation can look up other entries during finish; prepare
  * resolves the actual selected entry again before invoking this wrapper. */
 exec_continuation original=b->translated;
 enum exec_action action;uint8_t token;
 if(!original){r->fault=EXEC_UNMAPPED;return EXEC_YIELD;}
 action=original(state,app,resume);
 if(r->fault!=EXEC_OK)return action;
 if(action==EXEC_YIELD){
  token=a->producers->enter(a->producers->user);
  if(a->image->words[067]==0||a->image->words[067]>=AGC_SIGN){
   a->producers->leave(a->producers->user,token);return action;}
  action=(enum exec_action)service(b,resume,P66_PENDING_EXCHANGE);
#ifndef P66_PENDING_OBSERVE_INVOKER
  if(r->fault==EXEC_OK&&a->observe)a->observe(a->observer,a->image,a->image->words[5]);
#endif
  a->producers->leave(a->producers->user,token);
  return action;
 }
 return action;
}
exec_continuation p66_application_pending_resolve(void *user,exec_address address,enum exec_context_kind kind){
 p66_application_pending *b=resident_binding(user);
 exec_continuation original=p66_application_split_resolve(&b->recurrence.application,address,kind);
 if(original&&kind==EXEC_CONTEXT_INTERPRETIVE)b->translated=original;
 return original&&kind==EXEC_CONTEXT_INTERPRETIVE?continuation:original;
}
enum exec_status p66_application_pending_init(exec_native *r,executive *state,p66_application_pending *b){
 p66_descent_binding *d;enum exec_status status;
 if(!b)return EXEC_OUT_OF_SCOPE;
 d=&b->recurrence.application.display_job.descent;
 if(!d->math_modules||!d->math_modules->call)return EXEC_OUT_OF_SCOPE;
 b->original_math.call=d->math_modules->call;b->original_math.user=d->math_modules->user;
 b->wrapped_math.call=math;b->wrapped_math.user=b;
 status=p66_application_split_init(r,state,&b->recurrence.application);
 if(status!=EXEC_OK)return status;
 d->math_modules=&b->wrapped_math;
 /* The public initializer has completed all guards and initialized the same
  * first-member binding. Replace its resident resolver without a second
  * Executive initialization or source state change. */
 r->resolve=p66_application_pending_resolve;r->registry_user=b;r->user=b;
 return EXEC_OK;
}
#endif
