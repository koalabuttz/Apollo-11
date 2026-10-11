/* SPDX-License-Identifier: MIT */
#include "p66_application_recurrence.h"
#ifndef P66_RECURRENCE_DISPATCH_ONLY
enum exec_status p66_application_recur_prepare(p66_application_recurrence *b,uint16_t centiseconds,uint16_t *consumed){p66_native_binding *a;p66_bias_io *io;exec_native *r;uint16_t *w;
 if(!b||!consumed)return EXEC_OUT_OF_SCOPE;
 a=&b->application.display_job.descent.basic.arithmetic;r=a->runtime;io=&b->application.display_job.descent.basic.control;
 if(!a->image||!r||!r->state||!a->producers||!a->producers->enter||!a->producers->leave||r->fault!=EXEC_OK||!r->resolve||r->running||r->in_callback||b->in_interrupt||b->pending_t3>1u)return EXEC_OUT_OF_SCOPE;
 w=a->image->words;
 if(w[5]!=03214u||w[4]!=02000u||w[7]!=0||agc_overflow(w[0])||io->interrupt_enabled!=1u||w[067]!=AGC_NEG_ZERO||w[0167]!=AGC_NEG_ZERO||w[01410]!=03263u||(w[01411]!=064007u&&w[01411]!=064067u)||w[024]>037777u||w[025]>037777u||w[026]>037777u)return EXEC_OUT_OF_SCOPE;
 *consumed=0;while(*consumed<centiseconds&&!b->pending_t3){if(lm_clock_advance(a->image,1,&b->pending_t3)!=0)return EXEC_OUT_OF_SCOPE;++*consumed;}
 if(!b->pending_t3)return EXEC_IDLE;
 /* Hardware capture at known literal CCS NEWJOB. No instruction decode. */
 w[015]=03215;w[017]=010067;w[5]=04014;b->in_interrupt=1;b->pending_t3=0;return EXEC_OK;
}
enum exec_status p66_application_recur_adopt(p66_application_recurrence *b,exec_address next){p66_native_binding *a=&b->application.display_job.descent.basic.arithmetic;exec_native *r=a->runtime;if(next.location!=03275u||next.bank!=064067u||!r->resolve(r->registry_user,next,EXEC_CONTEXT_BASIC))return EXEC_UNMAPPED;p66_native_import_executive(r->state,a->image);r->current=next;r->current_kind=EXEC_CONTEXT_BASIC;r->running=1;return EXEC_OK;}
uint16_t p66_recurrence_entry(p66_application_recurrence *b,exec_address *next,uint16_t selector,uint16_t ticks,uint16_t *consumed){p66_descent_binding *d;apollo_overlay_state *image;p66_bias_io *io;if(!b||!next)return 0xffff;d=&b->application.display_job.descent;image=d->basic.arithmetic.image;io=&d->basic.control;switch(selector){case P66_RECURRENCE_PREPARE:return (uint16_t)p66_application_recur_prepare(b,ticks,consumed);case P66_RECURRENCE_ADOPT:return (uint16_t)p66_application_recur_adopt(b,*next);case P66_RECURRENCE_RODTASK:return (uint16_t)p66_rodtask_image(image,io);case P66_RECURRENCE_DISPATCH:return (uint16_t)lm_waitlist_irq_dispatch(image,io,b->in_interrupt,next);case P66_RECURRENCE_RESUME:return (uint16_t)lm_taskover_resume(image,io,&b->in_interrupt);case P66_RECURRENCE_IDLE:return (uint16_t)lm_idle_start(image,io,&b->application.display_job.channel11,next);default:return 0xffff;}}
#endif
static uint16_t call(p66_application_recurrence *b,exec_address *next,uint16_t selector,uint16_t ticks,uint16_t *consumed){
 if(b->invoke_recurrence)return b->invoke_recurrence(b->recurrence_modules,b,next,selector,ticks,consumed);
#ifndef P66_RECURRENCE_DISPATCH_ONLY
 return p66_recurrence_entry(b,next,selector,ticks,consumed);
#else
 return 0xffff;
#endif
}
enum exec_status p66_application_recur(p66_application_recurrence *b,uint16_t centiseconds,uint16_t *consumed){p66_native_binding *a;exec_address next={0,0};uint16_t status;uint8_t token;
 if(!b||!consumed)return EXEC_OUT_OF_SCOPE;
 a=&b->application.display_job.descent.basic.arithmetic;
 if(!a->producers||!a->producers->enter||!a->producers->leave)return EXEC_OUT_OF_SCOPE;
 token=a->producers->enter(a->producers->user);status=call(b,&next,P66_RECURRENCE_PREPARE,centiseconds,consumed);
 if(!status){status=call(b,&next,P66_RECURRENCE_DISPATCH,0,consumed);if(!status&&next.location==03263u&&(next.bank==064007u||next.bank==064067u))status=call(b,&next,P66_RECURRENCE_RODTASK,0,consumed);else if(!status)status=0xffff;
  if(!status)status=call(b,&next,P66_RECURRENCE_RESUME,0,consumed);
  if(!status)status=call(b,&next,P66_RECURRENCE_IDLE,0,consumed);
  if(!status)status=call(b,&next,P66_RECURRENCE_ADOPT,0,consumed);
  if(!status)dsky_display_output(&b->application.peripheral,011,b->application.display_job.channel11);
 }
 a->producers->leave(a->producers->user,token);return (enum exec_status)status;
}
