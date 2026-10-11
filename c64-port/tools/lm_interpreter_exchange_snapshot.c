/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_interpreter_change_cases.h"
#include "lm_interpreter_exchange_native.h"
#include "p66_native.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void *u,uint8_t old){(void)u;(void)old;}
int main(void){unsigned n,i;static const exec_exclusion guard={enter,leave,0};for(n=0;n<LM_INTERPRETER_CHANGE_CASES;++n){p66_bias_io io;lm_interpreter_exchange_binding binding;enum exec_status status;lm_interpreter_change_input(&image,&io,(uint16_t)n);memset(&binding,0,sizeof binding);binding.runtime=&runtime;binding.image=&image;binding.io=&io;p66_native_import_executive(&state,&image);state.alarm=0;
 if(exec_native_resolver_init(&runtime,&state,lm_interpreter_exchange_resolve,&binding,&binding)!=EXEC_OK)return 2;
 runtime.exclusion=&guard;runtime.running=1;runtime.current_kind=EXEC_CONTEXT_INTERPRETIVE;runtime.current.location=05126;runtime.current.bank=(uint16_t)(image.words[6]|io.superbank);runtime.interpreter_control.edop=0;runtime.interpreter_control.intbit15=040000;
 status=exec_native_step(&runtime);if(status!=EXEC_OK||binding.callbacks!=1||binding.status!=EXEC_OK||runtime.current.location!=binding.transfer.address.location||runtime.current.bank!=binding.transfer.address.bank||runtime.current_kind!=(binding.transfer.interpretive?EXEC_CONTEXT_INTERPRETIVE:EXEC_CONTEXT_BASIC))return 3;
 p66_native_export_executive(&image,&state);printf("%u %u",n,status);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",image.words[i]);printf(" %u %u %u %u\n",io.superbank,io.interrupt_enabled,lm_interpreter_change_channel11((uint16_t)n),lm_interpreter_change_channel14((uint16_t)n));}return 0;}
