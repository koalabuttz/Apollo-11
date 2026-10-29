/* SPDX-License-Identifier: MIT */
#include "p66_startup_check.h"
#include "p66_startup_vectors.h"
typedef struct {uint16_t field,count,base,patch,end,failures;} checker;
static void compare(void *u,agc_word w) {
 checker *c=(checker *)u; agc_word v;
 if(c->field>=c->count) {++c->failures;return;}
 v=p66_startup_base[c->base+c->field];
 if(c->patch<c->end && p66_startup_patch[c->patch][0]==c->field) v=p66_startup_patch[c->patch++][1];
 if(v!=w) ++c->failures;
 ++c->field;
}
uint16_t p66_startup_check(uint8_t n,p66_startup_case *s) {
 checker c; uint32_t target;
 if(n>=P66_STARTUP_TARGET_COUNT)return 1;
 target=p66_startup_target(n);p66_startup_prepare(s,target);
 c.count=s->mode==0 ? 2 : s->mode==1 ? 332 : 85;
 c.base=(uint16_t)((s->mode==0 ? 0 : s->mode==1 ? 16 : 2672)+(target%8u)*c.count);
 c.field=0;c.patch=p66_startup_offsets[n];c.end=p66_startup_offsets[n+1u];c.failures=0;
 if(p66_startup_apply(s)!=p66_startup_result[n])++c.failures;
 p66_startup_words(s,compare,&c);
 if(c.field!=c.count || c.patch!=c.end)++c.failures;
 return c.failures;
}
