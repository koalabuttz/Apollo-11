/* SPDX-License-Identifier: MIT */
#include "recovery.h"
uint8_t recovery_abort(recovery_abort_state *s,enum recovery_abort_entry entry) {
 uint8_t i;
 if(entry<RECOVERY_FLAGS || entry>RECOVERY_POODOO_PHASE4) return 1;
 if(entry!=RECOVERY_POODOO_PHASE4 && (s->q<0400 || s->q>07777)) return 1;
 if(entry==RECOVERY_FLAGS) {
  s->flagword3&=077757; s->flagword10&=077677; s->flagword2&=077776;
  s->a=agc_extend(s->flagword2);
 } else if(entry==RECOVERY_POODOO_PHASE4) {
  s->a=agc_extend(s->records.complement[3]); s->l=agc_extend(s->records.phase[3]);
  s->records.complement[3]=(agc_word)(AGC_MASK^035); s->records.phase[3]=035;
 } else {
  if(entry==RECOVERY_MR_KLEAN) { s->interrupt_enabled=0; s->records.complement[1]=AGC_NEG_ZERO; s->records.phase[1]=0; }
  if(entry!=RECOVERY_V37KLEAN) { s->records.complement[3]=AGC_NEG_ZERO; s->records.phase[3]=0; }
  /* Original order 1,3,5,6: final DXCH leaves the OLD sixth pair in A/L. */
  s->a=agc_extend(s->records.complement[5]); s->l=agc_extend(s->records.phase[5]);
  for(i=0;i<6;++i) if(i==0 || i==2 || i==4 || i==5) {
   s->records.complement[i]=AGC_NEG_ZERO; s->records.phase[i]=0;
  }
 }
 /* TC Q executes the Q register as a TC return instruction: Q then
  * contains the next register address, 3, at the caller destination. */
 if(entry!=RECOVERY_POODOO_PHASE4) s->q=3;
 return 0;
}
