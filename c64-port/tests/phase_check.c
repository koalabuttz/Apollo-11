/* SPDX-License-Identifier: MIT */
#include "phase_check.h"
#include "phase_vectors.h"
typedef struct { uint8_t n; uint16_t field,failures; } checker;
static void compare(void *u,agc_word w) {
 checker *c=(checker *)u;
 if(c->field>=PH_WORD_COUNT || w!=phase_expected[c->n][c->field]) ++c->failures;
 ++c->field;
}
uint16_t phase_check(uint8_t n,phase_case *c) {
 checker check; if(n>=PH_TARGET_COUNT) return 1;
 check.n=n; check.field=0; check.failures=0;
 phase_prepare(c,phase_target(n));
 if(phase_change(&c->state,c->words,c->count,c->two)!=PHASE_OK) ++check.failures;
 phase_words(c,compare,&check);
 if(check.field!=PH_WORD_COUNT) ++check.failures;
 return check.failures;
}
