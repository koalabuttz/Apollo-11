/* SPDX-License-Identifier: MIT
 * Luminary099/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc lines934..947,1223..1228.
 * Scheduling arithmetic metadata follows original WAIT2/WTLST2; the list
 * mutation itself uses the independently verified native Waitlist service. */
#include "p66_startup.h"
static agc_register add(agc_register a,agc_word b) { return agc_add_register(a,agc_extend(b)); }
static agc_register inv(agc_register a) { return (agc_register)(a^0177777u); }
static enum agc_ccs_class kind(agc_register a) {
 int v=agc_overflow(a);
 return v>0 ? AGC_POSITIVE : v<0 ? AGC_NEGATIVE : agc_classify(agc_correct(a));
}
static agc_register dabs(agc_register a) {
 agc_register m=(a&0100000u)!=0 ? inv(a) : a;
 return m==0 ? 0 : (agc_register)(m-1u);
}
void p66_descbits(p66_operator_state *s) {
 agc_word delta=(s->a&0100u)!=0 ? 077776 : 1;
 s->a=add(agc_extend(delta),s->rodcount);
 s->rodcount=agc_correct(s->a);
}
enum exec_status p66_rodtask(executive *s,p66_task_io *io) {
 uint8_t slot,vac; agc_word old_l=s->newloc[1],callerfb,oldbank;
 enum exec_status result;
 if(io->bbank>AGC_MASK || (io->bbank&076000)!=064000 ||
    (io->bbank&01770)!=0 || (io->superbank&077717)!=0 || io->interrupt_enabled>1)
  return EXEC_OUT_OF_SCOPE;
 if(s->alarm!=0) return EXEC_BAILOUT_PENDING;
 for(vac=0;vac<EXEC_VAC_COUNT;++vac) if(agc_classify(s->vac[vac][0])==AGC_POSITIVE) break;
 for(slot=0;slot<EXEC_CORE_COUNT;++slot) if(s->core[slot][EXEC_PRIORITY]==AGC_NEG_ZERO) break;
 oldbank=slot<EXEC_CORE_COUNT ? s->core[slot][EXEC_BANK] : 0;
 callerfb=(agc_word)(io->bbank&076000);
 if(vac<EXEC_VAC_COUNT) old_l=s->vac[vac][0];
 result=exec_admit(s,022000,P66_RODCOMP_LOCATION,P66_RODCOMP_BBCON,1);
 if(result!=EXEC_OK && result!=EXEC_NO_VAC && result!=EXEC_NO_CORE) return result;
 io->interrupt_enabled=0; io->exectem1=callerfb;
 io->q=06743;
 if(result==EXEC_OK) {
  io->exectem2=(agc_word)(7u-slot); io->a=agc_extend(callerfb); io->l=agc_extend(oldbank);
 } else {
  if(result==EXEC_NO_CORE) io->exectem2=0;
  io->exectem1=old_l; io->a=03265; io->l=agc_extend(callerfb);
  io->q=result==EXEC_NO_VAC ? 02617 : 02703;
  io->bbank=(agc_word)(02000|(io->bbank&7));
 }
 return result;
}
enum wl_status p66_verta(phase_state *p,waitlist *w) {
 const agc_word phase=3;
 exec_address task,discarded;
 agc_register a,pre;
 enum agc_ccs_class k;
 uint8_t slot=0,inserted=1;
 enum wl_status status;
 if((p->bbank&076000)!=064000 || w->alarm!=0 || w->time3>037777)
  return WL_OUT_OF_SCOPE;
 /* Preflight metadata and phase constraints before the first source write. */
 if(p->bbank>AGC_MASK || (p->bbank&01770)!=0 || (p->superbank&077717)!=0 || p->interrupt_enabled>1)
  return WL_OUT_OF_SCOPE;
 task.location=P66_RODTASK_LOCATION; task.bank=(agc_word)(p->bbank|p->superbank);
 discarded=w->task[WL_TASK_COUNT-1];
 p->q=03271;
 if(phase_change(p,&phase,1,0)!=PHASE_OK) return WL_OUT_OF_SCOPE;
 p->interrupt_enabled=0;
 p->scratch[PH_TEMPG]=03273; p->scratch[PH_TEMPP]=task.bank;
 p->scratch[PH_TEMPNM]=task.location;
 a=add(agc_extend(agc_negate(w->time3)),0200);
 k=kind(a); a=dabs(a);
 if(k==AGC_POSITIVE) { a=add(a,040001); a=inv(a); }
 else if(k==AGC_PLUS_ZERO) a=inv(a);
 if(k!=AGC_MINUS_ZERO) a=add(a,040201);
 a=add(a,100); k=kind(a); a=dabs(a);
 if(k==AGC_POSITIVE) {
  a=add(a,w->interval[0]);
  for(slot=1;slot<WL_TASK_COUNT;++slot) {
   k=kind(a); a=dabs(a);
   if(k==AGC_NEGATIVE || k==AGC_MINUS_ZERO) break;
   if(slot==WL_INTERVAL_COUNT) break;
   if(k==AGC_POSITIVE) a=add(a,w->interval[slot]);
  }
  if(slot==WL_INTERVAL_COUNT && k==AGC_POSITIVE) inserted=0;
  if(inserted) {
   if(k==AGC_NEGATIVE || k==AGC_PLUS_ZERO) a=add(a,1);
   p->scratch[PH_TEMPBB]=agc_correct(a);
  }
 }
 pre=inserted ? add(agc_extend(discarded.location),agc_negate(WL_END_LOCATION)) : a;
 status=wl_schedule(w,100,task);
 if(status!=WL_OK && status!=WL_FULL) return status;
 p->scratch[PH_TEMPG]=agc_correct(pre);
 p->scratch[PH_TEMPP]=inserted ? discarded.bank : task.bank;
 if(status==WL_OK) {
  p->a=05220; p->l=02003; p->q=slot;
 } else {
  p->a=03273; p->l=agc_extend(p->bbank); p->q=03373; p->bbank=02003;
 }
 return status;
}
