/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lm_interpreter_exchange_native.h"
#include "lm_interpreter_change_cases.h"
#include "p66_native.h"
static executive state;static exec_native runtime;static exec_irq_queue queue;static apollo_overlay_state image,expected;
static lm_interpreter_exchange_binding binding;static uint8_t excluded,inject,variant,original_exclusion;
static uint8_t enter(void *u){uint8_t old=excluded;(void)u;excluded=1;return old;}static void leave(void *u,uint8_t old){(void)u;excluded=old;}static const exec_exclusion guard={enter,leave,0};
static enum exec_action callback(executive *s,void *u,exec_address *resume){enum exec_action action=lm_interpreter_exchange_callback(s,u,resume);exec_request request={037000,{02001,062007},0};assert(excluded==original_exclusion);if(inject)assert(exec_irq_post(&queue,&request)==EXEC_OK);
 switch(variant){case 1:runtime.current_kind=EXEC_CONTEXT_BASIC;break;case 2:resume->location=0;resume->bank=0;state.newjob=AGC_NEG_ZERO;state.core[0][EXEC_PRIORITY]=AGC_NEG_ZERO;break;case 3:resume->location=0;break;case 4:resume->location=010000;break;case 5:resume->bank|=010;break;case 6:state.core[0][EXEC_PRIORITY]=0;break;case 7:resume->location=07777;break;case 8:runtime.interpreter_control.edop=1;break;case 9:runtime.interpreter_control.intbit15=0;break;case 10:runtime.requested_priority=1;break;case 11:runtime.fault=EXEC_BAILOUT_PENDING;break;default:break;}return action;}
static exec_continuation resolve(void *u,exec_address a,enum exec_context_kind k){if(k==EXEC_CONTEXT_INTERPRETIVE&&a.location==05126&&(a.bank&076000u)==064000u)return callback;return lm_interpreter_exchange_resolve(u,a,k);}
static void prepare(uint16_t n){static p66_bias_io io;lm_interpreter_change_input(&image,&io,n);memset(&binding,0,sizeof binding);binding.runtime=&runtime;binding.image=&image;binding.io=&io;p66_native_import_executive(&state,&image);state.alarm=0;assert(exec_native_resolver_init(&runtime,&state,resolve,&binding,&binding)==EXEC_OK);exec_irq_init(&queue);assert(exec_native_bind_irq(&runtime,&queue,&guard)==EXEC_OK);runtime.running=1;runtime.current_kind=EXEC_CONTEXT_INTERPRETIVE;runtime.current.location=05126;runtime.current.bank=(uint16_t)(image.words[6]|io.superbank);runtime.interpreter_control.edop=0;runtime.interpreter_control.intbit15=040000;}
int main(void){unsigned i,j;static const enum exec_status failures[12]={EXEC_OK,EXEC_INVALID_ACTION,EXEC_OUT_OF_SCOPE,EXEC_OUT_OF_SCOPE,EXEC_OUT_OF_SCOPE,EXEC_OUT_OF_SCOPE,EXEC_OUT_OF_SCOPE,EXEC_UNMAPPED,EXEC_OUT_OF_SCOPE,EXEC_OUT_OF_SCOPE,EXEC_INVALID_ACTION,EXEC_BAILOUT_PENDING};
 for(i=0;i<8u;++i){p66_bias_io want;exec_transfer next;prepare((uint16_t)(i&1u));variant=0;inject=(uint8_t)((i/4u)&1u);original_exclusion=(uint8_t)((i/2u)&1u);excluded=original_exclusion;expected=image;want=*binding.io;assert(lm_interpreter_change(&expected,&want,&next)==EXEC_OK);assert(exec_native_step(&runtime)==EXEC_OK);p66_native_export_executive(&image,&state);assert(memcmp(&image,&expected,sizeof image)==0);assert(binding.callbacks==1&&queue.read==0&&queue.write==inject&&excluded==original_exclusion);assert(runtime.current_kind==((i&1u)?EXEC_CONTEXT_INTERPRETIVE:EXEC_CONTEXT_BASIC));}
 inject=0;excluded=0;original_exclusion=0;
 for(i=1;i<12u;++i){prepare(1);variant=(uint8_t)i;assert(exec_native_step(&runtime)==failures[i]);assert(runtime.fault==failures[i]&&binding.callbacks==1&&queue.read==0&&queue.write==0&&excluded==0);}
 /* Old BASIC-only adoption must still reject an interpretive-origin callback. */
 for(j=0;j<2u;++j){prepare((uint16_t)j);runtime.in_callback=1;assert(exec_native_finish(&runtime,j?EXEC_DISPATCHED_INTERPRETER:EXEC_DISPATCHED_BASIC,(exec_address){03362,064007})==EXEC_INVALID_ACTION);}
 puts("PASS:8 exact exchanges/pendingIRQ+11 invalid/fault seams+2 retained old-origin guards");return 0;}
