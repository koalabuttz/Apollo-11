/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "projection_cases.h"
unsigned pj_check(projection_case *);
static void reject(projection_case *c,uint8_t phase){uint16_t before[PJ_WORDS],after[PJ_WORDS];pj_words(c,before);assert(pj_apply(c,phase)==P66_PROJECTION_OUT_OF_SCOPE);pj_words(c,after);assert(!memcmp(before,after,sizeof before));}
int main(void){projection_case c,original;uint16_t value;uint8_t phase;assert(!pj_check(&c));pj_input(&original,0);for(value=0;value<=AGC_MASK;++value)for(phase=0;phase<3u;++phase){c=original;c.previous.previous.base.previous.previous.previous.preparation.mpac[7]=value;c.previous.previous.base.previous.previous.push.pushloc=(agc_word)(c.previous.previous.base.previous.previous.push.base+(phase?2u:0u));if(value)reject(&c,phase);c=original;c.previous.previous.base.previous.previous.push.base=value;c.previous.previous.base.previous.previous.push.pushloc=(agc_word)(value+(phase?2u:0u));if(value<0401u||value>0661u||(uint16_t)(value-0401u)%054u)reject(&c,phase);c=original;c.previous.previous.base.previous.previous.push.pushloc=value;if(value!=c.previous.previous.base.previous.previous.push.base+(phase?2u:0u))reject(&c,phase);}
puts("PASS144 complete original phased projection states; unsupported MODE/VAC/pointer shapes reject unchanged");return 0;}
