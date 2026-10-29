/* SPDX-License-Identifier: MIT */
#include "p66_application.h"
#include "p66_direct_launch.h"
#ifndef P66_APPLICATION_DISPATCH_ONLY
#include "p66_launch_phased.h"
#include "p66_vertical_start.h"
#include "p66_mode_display.h"
#include "dsky_charin_number.h"
uint16_t p66_application_entry(apollo_overlay_state *image,p66_bias_io *io,agc_word *channel,uint16_t service){
 if(!image||!io)return 0xffff;
 switch(service){case P66_APPLICATION_SETUP:return (uint16_t)p66_launch_setup_phase(image,io);case P66_APPLICATION_BIAS:return (uint16_t)p66_launch_bias_phase(image,io);case P66_APPLICATION_VERTICAL:return (uint16_t)p66_vertical_start(image,io);case P66_APPLICATION_MODE:return (uint16_t)p66_mode_display(image,io);
 case P66_APPLICATION_NV:case P66_APPLICATION_NUMBER:{dsky_charin_io key;enum p66_bias_status status;if(!channel)return 0xffff;key.control=*io;key.channel11=*channel;key.wrote_channel11=0;status=service==P66_APPLICATION_NV?dsky_charin_nv(image,&key):dsky_charin_number(image,&key);*io=key.control;*channel=key.channel11;return (uint16_t)status;}
 default:return 0xffff;}
}
#endif
static uint16_t invoke(p66_application_binding *b,apollo_overlay_state *image,p66_bias_io *io,uint16_t selector){
 if(b->invoke_application)return b->invoke_application(b->application_modules,image,io,&b->display_job.channel11,selector);
#ifndef P66_APPLICATION_DISPATCH_ONLY
 return p66_application_entry(image,io,&b->display_job.channel11,selector);
#else
 return 0xffff;
#endif
}
uint16_t p66_application_basic(void *user,apollo_overlay_state *image,p66_bias_io *io,uint16_t selector){p66_application_binding *b=user;uint16_t status;
 if(selector==P66_DIRECT_VERTICAL_SERVICE)return invoke(b,image,io,P66_APPLICATION_VERTICAL);
 if(selector==P66_BASIC_MODE_DISPLAY)return invoke(b,image,io,P66_APPLICATION_MODE);
 if(selector!=P66_DIRECT_LAUNCH_SERVICE)return 0xffff;
 status=invoke(b,image,io,P66_APPLICATION_SETUP);if(status)return status;
 status=invoke(b,image,io,P66_APPLICATION_BIAS);if(status)return status;
 return invoke(b,image,io,P66_APPLICATION_VERTICAL);
}
static enum exec_action body(executive *state,void *user,exec_address *resume){p66_application_binding *b=user;p66_basic_binding *basic=&b->display_job.descent.basic;p66_native_binding *a=&basic->arithmetic;uint16_t *w=a->image->words,bank,selector;uint8_t token=a->producers->enter(a->producers->user);
 p66_native_export_executive(a->image,state);bank=(uint16_t)(a->runtime->current.bank&076007u);w[3]=(uint16_t)((bank&7u)*0400u);w[4]=(uint16_t)(bank&076000u);w[6]=bank;w[5]=a->runtime->current.location;basic->control.superbank=(uint16_t)(a->runtime->current.bank&0160u);
 selector=w[5]==03534?P66_APPLICATION_MODE:(w[0154]==021||w[0154]==037)?P66_APPLICATION_NV:P66_APPLICATION_NUMBER;
 b->status=invoke(b,a->image,&basic->control,selector);p66_native_import_executive(state,a->image);
 if(b->status!=0){a->runtime->fault=EXEC_OUT_OF_SCOPE;a->producers->leave(a->producers->user,token);return EXEC_DISPATCHED_BASIC;}
 dsky_display_output(&b->peripheral,011,b->display_job.channel11);
 if(a->observe)a->observe(a->observer,a->image,w[5]);
 a->producers->leave(a->producers->user,token);
 {enum exec_action action=p66_display_job_finish(state,&b->display_job,resume);dsky_display_output(&b->peripheral,011,b->display_job.channel11);return action;}
}
static exec_continuation display_resolve(void *user,exec_address address,enum exec_context_kind kind){uint8_t i;(void)user;for(i=0;i<P66_DISPLAY_JOB_MAPPING_COUNT;++i){const exec_context_mapping *m=&p66_display_job_mappings[i];if(m->address.location==address.location&&m->address.bank==address.bank&&m->kind==kind)return m->call;}return 0;}
exec_continuation p66_application_resolve(void *user,exec_address address,enum exec_context_kind kind){uint8_t i;p66_application_binding *b=user;if(kind==EXEC_CONTEXT_BASIC){for(i=0;i<P66_DIRECT_LAUNCH_MAPPING_COUNT;++i){const exec_context_mapping *m=&p66_direct_launch_mappings[i];if(m->address.location==address.location&&m->address.bank==address.bank)return m->call;}if((address.location==03534&&address.bank==060102)||(address.location==02077&&address.bank==060101))return body;}return p66_descent_resolve(&b->display_job.descent,address,kind);}
enum exec_status p66_application_init(exec_native *runtime,executive *state,p66_application_binding *b){enum exec_status status;
 if(!b)return EXEC_OUT_OF_SCOPE;
 b->display_job.descent.resolve_extra=display_resolve;b->display_job.descent.extra_registry=b;b->display_job.descent.finish_source=p66_display_job_finish;
 b->display_job.descent.basic.invoke_basic=p66_application_basic;b->display_job.descent.basic.basic_modules=b;b->display_job.descent.basic.display=&b->peripheral;
 status=p66_descent_init_resolved(runtime,state,&b->display_job.descent);if(status!=EXEC_OK)return status;
 return exec_native_resolver_init(runtime,state,p66_application_resolve,b,b);
}
