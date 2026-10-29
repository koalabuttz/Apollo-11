/* SPDX-License-Identifier: MIT */
#include "cm_irq_sequence.h"
typedef struct {
 cm_irq_runtime *irq; cm_irq_publish publish; void *producer_user;
 cm_dispatch_observer observe; void *observer_user;
 uint8_t n,step,manual_poll; unsigned errors;
} sequence;
static enum exec_status post(cm_irq_queue *q,const cm_irq_request *r,void *user) { (void)user; return cm_irq_post(q,r); }
static void observed(sequence *d,enum exec_status status,exec_address next) {
 if(d->observe!=0) d->observe(d->observer_user,d->step,d->irq->native->state,status,next);
 ++d->step;
}
static void payload(cm_executive *s,sequence *d,uint8_t pattern) {
 uint8_t i; agc_word base=(agc_word)(s->core[0][CM_EXEC_PRIORITY]&CM_EXEC_LOW9);
 for(i=0;i<7;++i) if(s->core[0][i]!=(agc_word)(pattern*0100u+i)) ++d->errors;
 if(s->core[0][CM_EXEC_MODE]!=((pattern&1u) ? AGC_NEG_ZERO : 0) || s->ovfind!=(agc_word)(pattern&1u)) ++d->errors;
 for(i=0;i<5;++i) if(base==(agc_word)(CM_EXEC_VAC1USE+i*44u+1u) && s->vac[i][1]!=(agc_word)(pattern*01000u+7u)) ++d->errors;
}
static enum cm_exec_action low(cm_executive *s,void *user,exec_address *resume) {
 sequence *d=user; cm_dispatch_event e; cm_irq_request req; exec_address zero={0,0}; enum exec_status status;
 cm_dispatch_event_prepare(&e,d->n,2); req.priority=e.priority; req.address=e.address; req.needs_vac=e.needs_vac;
 if(cm_irq_step(d->irq)!=EXEC_OUT_OF_SCOPE) ++d->errors;
 status=d->publish(d->irq->queue,&req,d->producer_user); if(status!=EXEC_OK) ++d->errors;
 if(d->manual_poll!=0) { status=cm_irq_poll(d->irq); observed(d,status,zero); if(status!=EXEC_OK) ++d->errors; }
 else ++d->step; /* Admission is verified in the following full exchange state. */
 cm_dispatch_touch(s,1); resume->location=LOW_RESUME; resume->bank=LOW_BANK; return CM_EXEC_YIELD;
}
static enum cm_exec_action high(cm_executive *s,void *user,exec_address *resume) { (void)user; cm_dispatch_touch(s,2); resume->location=HIGH_RESUME; resume->bank=HIGH_BANK; return CM_EXEC_YIELD; }
static enum cm_exec_action low_resume(cm_executive *s,void *user,exec_address *resume) { payload(s,user,1); cm_dispatch_touch(s,3); resume->location=LOW_FINISH; resume->bank=LOW_BANK; return CM_EXEC_YIELD; }
static enum cm_exec_action high_finish(cm_executive *s,void *user,exec_address *resume) { (void)resume; payload(s,user,2); cm_dispatch_touch(s,4); return CM_EXEC_FINISH; }
static enum cm_exec_action low_finish(cm_executive *s,void *user,exec_address *resume) { (void)resume; payload(s,user,3); cm_dispatch_touch(s,5); return CM_EXEC_FINISH; }
static const cm_exec_mapping maps[]={{{LOW_ENTRY,LOW_BANK},low},{{LOW_RESUME,LOW_BANK},low_resume},{{LOW_FINISH,LOW_BANK},low_finish},{{HIGH_ENTRY,HIGH_BANK},high},{{HIGH_RESUME,HIGH_BANK},high_finish}};
static const cm_exec_context_mapping typed[]={{{LOW_ENTRY,LOW_BANK},CM_EXEC_CONTEXT_BASIC,low},{{LOW_RESUME,LOW_BANK},CM_EXEC_CONTEXT_BASIC,low_resume},{{LOW_FINISH,LOW_BANK},CM_EXEC_CONTEXT_BASIC,low_finish},{{HIGH_ENTRY,HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high},{{HIGH_RESUME,HIGH_BANK},CM_EXEC_CONTEXT_BASIC,high_finish}};
unsigned cm_irq_sequence_run(cm_executive *s,uint8_t n,const exec_exclusion *x,cm_irq_publish publish,void *producer_user,cm_dispatch_observer observe,void *observer_user,uint8_t manual_poll,uint8_t typed_mapping) {
 cm_exec_native native; cm_irq_runtime irq; cm_irq_queue q; cm_dispatch_event e;
 exec_address zero={0,0}; enum exec_status status; sequence d;
 d.irq=&irq; d.publish=publish!=0 ? publish : post; d.producer_user=producer_user;
 d.observe=observe; d.observer_user=observer_user; d.n=n; d.step=0; d.manual_poll=manual_poll; d.errors=0;
 cm_dispatch_prepare(s,n); cm_irq_init(&q);
 status=typed_mapping!=0 ? cm_exec_native_context_init(&native,s,typed,5,&d) : cm_exec_native_init(&native,s,maps,5,&d);
 if(status!=EXEC_OK || cm_irq_bind(&irq,&native,&q,x)!=EXEC_OK) return 1;
 cm_dispatch_event_prepare(&e,n,0); status=cm_irq_admit(&irq,e.priority,e.address,e.needs_vac); observed(&d,status,zero);
 if(status!=EXEC_OK) return d.errors+1u;
 status=cm_irq_start(&irq); observed(&d,status,native.current);
 if(status!=EXEC_OK) return d.errors+1u;
 while(native.running!=0 && d.step<9) { status=cm_irq_step(&irq); observed(&d,status,native.current); if(status!=EXEC_OK && status!=EXEC_IDLE) return d.errors+1u; }
 if(d.step!=8 || status!=EXEC_IDLE || q.write!=1 || q.read!=1 || q.overflow!=0) ++d.errors;
 return d.errors;
}
