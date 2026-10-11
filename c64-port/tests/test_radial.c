/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "radial_cases.h"
static void reject(radial_case *c){uint16_t before[RA_WORDS],after[RA_WORDS];ra_words(c,before);assert(ra_apply(c)==P66_RADIAL_OUT_OF_SCOPE);ra_words(c,after);assert(!memcmp(before,after,sizeof before));}
int main(void){radial_case c,original;uint16_t value;assert(!ra_check(&c));ra_prepare(&original,0);for(value=0;value<=AGC_MASK;++value){c=original;if(value!=077776){c.previous.previous.previous.previous.previous.previous.preparation.mpac[7]=value;reject(&c);}c=original;if(value!=c.previous.previous.previous.previous.previous.push.base){c.previous.previous.previous.previous.previous.push.pushloc=value;reject(&c);}c=original;if(value<0401u||value>0661u||(value-0401u)%054u){c.previous.previous.previous.previous.previous.push.base=value;c.previous.previous.previous.previous.previous.push.pushloc=value;reject(&c);}}puts("PASS: complete original radial states, source errors and all unsupported MODE/VAC/pointer words unchanged");return 0;}
