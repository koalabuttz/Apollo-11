/* SPDX-License-Identifier: MIT */
#include "cm_dispatch_check.h"
#include "cm_dispatch_cases.h"
#include "cm_lifecycle_cases.h"
#include "cm_interpreter_cases.h"
#include "cm_mixed_cases.h"
#include "cm_dispatch_vectors.h"
static uint32_t add(uint32_t h,uint16_t word) {
 h=(h^(uint8_t)word)*UINT32_C(16777619);
 return (h^(uint8_t)(word>>8))*UINT32_C(16777619);
}
static int check(unsigned index,const cm_executive *s,enum exec_status status,
 exec_address next,unsigned format,enum cm_exec_context_kind kind,exec_interpreter control) {
 uint16_t words[318]; unsigned k=0,i,j,t; uint32_t h=UINT32_C(2166136261);
 if(index-CM_HASH_FIRST>=CM_HASH_COUNT) return 0;
 words[k++]=(uint16_t)status;
 for(i=0;i<7;++i) for(j=0;j<12;++j) words[k++]=s->core[i][j];
 for(i=0;i<5;++i) for(j=0;j<44;++j) words[k++]=s->vac[i][j];
 words[k++]=s->newjob; words[k++]=s->newprio;
 words[k++]=s->newloc[0]; words[k++]=s->newloc[1]; words[k++]=s->locctr;
 words[k++]=s->fixloc; words[k++]=s->ovfind; words[k++]=s->alarm;
 if(format==1) { words[k++]=control.intbit15; words[k++]=control.edop; }
 words[k++]=next.location; words[k++]=next.bank;
 if(format!=0) words[k++]=(uint16_t)kind;
 if(format==2) { words[k++]=control.intbit15; words[k++]=control.edop; }
 if(k!=(format==0?315u:318u)) return 0;
 for(i=0;i<k;++i) h=add(h,words[i]);
 if(h!=cm_dispatch_hashes[index-CM_HASH_FIRST]) return 0;
 for(t=0;t<CM_FULL_COUNT;++t) if(cm_dispatch_full_indices[t]==index) {
  if(k!=cm_dispatch_full_lengths[t]) return 0;
  for(i=0;i<k;++i) if(words[i]!=cm_dispatch_full_words[t][i]) return 0;
 }
 return 1;
}
#if defined(CM_CHECK_BASIC) || defined(CM_CHECK_LIFECYCLE) || defined(CM_CHECK_ALL)
typedef struct { unsigned failures,count,number,lifecycle; } observation;
static void observe(void *user,uint8_t step,const cm_executive *s,enum exec_status status,exec_address next) {
 observation *o=user; exec_interpreter control={0,0}; unsigned index;
 index=o->lifecycle?109u+cm_lifecycle_vector_index((uint8_t)o->number,step):cm_dispatch_vector_index((uint8_t)o->number,step);
 if(!check(index,s,status,next,0,CM_EXEC_CONTEXT_BASIC,control)) ++o->failures;
 ++o->count;
}
#endif
unsigned cm_dispatch_run_checks(void) {
 unsigned failures=0,count=0,n;
#if defined(CM_CHECK_BASIC) || defined(CM_CHECK_LIFECYCLE) || defined(CM_CHECK_ALL)
 observation o={0,0,0,0}; cm_executive state;
#endif
#if defined(CM_CHECK_BASIC) || defined(CM_CHECK_ALL)
 unsigned step;
 for(n=0;n<DISPATCH_CASE_COUNT;++n) {
  o.number=n;
  if(n>=DISPATCH_SEQUENCE_FIRST && n<50) {
   if(!cm_dispatch_demo(&state,(uint8_t)n,observe,&o)) ++o.failures;
  } else {
   cm_dispatch_prepare(&state,(uint8_t)n);
   for(step=0;step<cm_dispatch_steps((uint8_t)n);++step) {
    cm_dispatch_event e; exec_address next={0,0}; enum exec_status status;
    cm_dispatch_event_prepare(&e,(uint8_t)n,(uint8_t)step);
    status=cm_dispatch_apply(&state,&e,&next); observe(&o,(uint8_t)step,&state,status,next);
   }
  }
 }
#endif
#if defined(CM_CHECK_LIFECYCLE) || defined(CM_CHECK_ALL)
 o.lifecycle=1;
 for(n=0;n<LIFECYCLE_CASE_COUNT;++n) {
  o.number=n;
  if(n>=76) {
   if(!cm_lifecycle_demo(&state,(uint8_t)n,observe,&o)) ++o.failures;
  } else {
   cm_lifecycle_event e; exec_address next={0,0}; enum exec_status status;
   cm_lifecycle_prepare(&state,(uint8_t)n); cm_lifecycle_event_prepare(&e,(uint8_t)n,0);
   status=cm_lifecycle_apply(&state,&e,&next); observe(&o,0,&state,status,next);
  }
 }
#endif
#if defined(CM_CHECK_BASIC) || defined(CM_CHECK_LIFECYCLE) || defined(CM_CHECK_ALL)
 failures+=o.failures; count+=o.count;
#endif
#if defined(CM_CHECK_INTERPRETER) || defined(CM_CHECK_ALL)
 for(n=0;n<CM_INTERPRETER_COUNT;++n) {
  cm_interpreter_case c; exec_transfer next={{0,0},0}; enum exec_status status;
  cm_interpreter_prepare(&c,n); status=cm_exec_change_interpretive(&c.state,c.superbank,&c.control,&next);
  if(!check(224u+n,&c.state,status,next.address,1,next.interpretive?CM_EXEC_CONTEXT_INTERPRETIVE:CM_EXEC_CONTEXT_BASIC,c.control)) ++failures;
  ++count;
 }
#endif
#if defined(CM_CHECK_MIXED) || defined(CM_CHECK_ALL)
 for(n=0;n<IN_CASE_COUNT;++n) {
  unsigned step; cm_mixed_run r; cm_mixed_run_init(&r,n);
  for(step=0;step<cm_mixed_steps(n);++step) {
   enum exec_status status=cm_mixed_run_step(&r,step);
   unsigned index=736u+(n<7u?n*IN_STEPS+step:112u+n-7u);
   if(!check(index,&r.state,status,r.runtime.current,2,r.runtime.current_kind,r.runtime.interpreter_control)) ++failures;
   ++count;
  }
  failures+=r.failures;
 }
#endif
 if(count!=CM_CHECK_COUNT) ++failures;
 return failures;
}
