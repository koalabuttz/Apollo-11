/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_descent_runtime.h"
#include "p66_descent_control.h"
#include "p66_native_descent.h"
#include "p66_descent_live_cases.h"
#include "p66_descent_pending_counts.h"
typedef struct {uint16_t number,stage,phase; p66_descent_binding *binding;} observer;
static uint8_t enter(void *u){(void)u;return 0;}
static void leave(void *u,uint8_t p){(void)u;(void)p;}
static uint16_t call(void *u,apollo_overlay_state *m,uint16_t selector){(void)u;return selector<=5?p66_native_entry(m,selector):selector<190?p66_native_guidance_entry(m,selector):p66_native_descent_entry(m,selector);}
static uint16_t control(void *u,apollo_overlay_state *m,p66_bias_io *io,agc_word *channel,uint16_t selector){(void)u;return p66_descent_control_entry(m,io,channel,selector);}
static void observe(void *u,const apollo_overlay_state *m,agc_word loc){observer *o=u;uint16_t words[PDL_WORDS];unsigned i;p66_descent_binding *b=o->binding;
 if(o->stage>=o->phase){unsigned higher=o->stage>o->phase;if(higher&&loc!=03353){fprintf(stderr,"unexpected higher callback LOC%o\n",loc);return;}pdl_words(m,b->basic.control,b->channel14,words,higher?0:o->stage);printf("%u %u %u",o->number,o->phase,higher);for(i=0;i<PDL_WORDS;++i)printf(" %u",words[i]);puts("");}++o->stage;
}
static enum exec_status prepare(void *u,exec_native *r,exec_continuation *call){(void)u;return exec_native_prepare(r,call);}
static enum exec_status finish(void *u,exec_native *r,enum exec_action action,exec_address resume){(void)u;return exec_native_finish(r,action,resume);}
int main(int argc,char **argv){uint16_t n,phase;uint8_t resolved=0,services=0;int arg;static const exec_exclusion guard={enter,leave,0};static const exec_native_services hooks={prepare,finish};
 for(arg=1;arg<argc;++arg){if(!strcmp(argv[arg],"--resolved"))resolved=1;else if(!strcmp(argv[arg],"--services"))services=1;else return 2;}
 for(n=0;n<16u;++n)for(phase=23u;phase<=pending_counts[n]-3u;++phase){apollo_overlay_state image;executive state;exec_native runtime;exec_context_mapping maps[P66_DESCENT_MAPPING_COUNT];p66_descent_binding binding;observer o={n,0,phase,&binding};p66_guidance_modules modules={call,0};unsigned step;
  memset(&binding,0,sizeof binding);binding.basic.arithmetic.image=&image;binding.basic.arithmetic.producers=&guard;binding.basic.arithmetic.observe=observe;binding.basic.arithmetic.observer=&o;binding.math_modules=&modules;binding.invoke_control=control;
  pdl_prepare(&image,&state,&binding.basic.control,&binding.channel14,n);pgl_prepare_high(&image,n);p66_native_import_executive(&state,&image);
  if(p66_descent_init(&runtime,&state,&binding,maps,P66_DESCENT_MAPPING_COUNT,0,0)!=EXEC_OK)return 2;
  if(resolved){uint8_t i;for(i=0;i<P66_DESCENT_MAPPING_COUNT;++i)if(p66_descent_resolve(&binding,maps[i].address,maps[i].kind)!=maps[i].call)return 5;
   if(p66_descent_resolve(&binding,(exec_address){06000,064007},EXEC_CONTEXT_BASIC)!=0||p66_descent_resolve(&binding,(exec_address){03434,077777},EXEC_CONTEXT_INTERPRETIVE)!=0)return 5;
   if(p66_descent_init_resolved(&runtime,&state,&binding)!=EXEC_OK)return 5;
  }
  if(services&&exec_native_bind_services(&runtime,&hooks,0)!=EXEC_OK)return 6;
  runtime.running=1;runtime.current.location=03275;runtime.current.bank=n&1u?064067:064007;
  for(step=0;step<=phase+1u;++step){enum exec_status status;if(step==phase)image.words[0067]=12;status=p66_basic_step(&binding.basic);if(status!=EXEC_OK){fprintf(stderr,"case%u phase%u step%u status%u\n",n,phase,step,status);return 3;}if(step==phase&&(runtime.current_kind!=EXEC_CONTEXT_BASIC||runtime.current.location!=03275))return 4;}
  if(o.stage!=phase+2u||runtime.fault!=EXEC_OK)return 5;
 }
 return 0;
}
