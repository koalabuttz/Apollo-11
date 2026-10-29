/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "p66_application_split.h"
#include "p66_direct_launch_cases.h"
#include "p66_direct_pending_vectors.h"
static p66_application_binding binding;static exec_irq_queue queue;
static uint8_t excluded,stage,frame;
static uint8_t enter(void*u){uint8_t old=excluded;(void)u;excluded=1;return old;}
static void leave(void*u,uint8_t old){(void)u;excluded=old;}
static uint16_t math(void*u,apollo_overlay_state*m,uint16_t op){(void)u;(void)m;(void)op;return 0xffff;}
static uint16_t control(void*u,apollo_overlay_state*m,p66_bias_io*io,agc_word*c,uint16_t op){(void)u;(void)m;(void)io;(void)c;(void)op;return 0xffff;}
static uint16_t application(void*u,apollo_overlay_state*m,p66_bias_io*io,agc_word*c,uint16_t op){
 uint16_t result;(void)u;assert(excluded&&!frame);frame=1;
 if(op==P66_APPLICATION_SPLIT_PREPARE){assert(stage==0);result=p66_application_split_service(&binding,op);stage=1;}
 else if(op==P66_APPLICATION_SPLIT_FINISH){assert(stage==2);result=p66_application_split_service(&binding,op);stage=3;}
 else{exec_request request={037000,{03275,064007},0};assert(stage==1&&op==P66_APPLICATION_VERTICAL);
  result=p66_application_entry(m,io,c,op);assert(!result);stage=2;
  assert(exec_irq_post(&queue,&request)==EXEC_OK);}
 frame=0;return result;
}
int main(void){static apollo_overlay_state image,expected;executive state;exec_native runtime;
 exec_exclusion guard={enter,leave,0};p66_guidance_modules modules={math,0};unsigned i;
 uint16_t old_pending,old_priority;p66_direct_launch_prepare(&image,1137);expected=image;
 for(i=0;i<sizeof p66_direct_pending_patch/sizeof p66_direct_pending_patch[0];++i)
  if(p66_direct_pending_patch[i][0]<2048u)expected.words[p66_direct_pending_patch[i][0]]=p66_direct_pending_patch[i][1];
 old_pending=image.words[067];old_priority=image.words[0167];
 assert(old_pending&&image.words[0167+old_pending]>old_priority);
 p66_native_import_executive(&state,&image);state.alarm=0;
 binding.display_job.descent.basic.arithmetic.image=&image;
 binding.display_job.descent.basic.arithmetic.producers=&guard;
 binding.display_job.descent.math_modules=&modules;binding.display_job.descent.invoke_control=control;
 binding.display_job.descent.basic.control=(p66_bias_io){060,1};binding.invoke_application=application;
 assert(p66_application_split_init(&runtime,&state,&binding)==EXEC_OK);
 exec_irq_init(&queue);assert(exec_native_bind_irq(&runtime,&queue,&guard)==EXEC_OK);
 runtime.running=1;runtime.current=(exec_address){02536,062067};
 assert(exec_native_step(&runtime)==EXEC_OK&&runtime.current.location==03275u&&runtime.current.bank==064067u);
 assert(runtime.current_kind==EXEC_CONTEXT_BASIC&&runtime.fault==EXEC_OK&&stage==3&&!frame&&!excluded);
 assert(queue.read==0&&queue.write==1&&state.newjob==old_pending&&state.core[0][EXEC_PRIORITY]==old_priority);
 assert(binding.display_job.descent.basic.control.superbank==060&&binding.display_job.descent.basic.control.interrupt_enabled==0);
 p66_native_export_executive(&image,&state);assert(!memcmp(&image,&expected,sizeof image));
 puts("PASS: original1137 full2050-field higher-priority vertical fallthrough; prepare/body/finish frames return, pending NEWJOB and arriving IRQ retained");return 0;
}
