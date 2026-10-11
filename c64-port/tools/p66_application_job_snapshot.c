/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_application.h"
#include "p66_application_job_cases.h"
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void *u,uint8_t old){(void)u;(void)old;}
static uint16_t unused_math(void *u,apollo_overlay_state *m,uint16_t selector){(void)u;(void)m;(void)selector;return 0xffff;}
static uint16_t unused_control(void *u,apollo_overlay_state *m,p66_bias_io *io,agc_word *c,uint16_t selector){(void)u;(void)m;(void)io;(void)c;(void)selector;return 0xffff;}
int main(void){uint16_t n;static const exec_exclusion guard={enter,leave,0};
 for(n=0;n<P66_APPLICATION_JOB_COUNT;++n){apollo_overlay_state image;executive state;exec_native runtime;p66_application_binding app;p66_descent_binding *b=&app.display_job.descent;p66_guidance_modules modules={unused_math,0};unsigned i;enum exec_status status;
  memset(&app,0,sizeof app);p66_application_job_input(&image,&b->basic.control,&app.display_job.channel11,&b->channel14,n);p66_native_import_executive(&state,&image);state.alarm=0;
  b->basic.arithmetic.image=&image;b->basic.arithmetic.producers=&guard;b->math_modules=&modules;b->invoke_control=unused_control;
  if(p66_application_init(&runtime,&state,&app)!=EXEC_OK)return 2;
  runtime.running=1;runtime.current.location=image.words[5];runtime.current.bank=(uint16_t)(image.words[6]|b->basic.control.superbank);runtime.current_kind=EXEC_CONTEXT_BASIC;
  status=p66_basic_step(&b->basic);if(status!=EXEC_IDLE||runtime.fault!=EXEC_OK){fprintf(stderr,"case%u native%u PC%o status%u\n",n,status,image.words[5],app.status);return 3;}
  if(app.peripheral.channel11!=app.display_job.channel11)return 4;
  printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u %u %u\n",b->basic.control.superbank,b->basic.control.interrupt_enabled,app.display_job.channel11,b->channel14);
 }return 0;
}
