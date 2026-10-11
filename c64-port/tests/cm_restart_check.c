/* SPDX-License-Identifier: MIT */
#include "cm_restart_check.h"
#include "cm_restart_vectors.h"
typedef struct { uint32_t hash;unsigned errors;uint16_t field;const uint16_t *full; } check;
static void word(void *user,agc_word w) {
 check *c=user;c->hash=(c->hash^(uint8_t)w)*UINT32_C(16777619);c->hash=(c->hash^(uint8_t)(w>>8))*UINT32_C(16777619);
 if(c->full!=0 && c->full[c->field]!=w) ++c->errors;
 ++c->field;
}
unsigned cm_rs_check(cm_restart_case *c) {
 uint16_t n;unsigned errors=0;uint8_t j;
 for(n=0;n<1280;++n) {
  check k={UINT32_C(2166136261),0,0,0};int status;
  for(j=0;j<18;++j) if(cm_rs_indices[j]==n) k.full=cm_rs_full[j];
  cm_rs_prepare(c,n);status=cm_rs_apply(c,n);word(&k,(agc_word)status);cm_rs_words(c,word,&k);
  if(k.field!=430 || k.hash!=cm_rs_hashes[n]) ++k.errors;
  errors+=k.errors;
 }return errors;
}
