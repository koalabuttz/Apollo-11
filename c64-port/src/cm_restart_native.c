/* SPDX-License-Identifier: MIT */
#include "cm_restart_native.h"
enum exec_status cm_restart_native_stage(cm_exec_native *r,cm_restart_state *s,
 cm_waitlist *w,const cm_restart_phases *p,uint8_t initialize_counters) {
 uint8_t i; enum exec_status status;
 if(r==0 || s==0 || w==0 || p==0 || r->state==0 || r->running!=0 ||
    r->in_callback!=0 || initialize_counters>1) return EXEC_OUT_OF_SCOPE;
 if(r->fault!=EXEC_OK) return r->fault;
 for(i=0;i<6;++i) {
  agc_word phase=p->phase[i],priority=p->priority[i];
  agc_word corrected=(priority&AGC_SIGN)!=0 ? agc_negate(priority) : priority;
  if(phase==0) continue;
  if(phase>01777 || (phase&01407)!=00402 || corrected==0 ||
     (corrected&0140777)!=0 || p->address[i].bank>AGC_MASK)
   return EXEC_OUT_OF_SCOPE;
  if(!cm_exec_native_is_mapped(r,p->address[i],CM_EXEC_CONTEXT_BASIC)) return EXEC_UNMAPPED;
 }
 if(initialize_counters) cm_rs_startsub(s,r->state,w);
 else cm_rs_cleanup(s,r->state,w);
 if(cm_rs_validate(r->state,p)!=CM_RS_OK) return EXEC_OUT_OF_SCOPE;
 i=6;
 while(i!=0) {
  --i;
  if(p->phase[i]==0) continue;
  status=cm_rs_job(r->state,p,i);
  if(status!=EXEC_OK) return status;
 }
 return EXEC_OK;
}
enum exec_status cm_restart_native_start(cm_exec_native *r,cm_restart_state *s) {
 agc_word pending;enum exec_status status;
 if(r==0 || s==0 || r->state==0 || s->channel11>AGC_MASK) return EXEC_OUT_OF_SCOPE;
 pending=r->state->newjob;status=cm_exec_native_start(r);
 if(status==EXEC_OK) {
  s->channel11|=2;
  if(pending==0) r->state->core[0][CM_EXEC_LOC]=s->channel11;
 }
 return status;
}
