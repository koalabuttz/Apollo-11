/* SPDX-License-Identifier: MIT */
#include "p66_application_split.h"
#include "p66_direct_launch.h"
#ifndef P66_APPLICATION_SPLIT_DISPATCH_ONLY
uint16_t p66_application_split_service(p66_application_binding *b,uint16_t op){
 p66_basic_binding *basic=&b->display_job.descent.basic;
 p66_native_binding *a=&basic->arithmetic;exec_native *r=a->runtime;
 uint16_t *w=a->image->words,bank;
#ifdef P66_APPLICATION_SPLIT_ONLY_SERVICE
 if(op!=P66_APPLICATION_SPLIT_ONLY_SERVICE)return 0xffff;
 op=P66_APPLICATION_SPLIT_ONLY_SERVICE;
#endif
 if(op==P66_APPLICATION_SPLIT_FINISH){p66_native_import_executive(r->state,a->image);return 0;}
 if(op!=P66_APPLICATION_SPLIT_PREPARE)return 0xffff;
 p66_native_export_executive(a->image,r->state);
 bank=(uint16_t)(r->current.bank&076007u);
 w[3]=(uint16_t)((bank&7u)*0400u);w[4]=(uint16_t)(bank&076000u);
 w[6]=bank;w[5]=r->current.location;
 basic->control.superbank=(uint16_t)(r->current.bank&0160u);
 if(w[5]==02477u)return P66_DIRECT_LAUNCH_SERVICE;
 if(w[5]==02536u)return P66_DIRECT_VERTICAL_SERVICE;
 if(w[5]==03534u)return P66_APPLICATION_MODE;
 return(w[0154]==021u||w[0154]==037u)?P66_APPLICATION_NV:P66_APPLICATION_NUMBER;
}
#endif
#ifndef P66_APPLICATION_SPLIT_KERNEL_ONLY
static uint16_t bridge(p66_application_binding*b,uint16_t op){
 p66_basic_binding *basic=&b->display_job.descent.basic;
 if(b->invoke_application)return b->invoke_application(b->application_modules,
  basic->arithmetic.image,&basic->control,&b->display_job.channel11,op);
#ifndef P66_APPLICATION_SPLIT_DISPATCH_ONLY
 return p66_application_split_service(b,op);
#else
 return 0xffff;
#endif
}
static enum exec_action body(executive *state,void *user,exec_address *resume){
 p66_application_binding*b=user;p66_basic_binding *basic=&b->display_job.descent.basic;
 p66_native_binding*a=&basic->arithmetic;uint16_t op,status;uint8_t launch,token;
 (void)state;token=a->producers->enter(a->producers->user);
 launch=(uint8_t)(a->runtime->current.location==02477u||a->runtime->current.location==02536u);
 op=bridge(b,P66_APPLICATION_SPLIT_PREPARE);
 if(launch){status=p66_application_basic(b,a->image,&basic->control,op);basic->status=status;}
 else{
  if(b->invoke_application)status=b->invoke_application(b->application_modules,
   a->image,&basic->control,&b->display_job.channel11,op);
#ifndef P66_APPLICATION_SPLIT_DISPATCH_ONLY
  else status=p66_application_entry(a->image,&basic->control,&b->display_job.channel11,op);
#else
  else status=0xffff;
#endif
  b->status=status;
 }
 if(bridge(b,P66_APPLICATION_SPLIT_FINISH)!=0)status=0xffff;
 resume->location=a->image->words[5];
 resume->bank=(uint16_t)(a->image->words[6]|basic->control.superbank);
 if(status!=0){a->runtime->fault=EXEC_OUT_OF_SCOPE;
  a->producers->leave(a->producers->user,token);return EXEC_DISPATCHED_BASIC;}
 if(!launch)dsky_display_output(&b->peripheral,011,b->display_job.channel11);
 if(a->observe)a->observe(a->observer,a->image,resume->location);
 a->producers->leave(a->producers->user,token);
 if(launch)return EXEC_DISPATCHED_BASIC;
 {enum exec_action action=p66_display_job_finish(a->runtime->state,&b->display_job,resume);
  dsky_display_output(&b->peripheral,011,b->display_job.channel11);return action;}
}
static exec_continuation display(void*user,exec_address address,enum exec_context_kind kind){
 uint8_t i;(void)user;for(i=0;i<P66_DISPLAY_JOB_MAPPING_COUNT;++i){
  const exec_context_mapping*m=&p66_display_job_mappings[i];
  if(m->address.location==address.location&&m->address.bank==address.bank&&m->kind==kind)return m->call;
 }return 0;
}
exec_continuation p66_application_split_resolve(void*user,exec_address address,enum exec_context_kind kind){
 p66_application_binding*b=user;
 if(kind==EXEC_CONTEXT_BASIC){
  if((address.location==02477u||address.location==02536u)&&
     (address.bank&076007u)==062007u&&(address.bank&~076067u)==0)return body;
  if((address.location==03534u&&address.bank==060102u)||
     (address.location==02077u&&address.bank==060101u))return body;
 }
 return p66_descent_resolve(&b->display_job.descent,address,kind);
}
enum exec_status p66_application_split_init(exec_native*r,executive*s,p66_application_binding*b){
 enum exec_status status;if(!b)return EXEC_OUT_OF_SCOPE;
 b->display_job.descent.resolve_extra=display;b->display_job.descent.extra_registry=b;
 b->display_job.descent.finish_source=p66_display_job_finish;
 b->display_job.descent.basic.invoke_basic=p66_application_basic;
 b->display_job.descent.basic.basic_modules=b;b->display_job.descent.basic.display=&b->peripheral;
 status=p66_descent_init_resolved(r,s,&b->display_job.descent);if(status!=EXEC_OK)return status;
 return exec_native_resolver_init(r,s,p66_application_split_resolve,b,b);
}
#endif
