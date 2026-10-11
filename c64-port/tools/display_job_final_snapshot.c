/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "display_job_final_cases.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;static p66_display_job_binding binding;static uint8_t depth;static exec_context_mapping mappings[P66_DISPLAY_JOB_MAPPING_COUNT+P66_NATIVE_MAPPING_COUNT];
static uint8_t enter(void *p){uint8_t old=depth;(void)p;++depth;return old;}
static void leave(void *p,uint8_t old){(void)p;depth=old;}
static const exec_exclusion producers={enter,leave,0};
int main(void){uint16_t n,i,phase;memcpy(mappings,p66_display_job_mappings,sizeof p66_display_job_mappings);memcpy(mappings+P66_DISPLAY_JOB_MAPPING_COUNT,p66_native_mappings,sizeof p66_native_mappings);for(n=0;n<DISPLAY_JOB_FINAL_CASES;++n){p66_bias_io io;agc_word channel;display_job_final_input(&image,&io,&channel,n);memset(&state,0,sizeof state);memset(&binding,0,sizeof binding);p66_native_import_executive(&state,&image);binding.descent.basic.arithmetic.image=&image;binding.descent.basic.arithmetic.runtime=&runtime;binding.descent.basic.arithmetic.producers=&producers;binding.descent.basic.control=io;binding.channel11=channel;if(exec_native_context_init(&runtime,&state,mappings,P66_DISPLAY_JOB_MAPPING_COUNT+P66_NATIVE_MAPPING_COUNT,&binding)!=EXEC_OK)return 2;runtime.running=1;runtime.current.location=02546;runtime.current.bank=020067;for(phase=0;phase<DISPLAY_JOB_FINAL_PHASES;++phase){enum exec_status status;
#ifdef DISPLAY_JOB_DIRECT_FINISH
if(phase==23u){exec_continuation call;exec_address resume={0,0};enum exec_action action;uint8_t token=enter(0);status=exec_native_prepare(&runtime,&call);leave(0,token);if(status==EXEC_OK){runtime.current.location=03531;runtime.current.bank=064067;action=p66_display_job_finish(&state,&binding,&resume);token=enter(0);status=exec_native_finish(&runtime,action,resume);leave(0,token);}}else status=exec_native_step(&runtime);
#else
status=exec_native_step(&runtime);
#endif
if(status!=EXEC_OK&&!(phase==23u&&n%3u==0u&&status==EXEC_IDLE)){fprintf(stderr,"case%u phase%u native%u entry%05o/%05o module%u\n",n,phase,(unsigned)status,runtime.current.location,runtime.current.bank,binding.status);return 2;}if(depth!=0)return 2;printf("%u %u 0",n,phase);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",image.words[i]);printf(" %u %u %u\n",binding.descent.basic.control.superbank,binding.descent.basic.control.interrupt_enabled,binding.channel11);}if(n%3u==2u&&(runtime.current.location!=03354u||runtime.current_kind!=EXEC_CONTEXT_INTERPRETIVE||runtime.current.bank!=064067u))return 2;}return 0;}
