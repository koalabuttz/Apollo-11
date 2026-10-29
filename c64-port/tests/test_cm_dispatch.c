/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cm_dispatch_check.h"
#include "cm_native_dispatch.h"
static enum cm_exec_action finish(cm_executive *s,void *u,exec_address *a) {
 (void)s;(void)u;(void)a; return CM_EXEC_FINISH;
}
static enum cm_exec_action yield(cm_executive *s,void *u,exec_address *a) {
 (void)s;(void)u; a->location=02212; a->bank=06000; return CM_EXEC_YIELD;
}
int main(void) {
 cm_executive state,before; cm_exec_native runtime;
 const cm_exec_mapping maps[]={{{02000,06000},finish}};
 exec_address next={01234,04567},saved=next;
 assert(cm_dispatch_run_checks()==0);
 cm_exec_init(&state); state.alarm=01201; before=state;
 assert(cm_exec_native_init(&runtime,&state,maps,1,0)==EXEC_OK);
 assert(cm_exec_native_start(&runtime)==EXEC_BAILOUT_PENDING);
 assert(memcmp(&state,&before,sizeof state)==0);
 cm_exec_init(&state); state.newjob=84; before=state;
 assert(cm_exec_change_basic(&state,(exec_address){02000,06000},&next)==EXEC_OUT_OF_SCOPE);
 assert(memcmp(&state,&before,sizeof state)==0 && memcmp(&next,&saved,sizeof next)==0);
 /* A missing incoming mapping must be rejected before LOC/exchange writes. */
 {
  const cm_exec_context_mapping mixed[]={
   {{02210,06000},CM_EXEC_CONTEXT_INTERPRETIVE,yield},
   {{02212,06000},CM_EXEC_CONTEXT_INTERPRETIVE,yield}};
  cm_exec_init(&state); state.core[0][CM_EXEC_PRIORITY]=02401;
  state.core[0][CM_EXEC_LOC]=02210; state.core[0][CM_EXEC_BANK]=06000;
  state.core[0][CM_EXEC_PUSH]=0401; state.fixloc=0401;
  state.core[1][CM_EXEC_PRIORITY]=03455; state.core[1][CM_EXEC_LOC]=03000;
  state.core[1][CM_EXEC_BANK]=010104; state.core[1][CM_EXEC_PUSH]=0455;
  state.newjob=12; before=state;
  assert(cm_exec_native_context_init(&runtime,&state,mixed,2,0)==EXEC_OK);
  runtime.current=(exec_address){02210,06000}; runtime.current_kind=CM_EXEC_CONTEXT_INTERPRETIVE; runtime.running=1;
  assert(cm_exec_native_step(&runtime)==EXEC_UNMAPPED);
  assert(memcmp(&state,&before,sizeof state)==0);
  assert(cm_exec_native_step(&runtime)==EXEC_UNMAPPED);
  assert(memcmp(&state,&before,sizeof state)==0);
 }
 puts("PASS: 876 CM full-state fingerprints, 16 complete samples, 20 actual callback sequences, invalid/pending boundaries");
 return 0;
}
