/* SPDX-License-Identifier: MIT */
#include "cm_phase_check.h"
#include "cm_phase_vectors.h"
typedef struct { uint8_t n; uint16_t field,failures; } checker;
static void compare(void *u,agc_word w) {
 checker *c=(checker *)u;
 if(c->field>=CM_PH_WORD_COUNT || w!=cm_phase_expected[c->n][c->field]) ++c->failures;
 ++c->field;
}
uint16_t cm_phase_check(uint8_t n,cm_phase_case *c) {
 checker check; if(n>=CM_PH_TARGET_COUNT) return 1;
 check.n=n; check.field=0; check.failures=0;
 cm_phase_prepare(c,cm_phase_target(n));
 if(cm_phase_change(&c->state,c->words,c->count,c->two)!=CM_PHASE_OK) ++check.failures;
 cm_phase_words(c,compare,&check);
 if(check.field!=CM_PH_WORD_COUNT) ++check.failures;
 return check.failures;
}
