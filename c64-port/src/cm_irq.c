/* SPDX-License-Identifier: MIT */
#include "cm_irq.h"
void cm_irq_init(cm_irq_queue *q) { q->write=0; q->read=0; q->overflow=0; q->rejected=0; }
enum exec_status cm_irq_post(cm_irq_queue *q,const cm_irq_request *r) {
 uint8_t ticket,slot;
 if(q==0 || r==0 || r->priority==0 || (r->priority&0140777)!=0 ||
    r->address.location==0 || r->address.location>07777 ||
    (r->address.bank&(agc_word)~UINT16_C(076167))!=0 || r->needs_vac>1) return EXEC_OUT_OF_SCOPE;
 ticket=q->write;
 if(q->overflow!=0 || (uint8_t)(ticket-q->read)>=CM_IRQ_CAPACITY) {
  q->overflow=1; if(q->rejected!=UINT8_MAX) ++q->rejected; return EXEC_QUEUE_FULL;
 }
 slot=(uint8_t)(ticket&(CM_IRQ_CAPACITY-1));
 q->item[slot].priority=r->priority; q->item[slot].address.location=r->address.location;
 q->item[slot].address.bank=r->address.bank; q->item[slot].needs_vac=r->needs_vac;
 q->write=(uint8_t)(ticket+1u); return EXEC_OK;
}
static enum exec_status fault(cm_irq_runtime *r,enum exec_status status) {
 if(status!=EXEC_OK && status!=EXEC_IDLE && status!=EXEC_NOT_FOUND) {
  r->native->fault=status; r->native->running=0;
 }
 return status;
}
static enum exec_status service(cm_irq_runtime *r) {
 cm_exec_native *n=r->native; cm_irq_queue *q=r->queue; uint8_t delivered=0;
 if(n->fault!=EXEC_OK) return n->fault;
 if(q->overflow!=0) return fault(r,EXEC_QUEUE_FULL);
 if(n->state->alarm!=0) return fault(r,EXEC_BAILOUT_PENDING);
 while(q->read!=q->write && delivered<CM_IRQ_CAPACITY) {
  uint8_t ticket=q->read,slot=(uint8_t)(ticket&(CM_IRQ_CAPACITY-1));
  cm_irq_request req; enum exec_status status;
  req.priority=q->item[slot].priority; req.address.location=q->item[slot].address.location;
  req.address.bank=q->item[slot].address.bank; req.needs_vac=q->item[slot].needs_vac;
  if(!cm_exec_native_is_mapped(n,req.address,CM_EXEC_CONTEXT_BASIC)) return fault(r,EXEC_UNMAPPED);
  status=cm_exec_admit(n->state,req.priority,req.address.location,req.address.bank,req.needs_vac);
  if(status!=EXEC_OK) return fault(r,status); /* Failed head remains published. */
  q->read=(uint8_t)(ticket+1u); ++delivered;
 }
 return EXEC_OK;
}
static enum cm_exec_action callback(cm_executive *s,void *user,exec_address *resume) {
 cm_irq_runtime *r=user; cm_exec_native *n=r->native;
 cm_exec_continuation call=0; uint8_t i; enum cm_exec_action action;
 for(i=0;i<n->count;++i) {
  exec_address a=r->original_context!=0 ? r->original_context[i].address : r->original_basic[i].address;
  enum cm_exec_context_kind kind=r->original_context!=0 ? r->original_context[i].kind : CM_EXEC_CONTEXT_BASIC;
  if(a.location==n->current.location && a.bank==n->current.bank && kind==n->current_kind) {
   call=r->original_context!=0 ? r->original_context[i].call : r->original_basic[i].call; break;
  }
 }
 /* native_step verified this mapping before entry. Only the foreground adapter
  * changes table references, and binding happened with the source stopped. */
 r->exclusion->leave(r->exclusion->user,r->token); r->held=0;
 action=call(s,r->original_user,resume);
 r->token=r->exclusion->enter(r->exclusion->user); r->held=1;
 (void)service(r); /* Native epilogue observes the latched fault before exchange. */
 return action;
}
enum exec_status cm_irq_bind(cm_irq_runtime *r,cm_exec_native *n,cm_irq_queue *q,const exec_exclusion *x) {
 uint8_t i;
 if(r==0 || n==0 || q==0 || x==0 || x->enter==0 || x->leave==0 ||
    n->state==0 || n->fault!=EXEC_OK || n->running!=0 || n->in_callback!=0 ||
    n->count==0 || n->count>CM_IRQ_MAP_CAPACITY || (n->mapping==0 && n->context_mapping==0)) return EXEC_OUT_OF_SCOPE;
 for(i=0;i<n->count;++i) if((n->context_mapping!=0 ? n->context_mapping[i].call : n->mapping[i].call)==callback) return EXEC_OUT_OF_SCOPE;
 r->native=n; r->queue=q; r->exclusion=x; r->original_basic=n->mapping;
 r->original_context=n->context_mapping; r->original_user=n->user; r->held=0; r->token=0;
 for(i=0;i<n->count;++i) {
  if(n->context_mapping!=0) { r->proxy.context[i]=n->context_mapping[i]; r->proxy.context[i].call=callback; }
  else { r->proxy.basic[i]=n->mapping[i]; r->proxy.basic[i].call=callback; }
 }
 if(n->context_mapping!=0) n->context_mapping=r->proxy.context;
 else n->mapping=r->proxy.basic;
 n->user=r; return EXEC_OK;
}
static uint8_t lock(cm_irq_runtime *r) { return r->exclusion->enter(r->exclusion->user); }
static void unlock(cm_irq_runtime *r,uint8_t token) { r->exclusion->leave(r->exclusion->user,token); }
enum exec_status cm_irq_poll(cm_irq_runtime *r) {
 uint8_t token=lock(r); enum exec_status status=service(r); unlock(r,token); return status;
}
enum exec_status cm_irq_start(cm_irq_runtime *r) {
 uint8_t token; enum exec_status status;
 if(r->native->in_callback!=0) return EXEC_OUT_OF_SCOPE;
 token=lock(r); status=service(r);
 if(status==EXEC_OK) status=cm_exec_native_start(r->native);
 unlock(r,token); return status;
}
enum exec_status cm_irq_step(cm_irq_runtime *r) {
 enum exec_status status;
 if(r->native->in_callback!=0 || r->held!=0) return EXEC_OUT_OF_SCOPE;
 r->token=lock(r); r->held=1; status=service(r);
 if(status==EXEC_OK) status=cm_exec_native_step(r->native);
 unlock(r,r->token); r->held=0; return status;
}
enum exec_status cm_irq_admit(cm_irq_runtime *r,agc_word priority,exec_address a,int vac) {
 uint8_t token=lock(r); enum exec_status status=service(r);
 if(status==EXEC_OK) {
  if(!cm_exec_native_is_mapped(r->native,a,CM_EXEC_CONTEXT_BASIC)) status=fault(r,EXEC_UNMAPPED);
  else status=cm_exec_native_admit(r->native,priority,a,vac);
 }
 unlock(r,token); return status;
}
enum exec_status cm_irq_wake(cm_irq_runtime *r,agc_word cadr) {
 uint8_t token=lock(r); enum exec_status status=service(r);
 if(status==EXEC_OK) status=cm_exec_native_wake(r->native,cadr);
 unlock(r,token); return status;
}
