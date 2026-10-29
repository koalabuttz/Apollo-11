/* SPDX-License-Identifier: MIT */
/* Reuse the independent native factory/compiled bodies, stop at the literal
 * source-selected mode job, and verify an arriving IRQ ticket is not polled. */
#define main inherited_pending_main
#include "p66_pending_flight_snapshot.c"
#undef main
#include <assert.h>
typedef struct {observer output;exec_irq_queue *queue;uint8_t irq,posted;} adoption;
static uint8_t exclude(void *user){adoption *a=user;uint8_t old=a->irq;a->irq=0;return old;}
static void restore(void *user,uint8_t old){((adoption *)user)->irq=old;}
static void observe_adoption(void *user,const apollo_overlay_state *image,agc_word loc){
 adoption *a=user;observe(&a->output,image,loc);
 if(loc==03534u){exec_request request={030000,{02546,020067},0};
  assert(a->irq==0&&a->posted==0);
  assert(exec_irq_post(a->queue,&request)==EXEC_OK);a->posted=1;}
}
int main(void){uint16_t n;
 for(n=0;n<16u;++n){apollo_overlay_state image;executive state;exec_native runtime;
  p66_application_pending app;exec_irq_queue queue;adoption output={{n,0,&app,0},&queue,(uint8_t)(n&1u),0};
  exec_exclusion guard={exclude,restore,&output};p66_guidance_modules modules={call,0};
  p66_descent_binding *b=&app.recurrence.application.display_job.descent;unsigned steps;
  memset(&app,0,sizeof app);prl_prepare(&image,&state,&b->basic.control,&b->channel14,
                                     &app.recurrence.application.display_job.channel11,n);
  b->basic.arithmetic.image=&image;b->basic.arithmetic.producers=&guard;
  b->basic.arithmetic.observe=observe_adoption;b->basic.arithmetic.observer=&output;
  b->math_modules=&modules;b->invoke_control=control;
  assert(p66_application_pending_init(&runtime,&state,&app)==EXEC_OK);
  exec_irq_init(&queue);assert(exec_native_bind_irq(&runtime,&queue,&guard)==EXEC_OK);
  runtime.running=1;runtime.current.location=02477;
  runtime.current.bank=(uint16_t)(062007u|b->basic.control.superbank);
  /* A lookup used solely for validation must not leave a stale translated
   * callback: the actual prepare resolves its selected continuation again. */
  assert(exec_native_is_mapped(&runtime,(exec_address){03434,064007},EXEC_CONTEXT_INTERPRETIVE));
  for(steps=0;steps<8u&&!output.posted;++steps)
   assert(p66_application_pending_step(&app)==EXEC_OK);
  assert(output.posted==1&&output.output.stage==4);
  assert(runtime.current_kind==EXEC_CONTEXT_BASIC&&runtime.current.location==03534u&&runtime.current.bank==060102u);
  assert(runtime.fault==EXEC_OK&&runtime.in_callback==0);
  assert(queue.write==1&&queue.read==0&&queue.overflow==0&&queue.rejected==0);
  assert(output.irq==(uint8_t)(n&1u));
 }
 return 0;
}
