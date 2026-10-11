/* SPDX-License-Identifier: MIT */
#include "cm_restart_native_check.h"
#include "cm_restart_native_vectors.h"
typedef struct { uint32_t hash;unsigned errors;uint16_t field;const uint16_t *full; } check;
static void word(void *u,agc_word w) {
 check *c=u;c->hash=(c->hash^(uint8_t)w)*UINT32_C(16777619);
 c->hash=(c->hash^(uint8_t)(w>>8))*UINT32_C(16777619);
 if(c->full && c->full[c->field]!=w) ++c->errors;
 ++c->field;
}
unsigned cm_rn_check(cm_rn_run *r) {
 uint8_t n,step,j;uint16_t row=0;unsigned errors=0;
 for(n=0;n<64;++n) {
  cm_rn_init(r,n);
  for(step=0;step<14;++step) {
   check c={UINT32_C(2166136261),0,0,0};enum exec_status status=cm_rn_step(r,step);
   for(j=0;j<8;++j) if(cm_rn_indices[j]==row) c.full=cm_rn_full[j];
   cm_rn_words(&r->state,status,r->runtime.current,r->callbacks,word,&c);
   if(row>=320 || c.field!=434 || c.hash!=cm_rn_hashes[row]) ++c.errors;
   errors+=c.errors;++row;
   if(r->failures || r->active || r->runtime.in_callback) ++errors;
   if(status==EXEC_IDLE) break;
   if(status!=EXEC_OK) { ++errors;break; }
  }
 }
 if(row!=320) ++errors;
 return errors;
}
