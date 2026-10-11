/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "display_job_cases.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;static p66_display_job_binding binding;static uint8_t depth;
static uint8_t enter(void *p){uint8_t old=depth;(void)p;++depth;return old;}
static void leave(void *p,uint8_t old){(void)p;depth=old;}
static const exec_exclusion producers={enter,leave,0};
int main(void){uint16_t n,i,phase;for(n=0;n<DISPLAY_JOB_CASES;++n){p66_bias_io io;agc_word channel;display_job_input(&image,&io,&channel,n);memset(&state,0,sizeof state);memset(&binding,0,sizeof binding);p66_native_import_executive(&state,&image);binding.descent.basic.arithmetic.image=&image;binding.descent.basic.arithmetic.runtime=&runtime;binding.descent.basic.arithmetic.producers=&producers;binding.descent.basic.control=io;binding.channel11=channel;if(exec_native_context_init(&runtime,&state,p66_display_job_mappings,P66_DISPLAY_JOB_MAPPING_COUNT,&binding)!=EXEC_OK)return 2;runtime.running=1;runtime.current.location=02546;runtime.current.bank=020067;for(phase=0;phase<DISPLAY_JOB_PHASES;++phase){enum exec_status status=exec_native_step(&runtime);if(status!=EXEC_OK){fprintf(stderr,"case%u phase%u native%u entry%05o/%05o module%u\n",n,phase,(unsigned)status,runtime.current.location,runtime.current.bank,binding.status);return 2;}if(depth!=0)return 2;printf("%u %u 0",n,phase);for(i=0;i<APOLLO_SHARED_WORDS;++i)printf(" %u",image.words[i]);printf(" %u %u %u\n",binding.descent.basic.control.superbank,binding.descent.basic.control.interrupt_enabled,binding.channel11);}if(image.words[5]!=05155)return 2;}return 0;}
