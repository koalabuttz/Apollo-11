/* SPDX-License-Identifier: MIT */
#include "cm_enema.h"
static int zero(agc_register a) { return a==0 || a==0177777; }
enum cm_enema_result cm_enema_begin(cm_restart_state *r,cm_executive *e,cm_waitlist *w,const cm_restart_phases *p,cm_enema_io *io) {
 enum cm_rs_status phase; uint8_t fresh=0;
 if(io->main_keys>AGC_MASK || io->navigation_keys>AGC_MASK || io->interrupt_enabled>1 ||
    (io->bbank&076000)!=012000 || (io->bbank&001770)!=0 || io->bbank>AGC_MASK ||
    io->superbank>AGC_MASK || (io->superbank&077617)!=0 || w->time3>037777) return CM_ENEMA_OUT_OF_SCOPE;
 io->interrupt_enabled=0;
 if((io->navigation_keys&0100)!=0) {
  agc_register a=agc_add_register(agc_extend((agc_word)(io->navigation_keys&037)),agc_extend(077755));
  if(zero(a)) fresh=1;
  else if(zero(agc_add_register(agc_extend(io->main_keys),agc_extend(077755)))) fresh=1;
 }
 io->bbank=012003;
 if(fresh!=0) {
  cm_rs_startsub(r,e,w); io->q=3;io->a=agc_extend(077754);
  return CM_ENEMA_FRESH_START;
 }
 cm_rs_cleanup(r,e,w);io->q=3;
 r->word[CM_RS_RASFLAG]&=057677;
 if((r->word[CM_RS_FLAGWRD6]&060000)==040000) {
  exec_address tvc={02720,034066};uint8_t slot;
  if(cm_wl_schedule(w,50,tvc)!=CM_WL_OK) return CM_ENEMA_OUT_OF_SCOPE;
  e->newprio=tvc.location;
  for(slot=0;slot<9;++slot) if(w->task[slot].location==tvc.location && w->task[slot].bank==tvc.bank) break;
  if(slot>=8) return CM_ENEMA_OUT_OF_SCOPE;
  if(slot!=0) e->locctr=agc_negate(w->interval[slot]);
  io->q=slot;io->waitexit=AGC_NEG_ZERO;io->waitbank=CM_WL_END_BANK;
 }
 phase=cm_rs_validate(e,p);
 if(phase==CM_RS_PHASE_BAD) {
  uint8_t group=(uint8_t)e->core[0][5];agc_word x=(agc_word)(p->complement[group]^p->phase[group]);
  io->a=agc_diminished_abs(x);io->l=agc_extend(p->phase[group]);return CM_ENEMA_BAD_PHASE;
 }
 io->a=0;io->l=agc_extend(p->phase[0]);io->q=02707;return CM_ENEMA_DISPLAY;
}
