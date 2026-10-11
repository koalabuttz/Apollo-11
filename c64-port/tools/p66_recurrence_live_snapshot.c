/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_application_recurrence.h"
#include "p66_length_return_control.h"
#include "p66_recurrence_live_cases.h"
#include "p66_descent_control.h"
#include "p66_native_descent.h"
typedef struct {uint16_t number,stage;p66_application_recurrence *binding;} observer;
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void *u,uint8_t old){(void)u;(void)old;}
static uint16_t call(void *u,apollo_overlay_state *m,uint16_t selector){(void)u;uint16_t status=selector<=5?p66_native_entry(m,selector):selector<190?p66_native_guidance_entry(m,selector):p66_native_descent_entry(m,selector);if(!status&&(selector==212u||selector==236u))status=p66_length_return_control(m,selector);if(!status&&selector==193u)status=p66_feedback_store_return_control(m);return status;}
static uint16_t control(void *u,apollo_overlay_state *m,p66_bias_io *io,agc_word *c,uint16_t selector){(void)u;return p66_descent_control_entry(m,io,c,selector);}
static void observe(void *u,const apollo_overlay_state *m,agc_word loc){observer *o=u;p66_display_job_binding *d=&o->binding->application.display_job;p66_descent_binding *b=&d->descent;unsigned i;printf("%u %u %u",o->number,o->stage++,loc);for(i=0;i<2048u;++i)printf(" %u",m->words[i]);printf(" %u %u %u %u %u\n",b->basic.control.superbank,b->basic.control.interrupt_enabled,b->channel14,d->channel11,o->binding->in_interrupt);}
static uint16_t recurrence(void *u,p66_application_recurrence *b,exec_address *next,uint16_t selector,uint16_t ticks,uint16_t *consumed){observer *o=u;uint16_t status=p66_recurrence_entry(b,next,selector,ticks,consumed);apollo_overlay_state *image=b->application.display_job.descent.basic.arithmetic.image;if(status==0&&selector!=P66_RECURRENCE_PREPARE&&selector!=P66_RECURRENCE_ADOPT)observe(o,image,image->words[5]);return status;}
int main(void){uint16_t n;static const exec_exclusion guard={enter,leave,0};
 for(n=0;n<16u;++n){apollo_overlay_state image;executive state;exec_native runtime;p66_application_recurrence app;p66_descent_binding *b=&app.application.display_job.descent;p66_guidance_modules modules={call,0};observer o={n,0,&app};unsigned step;enum exec_status status=EXEC_OK;
  memset(&app,0,sizeof app);prl_prepare(&image,&state,&b->basic.control,&b->channel14,&app.application.display_job.channel11,n);
  b->basic.arithmetic.image=&image;b->basic.arithmetic.producers=&guard;b->basic.arithmetic.observe=observe;b->basic.arithmetic.observer=&o;b->math_modules=&modules;b->invoke_control=control;
  app.invoke_recurrence=recurrence;app.recurrence_modules=&o;
  if(p66_application_init(&runtime,&state,&app.application)!=EXEC_OK)return 2;
  runtime.running=1;runtime.current.location=02477;runtime.current.bank=(uint16_t)(062007u|b->basic.control.superbank);
  for(step=0;step<100u;++step){status=p66_basic_step(&b->basic);if(status!=EXEC_OK)break;}
  if(status!=EXEC_IDLE||runtime.fault!=EXEC_OK){fprintf(stderr,"case%u step%u native%u PC%o LOC%o status%u\n",n,step,status,image.words[5],image.words[0164],b->basic.arithmetic.status);return 3;}
  {uint16_t consumed=0;status=p66_application_recur(&app,100,&consumed);if(status!=EXEC_OK||consumed!=100){fprintf(stderr,"recur%u status%u consumed%u TIME3%o PC%o NEWJOB%o RUPTAGN%o IRQ%u\n",n,status,consumed,image.words[026],image.words[5],image.words[067],image.words[0734],b->basic.control.interrupt_enabled);return 4;}}
  for(step=0;step<100u;++step){status=p66_basic_step(&b->basic);if(status!=EXEC_OK)break;}
  if(status!=EXEC_IDLE||runtime.fault!=EXEC_OK){fprintf(stderr,"second%u native%u PC%o LOC%o status%u\n",n,status,image.words[5],image.words[0164],b->basic.arithmetic.status);return 5;}
 }return 0;
}
