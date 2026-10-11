/* SPDX-License-Identifier: MIT
 * Actual resident STARTP66 callback against retained complete original image. */
#include <stdio.h>
#include <string.h>
#include "p66_basic.h"
#include "p66_launch_vectors.h"
static apollo_overlay_state image,expected;
static executive state;
static exec_native runtime;
static p66_basic_binding binding;
static exec_context_mapping combined[P66_NATIVE_MAPPING_COUNT+P66_BASIC_MAPPING_COUNT];
static uint8_t excluded,observed;
static uint16_t errors;
static uint8_t enter(void *user){uint8_t old=excluded;(void)user;excluded=1;return old;}
static void leave(void *user,uint8_t old){(void)user;excluded=old;}
static const exec_exclusion guard={enter,leave,0};
static void observe(void *user,const apollo_overlay_state *s,agc_word pc){uint16_t i;(void)user;if(pc!=03275 || excluded!=1)++errors;for(i=0;i<APOLLO_SHARED_WORDS;++i)if(s->words[i]!=expected.words[i])++errors;if(binding.control.superbank!=0 || binding.control.interrupt_enabled!=0)++errors;observed=1;}
int main(void){uint8_t i,found=0;uint16_t p;
 p66_launch_prepare(&image,0);expected=image;
 for(p=p66_launch_offsets[0];p<p66_launch_offsets[1];++p)if(p66_launch_patch[p][0]<APOLLO_SHARED_WORDS)expected.words[p66_launch_patch[p][0]]=p66_launch_patch[p][1];
 for(i=0;i<P66_NATIVE_MAPPING_COUNT;++i)combined[i]=p66_native_mappings[i];
 for(i=0;i<P66_BASIC_MAPPING_COUNT;++i)combined[P66_NATIVE_MAPPING_COUNT+i]=p66_basic_mappings[i];
 p66_native_import_executive(&state,&image);binding.arithmetic.image=&image;binding.arithmetic.runtime=&runtime;binding.arithmetic.producers=&guard;binding.arithmetic.observe=observe;
 if(exec_native_context_init(&runtime,&state,combined,P66_NATIVE_MAPPING_COUNT+P66_BASIC_MAPPING_COUNT,&binding)!=EXEC_OK)++errors;
 runtime.running=1;runtime.current.location=02477;runtime.current.bank=062007;
 if(p66_basic_step(&binding)!=EXEC_OK || binding.status!=0 || observed!=1 || excluded!=0 || runtime.current.location!=03275 || runtime.current.bank!=064007)++errors;
 for(i=0;i<9;++i)if(image.words[01410+2u*i]==03263 && image.words[01411+2u*i]==064007)++found;
 if(image.words[01011]!=66 || found!=1 || image.words[0200]!=03534 || image.words[0201]!=060102)++errors;
 printf("P66 BASIC LAUNCH: real STARTP66, full original2050word checkpoint, actual queued display/task and RODCOMP transfer: %u failures\n",errors);return errors!=0;
}
