/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "horizontal_cases.h"
unsigned hh_check(horizontal_case *);
static void reject(horizontal_case *c,uint8_t phase){uint16_t before[HH_WORDS],after[HH_WORDS];hh_words(c,before);assert(hh_apply(c,phase)==P66_HORIZONTAL_OUT_OF_SCOPE);hh_words(c,after);assert(!memcmp(before,after,sizeof before));}
int main(void){horizontal_case c,original;uint16_t value;uint8_t phase;assert(!hh_check(&c));hh_input(&original,0);for(value=0;value<=AGC_MASK;++value)for(phase=0;phase<8u;++phase){c=original;c.previous.base.previous.previous.previous.preparation.mpac[7]=value;c.previous.base.previous.previous.push.pushloc=(agc_word)(c.previous.base.previous.previous.push.base+((phase==3u||phase==4u)?6u:0u));if(value!=((phase>=1u&&phase<=5u)?077776u:0u))reject(&c,phase);c=original;c.previous.base.previous.previous.previous.preparation.mpac[7]=(phase>=1u&&phase<=5u)?077776:0;c.previous.base.previous.previous.push.base=value;c.previous.base.previous.previous.push.pushloc=(agc_word)(value+((phase==3u||phase==4u)?6u:0u));if(value<0401u||value>0661u||(uint16_t)(value-0401u)%054u)reject(&c,phase);c=original;c.previous.base.previous.previous.previous.preparation.mpac[7]=(phase>=1u&&phase<=5u)?077776:0;c.previous.base.previous.previous.push.pushloc=value;if(value!=c.previous.base.previous.previous.push.base+((phase==3u||phase==4u)?6u:0u))reject(&c,phase);}
puts("PASS256 complete original phased horizontal states; unsupported MODE/VAC/pointer shapes reject unchanged");return 0;}
