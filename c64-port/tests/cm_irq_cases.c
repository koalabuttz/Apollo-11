/* SPDX-License-Identifier: MIT */
#include "cm_irq_cases.h"
void cm_irq_case_prepare(cm_scenario *s,uint16_t n) {
 cm_case_prepare(s,n<26 ? n : (uint16_t)(n+26u)); s->complete=0;
 s->location=(agc_word)(02000u+n%4u);
 s->bank=(agc_word)(06003u+(n%4u)*020u);
}
#ifndef CM_IRQ_INPUT_ONLY
static uint8_t enter(void *user) { (void)user; return 0; }
static void leave(void *user,uint8_t token) { (void)user; (void)token; }
static const exec_exclusion no_source={enter,leave,0};
static enum cm_exec_action noop(cm_executive *s,void *u,exec_address *r) { (void)s; (void)u; (void)r; return CM_EXEC_FINISH; }
enum exec_status cm_irq_case_run(cm_scenario *s,uint16_t n,const exec_exclusion *x) {
 cm_exec_mapping m; cm_exec_native native; cm_irq_runtime irq; cm_irq_queue q; cm_irq_request req;
 enum exec_status status;
 cm_irq_case_prepare(s,n); m.address.location=s->location; m.address.bank=s->bank; m.call=noop;
 cm_irq_init(&q); q.write=q.read=(uint8_t)n;
 status=cm_exec_native_init(&native,&s->state,&m,1,0); if(status!=EXEC_OK) return status;
 status=cm_irq_bind(&irq,&native,&q,x!=0 ? x : &no_source); if(status!=EXEC_OK) return status;
 req.priority=s->priority; req.address=m.address; req.needs_vac=s->needs_vac;
 status=cm_irq_post(&q,&req); if(status!=EXEC_OK) return status;
 status=cm_irq_poll(&irq);
 if(q.write!=(uint8_t)(n+1u) || q.read!=(uint8_t)(status==EXEC_OK ? n+1u : n) || q.overflow!=0 || q.rejected!=0) return EXEC_INVALID_ACTION;
 return status;
}
#endif
