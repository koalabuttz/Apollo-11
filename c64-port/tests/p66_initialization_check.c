/* SPDX-License-Identifier: MIT */
#include "p66_initialization_check.h"
#include "p66_initialization_vectors.h"
typedef struct {uint16_t field,failures;uint8_t n;} checker;
static void compare(void *u,agc_word w){checker *c=(checker *)u;if(c->field>=P66_INIT_WORD_COUNT || w!=p66_init_expected[c->n][c->field])++c->failures;++c->field;}
uint16_t p66_init_check(uint8_t n,p66_init_case *s){checker c;enum p66_init_status result;if(n>=P66_INIT_TARGET_COUNT)return 1;c.field=0;c.failures=0;c.n=n;p66_init_prepare(s,(uint16_t)((n/2u)*14u+n%2u));result=s->fast ? p66_fastchng(&s->state) : p66_stoprate(&s->state);if(result!=P66_INIT_OK)++c.failures;p66_init_words(s,compare,&c);if(c.field!=P66_INIT_WORD_COUNT)++c.failures;return c.failures;}
