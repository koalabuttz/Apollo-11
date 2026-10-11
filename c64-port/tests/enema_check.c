/* SPDX-License-Identifier: MIT */
#include "enema_check.h"
#include "enema_vectors.h"
typedef struct { uint16_t field,patch,end,failures; uint8_t baseline; } checker;
static void compare(void *u,agc_word w) {
 checker *c=(checker *)u; agc_word expected;
 if(c->field>=ENEMA_WORD_COUNT) { ++c->failures; return; }
 expected=enema_base[c->baseline][c->field];
 if(c->patch<c->end && enema_patch[c->patch][0]==c->field) expected=enema_patch[c->patch++][1];
 if(w!=expected) ++c->failures;
 ++c->field;
}
uint16_t enema_check(uint8_t n,enema_case *c) {
 checker check; enum enema_result result;
 if(n>=ENEMA_TARGET_COUNT) return 1;
 enema_prepare(c,enema_target(n));
 check.field=0; check.patch=enema_patch_offset[n]; check.end=enema_patch_offset[(uint16_t)n+1];
 check.failures=0; check.baseline=(uint8_t)(enema_target(n)%16);
 result=enema_begin(&c->previous.record,&c->previous.exec,&c->previous.wait,&c->previous.phases,&c->io);
 if((agc_word)result!=enema_result[n]) ++check.failures;
 enema_words(c,compare,&check);
 if(check.field!=ENEMA_WORD_COUNT || check.patch!=check.end) ++check.failures;
 return check.failures;
}
