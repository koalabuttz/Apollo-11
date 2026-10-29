/* SPDX-License-Identifier: MIT */
#include "waitlist_zero_check.h"
#include "waitlist_zero_vectors.h"
typedef struct { uint16_t n,field,failures; } checker;
static void compare(void *u,agc_word w) {
 checker *c=(checker *)u;
 if(c->field>=WL_ZERO_WORD_COUNT || w!=wl_zero_expected[c->n][c->field+1u]) ++c->failures;
 ++c->field;
}
uint16_t wl_zero_check(uint16_t n,waitlist *w) {
 checker c; exec_address task;
 if(n>=WL_ZERO_CASE_COUNT) return 1;
 c.n=n;c.field=0;c.failures=0;
 wl_zero_prepare(w,n,&task);
 if((agc_word)wl_schedule(w,20,task)!=wl_zero_expected[n][0]) ++c.failures;
 wl_zero_words(w,compare,&c);
 if(c.field!=WL_ZERO_WORD_COUNT) ++c.failures;
 return c.failures;
}
