/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "p66_direct_pending_check.h"
#include "p66_direct_pending_vectors.h"
#include "p66_vertical_start.h"
static executive state;static exec_native runtime;static p66_basic_binding binding;static exec_irq_queue queue;static uint8_t excluded;static uint16_t errors;
static uint8_t enter(void *u){uint8_t old=excluded;(void)u;excluded=1;return old;}static void leave(void *u,uint8_t old){(void)u;excluded=old;}static const exec_exclusion guard={enter,leave,0};
static uint16_t invoke(void *u,apollo_overlay_state *s,p66_bias_io *io,uint16_t selector){exec_request r={037000,{03275,064007},0};enum wl_status status;(void)u;if(selector!=P66_DIRECT_VERTICAL_SERVICE||excluded!=1)++errors;status=p66_vertical_start(s,io);if(status==WL_OK&&exec_irq_post(&queue,&r)!=EXEC_OK)++errors;return (uint16_t)status;}
static enum exec_action stop(executive *s,void *u,exec_address *a){(void)s;(void)u;(void)a;return EXEC_DISPATCHED_BASIC;}
static const exec_context_mapping mapping[]={{{02536,062067},EXEC_CONTEXT_BASIC,p66_direct_launch_callback},{{03275,064067},EXEC_CONTEXT_BASIC,stop},{{03275,064007},EXEC_CONTEXT_BASIC,stop}};
uint16_t p66_direct_pending_check(apollo_overlay_state *s,apollo_overlay_state *want){uint16_t i,old_pending,old_priority;errors=0;p66_direct_launch_prepare(s,1137);*want=*s;
 for(i=0;i<sizeof p66_direct_pending_patch/sizeof p66_direct_pending_patch[0];++i)if(p66_direct_pending_patch[i][0]<2048u)want->words[p66_direct_pending_patch[i][0]]=p66_direct_pending_patch[i][1];
 old_pending=s->words[067];old_priority=s->words[0167];if(old_pending==0||s->words[0167+old_pending]<=old_priority)++errors;
 memset(&binding,0,sizeof binding);state.alarm=0;p66_native_import_executive(&state,s);binding.arithmetic.image=s;binding.arithmetic.runtime=&runtime;binding.arithmetic.producers=&guard;binding.invoke_basic=invoke;binding.control.superbank=060;binding.control.interrupt_enabled=1;excluded=0;
 if(exec_native_context_init(&runtime,&state,mapping,3,&binding)!=EXEC_OK)++errors;
 exec_irq_init(&queue);if(exec_native_bind_irq(&runtime,&queue,&guard)!=EXEC_OK)++errors;runtime.running=1;runtime.current=(exec_address){02536,062067};
 if(exec_native_step(&runtime)!=EXEC_OK||binding.status!=0||runtime.current.location!=03275||runtime.current.bank!=064067||runtime.current_kind!=EXEC_CONTEXT_BASIC||runtime.fault!=EXEC_OK||excluded!=0||queue.read!=0||queue.write!=1||state.newjob!=old_pending||state.core[0][EXEC_PRIORITY]!=old_priority||binding.control.superbank!=060||binding.control.interrupt_enabled!=0)++errors;
 p66_native_export_executive(s,&state);for(i=0;i<2048;++i)if(s->words[i]!=want->words[i])++errors;return errors;}
