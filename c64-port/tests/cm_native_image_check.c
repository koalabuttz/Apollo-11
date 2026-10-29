/* SPDX-License-Identifier: MIT */
#include "cm_native_image_check.h"
#include "cm_restart_native_vectors.h"
#include <string.h>
typedef struct { uint32_t hash;uint16_t field;unsigned errors;const uint16_t *full; } image_check;
static void word(void *u,agc_word w) {
 image_check *c=u;c->hash=(c->hash^(uint8_t)w)*UINT32_C(16777619);
 c->hash=(c->hash^(uint8_t)(w>>8))*UINT32_C(16777619);
 if(c->full && c->full[c->field]!=w) ++c->errors;
 ++c->field;
}
static void commit(apollo_overlay_state *s,cm_native_io *io,const cm_restart_case *c) {
 cm_native_commit_executive(s,io,&c->exec);cm_native_commit_waitlist(s,io,&c->wait);cm_native_commit_restart(s,io,&c->record);
}
static void gather(const apollo_overlay_state *s,const cm_native_io *io,cm_restart_case *c) {
 cm_phase_state p;memset(c,0xcc,sizeof *c);
 cm_native_gather_executive(s,io,&c->exec);cm_native_gather_waitlist(s,io,&c->wait);cm_native_gather_restart(s,io,&c->record);
 cm_native_gather_phase(s,io,&p);c->phases=p.records;
}
unsigned cm_image_check(apollo_overlay_state *s,cm_native_io *io,cm_rn_run *r) {
 uint8_t n,step,j;uint16_t row=0,i;unsigned errors=0;
 for(n=0;n<64;++n) {
  cm_phase_state p;cm_enema_io registers;cm_rn_init(r,n);memset(io,0,sizeof *io);
  for(i=0;i<2048;++i) s->words[i]=(agc_word)((i*17u+012345u)&AGC_MASK);
  /* Preserve raw A/L register representations while gathering typed views. */
  s->words[0]=0177777;s->words[1]=0100000;s->words[6]=02003;
  cm_native_gather_phase(s,io,&p);p.records=r->state.phases;cm_native_commit_phase(s,io,&p);
  commit(s,io,&r->state);
  cm_native_gather_enema_io(s,io,&registers);
  if(registers.a!=0177777 || registers.l!=0100000) ++errors;
  cm_native_commit_enema_io(s,io,&registers);
  for(step=0;step<14;++step) {
   image_check k={UINT32_C(2166136261),0,0,0};enum exec_status status;
   gather(s,io,&r->state);status=cm_rn_step(r,step);commit(s,io,&r->state);
   /* Poison then regather. Every compared source word must come back from
    * the sole canonical image or explicitly owned external device metadata. */
   gather(s,io,&r->state);
#ifndef CM_IMAGE_NO_FULL
   for(j=0;j<8;++j) if(cm_rn_indices[j]==row) k.full=cm_rn_full[j];
#else
   (void)j;
#endif
   cm_rn_words(&r->state,status,r->runtime.current,r->callbacks,word,&k);
   if(row>=320 || k.field!=434 || k.hash!=cm_rn_hashes[row]) ++k.errors;
   errors+=k.errors;++row;
   if(s->words[0]!=0177777 || s->words[1]!=0100000 || s->words[3]!=01400 || s->words[4]!=02000) ++errors;
   for(i=0300;i<0320;++i) if(s->words[i]!=((i*17u+012345u)&AGC_MASK)) ++errors;
   if(r->active || r->runtime.in_callback || r->failures) ++errors;
   if(status==EXEC_IDLE) break;
   if(status!=EXEC_OK) { ++errors;break; }
  }
 }
 if(row!=320) ++errors;
 return errors;
}
