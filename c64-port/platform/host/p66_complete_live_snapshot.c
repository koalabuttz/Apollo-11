/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_descent_runtime.h"
#include "p66_descent_control.h"
#include "p66_native_descent.h"
#include "p66_complete_live_cases.h"
typedef struct {uint16_t number,stage; p66_display_job_binding *binding;} observer;
static uint8_t enter(void *u){(void)u;return 0;}
static void leave(void *u,uint8_t p){(void)u;(void)p;}
static uint16_t call(void *u,apollo_overlay_state *m,uint16_t selector){(void)u;return selector<=5?p66_native_entry(m,selector):selector<190?p66_native_guidance_entry(m,selector):p66_native_descent_entry(m,selector);}
static uint16_t control(void *u,apollo_overlay_state *m,p66_bias_io *io,agc_word *channel,uint16_t selector){(void)u;return p66_descent_control_entry(m,io,channel,selector);}
static exec_continuation resolve_display(void*u,exec_address a,enum exec_context_kind kind){uint8_t i;(void)u;for(i=0;i<P66_DISPLAY_JOB_MAPPING_COUNT;++i){const exec_context_mapping*m=&p66_display_job_mappings[i];if(m->address.location==a.location&&m->address.bank==a.bank&&m->kind==kind)return m->call;}return 0;}
static uint16_t display(void*u,apollo_overlay_state*m,p66_bias_io*io,agc_word*channel,uint16_t selector){(void)u;return p66_display_job_entry(m,io,channel,selector);}
static void observe(void *u,const apollo_overlay_state *m,agc_word loc){observer *o=u;uint16_t words[PCL_WORDS];unsigned i;p66_display_job_binding *full=o->binding;p66_descent_binding *b=&full->descent;
 pcl_words(m,b->basic.control,b->channel14,full->channel11,words,o->stage);printf("%u %u %u",o->number,o->stage++,loc);for(i=0;i<PCL_WORDS;++i)printf(" %u",words[i]);puts("");
}
static enum exec_status prepare(void *u,exec_native *r,exec_continuation *call){(void)u;return exec_native_prepare(r,call);}
static enum exec_status finish(void *u,exec_native *r,enum exec_action action,exec_address resume){(void)u;return exec_native_finish(r,action,resume);}
int main(int argc,char **argv){uint16_t n;uint8_t services=0;int arg;static const exec_exclusion guard={enter,leave,0};static const exec_native_services hooks={prepare,finish};
 for(arg=1;arg<argc;++arg){if(!strcmp(argv[arg],"--resolved")){}else if(!strcmp(argv[arg],"--services"))services=1;else return 2;}
 for(n=0;n<16u;++n){apollo_overlay_state image;executive state;exec_native runtime;p66_display_job_binding full;p66_descent_binding *binding=&full.descent;observer o={n,0,&full};p66_guidance_modules modules={call,0};unsigned step;
  memset(&full,0,sizeof full);binding->basic.arithmetic.image=&image;binding->basic.arithmetic.producers=&guard;binding->basic.arithmetic.observe=observe;binding->basic.arithmetic.observer=&o;binding->math_modules=&modules;binding->invoke_control=control;binding->resolve_extra=resolve_display;binding->finish_source=p66_display_job_finish;full.invoke_display=display;
  pcl_prepare(&image,&state,&binding->basic.control,&binding->channel14,&full.channel11,n);
  if(p66_descent_init_resolved(&runtime,&state,binding)!=EXEC_OK)return 2;
  if(services&&exec_native_bind_services(&runtime,&hooks,0)!=EXEC_OK)return 6;
  runtime.running=1;runtime.current.location=03275;runtime.current.bank=n&1u?064067:064007;
  for(step=0;step<80u;++step){enum exec_status status=p66_basic_step(&binding->basic);
   if(status==EXEC_IDLE)break;
   if(status!=EXEC_OK){fprintf(stderr,"case%u step%u exec%u arithmetic%u basic%u PC%o LOC%o current%o/%o MODE%o PUSH%o EDOP%o\n",n,step,status,binding->basic.arithmetic.status,binding->basic.status,image.words[5],image.words[0164],runtime.current.location,runtime.current.bank,image.words[0163],image.words[0166],image.words[023]);return 3;}
  }
  if(step==80u||o.stage<40u||runtime.fault!=EXEC_OK||state.newjob!=AGC_NEG_ZERO)return 4;
 }
 return 0;
}
