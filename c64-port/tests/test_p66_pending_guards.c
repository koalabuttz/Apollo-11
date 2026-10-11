/* SPDX-License-Identifier: MIT */
#define main pending_unused_snapshot_main
#include "../tools/p66_pending_flight_snapshot.c"
#undef main
#include <assert.h>
typedef struct {uint8_t irq,depth;unsigned enters,leaves,calls;uint16_t fail;} guard_test;
static uint8_t enter_guard(void *user){guard_test*g=user;uint8_t old=g->irq;
 assert(g->depth==0);g->depth=1;g->irq=0;++g->enters;return old;}
static void leave_guard(void *user,uint8_t old){guard_test*g=user;
 assert(g->depth==1&&g->irq==0);g->depth=0;g->irq=old;++g->leaves;}
static uint16_t invoke_guard(void *user,p66_application_pending*b,exec_address*r,uint16_t s){
 guard_test*g=user;assert(g->depth==1);++g->calls;
 return s==g->fail?0xffff:p66_application_pending_entry(b,r,s);}
int main(void){uint16_t n;
 assert(p66_application_pending_entry(0,0,88)==0xffff);
 assert(p66_application_pending_step(0)==EXEC_OUT_OF_SCOPE);
 for(n=0;n<2;++n){apollo_overlay_state image,before;executive state;
  exec_native runtime;p66_application_pending app;guard_test g={(uint8_t)n,0,0,0,0,98};
  exec_exclusion guard={enter_guard,leave_guard,&g};p66_guidance_modules modules={call,0};
  p66_descent_binding*b=&app.recurrence.application.display_job.descent;exec_address resume={0123,0456};
  memset(&app,0,sizeof app);prl_prepare(&image,&state,&b->basic.control,&b->channel14,
   &app.recurrence.application.display_job.channel11,n);
  b->basic.arithmetic.image=&image;b->basic.arithmetic.producers=&guard;b->math_modules=&modules;
  b->invoke_control=control;assert(p66_application_pending_init(&runtime,&state,&app)==EXEC_OK);
  runtime.running=1;runtime.current=(exec_address){02477,(uint16_t)(062007u|b->basic.control.superbank)};
  before=image;
  assert(p66_application_pending_entry(&app,&resume,0xffff)==0xffff);
  assert(p66_application_pending_entry(&app,0,88)==0xffff);
  assert(!memcmp(&image,&before,sizeof image)&&resume.location==0123&&resume.bank==0456);
  app.invoke_pending=invoke_guard;app.pending_modules=&g;
  assert(p66_application_pending_step(&app)==EXEC_OUT_OF_SCOPE);
  assert(g.calls==1&&g.enters==1&&g.leaves==1&&g.irq==n&&g.depth==0);
  assert(!memcmp(&image,&before,sizeof image)&&runtime.current.location==02477&&runtime.in_callback==0);
  /* Export transport failure retains the already completed source body. The
   * caller receives failure and must stop rather than replay the callback. */
  g.fail=99;g.calls=0;
  assert(p66_application_pending_step(&app)==EXEC_OUT_OF_SCOPE);
  assert(g.calls==2&&g.enters==g.leaves&&g.irq==n&&g.depth==0);
  assert(runtime.current.location==03275&&runtime.in_callback==0&&runtime.fault==EXEC_OK);
  assert(image.words[5]==03275&&!memcmp(&image.words[024],&before.words[024],2*sizeof(uint16_t)));
 }
 puts("PASS:pending service guards; import/export transport failures retain source bodies and restore enabled/masked exclusion");
 return 0;
}
