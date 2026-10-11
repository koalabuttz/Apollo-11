/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_basic.h"
#include "p66_basic_task_cases.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;static waitlist wait;static wl_native tasks;static p66_basic_binding binding;
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void *u,uint8_t t){(void)u;(void)t;}static const exec_exclusion guard={enter,leave,0};
int main(void){uint16_t n,i;for(n=0;n<P66_BASIC_TASK_CASE_COUNT;++n){p66_basic_task_prepare(&image,n);memset(&binding,0,sizeof binding);state.alarm=0;p66_native_import_executive(&state,&image);binding.arithmetic.image=&image;binding.arithmetic.runtime=&runtime;binding.arithmetic.producers=&guard;
 if(exec_native_context_init(&runtime,&state,p66_native_mappings,P66_NATIVE_MAPPING_COUNT,&binding)!=EXEC_OK)return 2;
 if(p66_basic_task_init(&binding,&tasks,&wait)!=WL_OK)return 2;
 tasks.current.location=03263;tasks.current.bank=(uint16_t)(image.words[6]|((n/432u)%4u*020u));binding.control.superbank=(uint16_t)((n/432u)%4u*020u);binding.control.interrupt_enabled=(uint16_t)(n%2u);
 (void)p66_basic_task_mappings[1].call(&tasks,&binding);printf("%u %u",n,binding.status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",image.words[i]);printf(" %u %u\n",binding.control.superbank,binding.control.interrupt_enabled);
 }return 0;}
