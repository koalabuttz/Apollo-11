/* SPDX-License-Identifier: MIT */
#include "recovery_check.h"
#include "recovery_vectors.h"
typedef struct { uint8_t n; uint16_t field,failures; } checker;
static void compare(void *u,agc_word w) {
 checker *c=(checker *)u;
 if(c->field>=RECOVERY_WORD_COUNT || w!=recovery_expected[c->n][c->field]) ++c->failures;
 ++c->field;
}
uint16_t recovery_check(uint8_t n,recovery_case *c) {
 checker check; if(n>=RECOVERY_TARGET_COUNT) return 1;
 check.n=n; check.field=0; check.failures=0;
 recovery_prepare(c,recovery_target(n));
 if(recovery_abort(&c->state,c->entry)!=0) ++check.failures;
 recovery_words(c,compare,&check);
 if(check.field!=RECOVERY_WORD_COUNT) ++check.failures;
 return check.failures;
}
