/* SPDX-License-Identifier: MIT */
#include "enema.h"
enum enema_result enema_begin(restart_state *r,executive *e,waitlist *w,
                             const restart_phases *p,enema_io *io) {
 agc_word old_t5_bank,xorword; enum rs_status phase;
 if(io->main_keys>AGC_MASK || io->navigation_keys>AGC_MASK ||
    io->flagword10>AGC_MASK || (io->bbank&076000)!=012000 ||
    (io->bbank&001770)!=0 || io->bbank>AGC_MASK ||
    io->superbank>AGC_MASK || (io->superbank&077617)!=0)
  return ENEMA_OUT_OF_SCOPE;
 old_t5_bank=r->word[RS_T5ADR1];
 rs_cleanup(r,e,w);
 io->bbank=012003; io->interrupt_enabled=0;
 io->q=3; io->l=agc_extend(old_t5_bank);
 if((io->navigation_keys&020)!=0) {
  io->a=agc_add_register(agc_extend(io->main_keys),agc_extend(077755));
  if(io->a==0 || io->a==0177777) {
   io->q=03104; return ENEMA_FRESH_START;
  }
 }
 io->flagword10&=057677;
 phase=rs_validate(e,p);
 if(phase==RS_PHASE_BAD) {
  uint8_t group=(uint8_t)e->core[0][5];
  xorword=(agc_word)(p->complement[group]^p->phase[group]);
  io->a=agc_diminished_abs(xorword); io->l=agc_extend(p->phase[group]);
  return ENEMA_BAD_PHASE;
 }
 io->a=0; io->l=agc_extend(p->phase[0]); io->q=03015;
 return ENEMA_DISPLAY;
}
