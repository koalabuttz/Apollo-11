/* SPDX-License-Identifier: MIT */
#include "p66_pending_initializer.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_pending_initializer_arguments *args=(p66_pending_initializer_arguments *)(uintptr_t)selector;
 p66_pending_initializer_callbacks const*c;p66_application_pending*p;
 p66_application_recurrence*r;p66_display_job_binding*full;p66_descent_binding*b;
 if(!args||!args->binding||!args->callbacks)return 0xffff;
 c=args->callbacks;p=args->binding;r=&p->recurrence;full=&r->application.display_job;b=&full->descent;
 b->basic.arithmetic.image=image;b->basic.arithmetic.producers=c->guard;
 b->basic.arithmetic.observe=c->observe;b->basic.arithmetic.observer=0;b->basic.arithmetic.status=0;
 b->math_modules=c->math;b->invoke_control=c->control;b->control_modules=0;
 full->invoke_display=c->display;full->display_modules=0;
 /* finish_source is replaced by the resident public initializer before the
  * next source entry. Avoid retaining this module's function address. */
 b->finish_source=0;b->basic.invoke_basic=0;b->basic.tasks=0;b->basic.display=0;
 r->application.invoke_application=c->application;r->application.application_modules=0;
 dsky_display_reset(&r->application.peripheral);r->in_interrupt=0;r->pending_t3=0;
 r->invoke_recurrence=c->recurrence;r->recurrence_modules=0;
 p->invoke_pending=c->pending;p->pending_modules=0;
 P66_RECURRENCE_FIXTURE->in_interrupt=&r->in_interrupt;
 P66_RECURRENCE_FIXTURE->io=&b->basic.control;P66_RECURRENCE_FIXTURE->channel14=&b->channel14;
 P66_RECURRENCE_FIXTURE->channel11=&full->channel11;
 return 0;
}
