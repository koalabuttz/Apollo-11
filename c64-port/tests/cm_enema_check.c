/* SPDX-License-Identifier: MIT */
#include "cm_enema_check.h"
#include "cm_enema_vectors.h"
typedef struct { uint32_t hash;unsigned errors;uint16_t field;const uint16_t *full; } check;
static void word(void *user,agc_word w) {
 check *c=user;c->hash=(c->hash^(uint8_t)w)*UINT32_C(16777619);c->hash=(c->hash^(uint8_t)(w>>8))*UINT32_C(16777619);
 if(c->full!=0 && c->full[c->field]!=w) ++c->errors;
 ++c->field;
}
unsigned cm_enema_check(cm_enema_case *c) {
 uint16_t n;unsigned errors=0;uint8_t j;
 for(n=0;n<4096;++n) {
  check k={UINT32_C(2166136261),0,0,0};int status;
  for(j=0;j<8;++j) if(cm_enema_indices[j]==n) k.full=cm_enema_full[j];
  cm_enema_prepare(c,n);status=cm_enema_begin(&c->previous.record,&c->previous.exec,&c->previous.wait,&c->previous.phases,&c->io);word(&k,(agc_word)status);cm_enema_words(c,word,&k);
  if(k.field!=440 || k.hash!=cm_enema_hashes[n]) ++k.errors;
  errors+=k.errors;
 }return errors;
}
