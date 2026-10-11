/* SPDX-License-Identifier: MIT */
#include "cm_startup_check.h"
#include "cm_startup_vectors.h"
typedef struct { uint32_t hash;unsigned errors;uint16_t field;const uint16_t *full; } check;
static void word(void *u,agc_word w) {
 check *c=u;c->hash=(c->hash^(uint8_t)w)*UINT32_C(16777619);
 c->hash=(c->hash^(uint8_t)(w>>8))*UINT32_C(16777619);
 if(c->full && c->full[c->field]!=w) ++c->errors;
 ++c->field;
}
unsigned cm_startup_check(apollo_overlay_state *s,cm_native_io *io,cm_startup_ports *p) {
 uint16_t n;uint8_t j;unsigned errors=0;
 for(n=0;n<640;++n) {
  check k={UINT32_C(2166136261),0,0,0};enum cm_startup_status status;
  for(j=0;j<5;++j) if(cm_startup_indices[j]==n) k.full=cm_startup_full[j];
  cm_startup_prepare(s,io,p,n);status=cm_startup_begin(s,io,p,(enum cm_startup_entry)(n/128u));
  cm_startup_words(s,io,p,status,word,&k);
  if(k.field!=2063 || k.hash!=cm_startup_hashes[n]) ++k.errors;
  errors+=k.errors;
 }
 return errors;
}
