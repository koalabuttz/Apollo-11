/* SPDX-License-Identifier: MIT */
#include "cm_restart_native_cases.h"
exec_address cm_rn_entry(uint8_t i) {
 exec_address a={(agc_word)(01150u+i*4u),02003};return a;
}
void cm_rn_prepare(cm_restart_case *c,uint8_t n) {
 uint8_t i;cm_rs_prepare(c,n);
 for(i=0;i<6;++i) {
  c->phases.phase[i]=(n&(1u<<i))!=0 ? 00402 : 0;
  c->phases.complement[i]=(agc_word)(c->phases.phase[i]^AGC_MASK);
  c->phases.address[i]=cm_rn_entry(i);
  c->phases.priority[i]=(agc_word)((1u+((n+i*3u)%7u))*01000u);
  if(((n+i)&3u)!=0 || i==0) c->phases.priority[i]=(agc_word)(c->phases.priority[i]^AGC_MASK);
 }
}
void cm_rn_words(const cm_restart_case *c,enum exec_status status,exec_address a,
 uint8_t callbacks,void (*emit)(void *,agc_word),void *u) {
 emit(u,(agc_word)status);cm_rs_words(c,emit,u);emit(u,a.location);emit(u,a.bank);
 emit(u,callbacks);emit(u,0);
}
#ifndef CM_RESTART_NATIVE_INPUT_ONLY
static enum cm_exec_action callback(cm_executive *e,void *u,exec_address *next) {
 cm_rn_run *r=u;volatile uint32_t guard=UINT32_C(0x71d2048b);(void)next;
 if(r->active || r->runtime.in_callback!=1) ++r->failures;
 r->active=1;e->core[0][2]=agc_add_word(e->core[0][2],1);++r->callbacks;
 if(guard!=UINT32_C(0x71d2048b)) ++r->failures;
 r->active=0;return CM_EXEC_FINISH;
}
static const cm_exec_mapping map[]={
 {{01150,02003},callback},{{01154,02003},callback},{{01160,02003},callback},
 {{01164,02003},callback},{{01170,02003},callback},{{01174,02003},callback}};
void cm_rn_init(cm_rn_run *r,uint8_t n) {
 cm_rn_prepare(&r->state,n);r->callbacks=r->active=r->failures=0;
 if(cm_exec_native_init(&r->runtime,&r->state.exec,map,6,r)!=EXEC_OK) ++r->failures;
}
enum exec_status cm_rn_step(cm_rn_run *r,uint8_t step) {
 if(step==0) return cm_restart_native_stage(&r->runtime,&r->state.record,
  &r->state.wait,&r->state.phases,0);
 if(step==1) return cm_restart_native_start(&r->runtime,&r->state.record);
 return cm_exec_native_step(&r->runtime);
}
#endif
