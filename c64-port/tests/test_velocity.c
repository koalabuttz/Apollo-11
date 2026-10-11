/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "velocity_cases.h"
static void rejected(velocity_case *c) {
    uint16_t before[VL_WORD_COUNT],after[VL_WORD_COUNT];
    vl_words(c,before); assert(vl_apply(c)==P66_VELOCITY_OUT_OF_SCOPE); vl_words(c,after);
    assert(memcmp(before,after,sizeof before)==0);
}
int main(void) {
    velocity_case c,original; uint16_t value;
    assert(vl_check(&c)==0); vl_prepare(&original,0);
    for(value=0;value<=AGC_MASK;++value) {
        c=original;
        if(value!=077776) { c.previous.previous.previous.previous.previous.preparation.mpac[7]=value; rejected(&c); }
        c=original;
        if(value!=c.previous.previous.previous.previous.push.base+8u) {
            c.previous.previous.previous.previous.push.pushloc=value; rejected(&c);
        }
        c=original;
        if(value<0401u || value>0661u || (value-0401u)%054u!=0u) {
            c.previous.previous.previous.previous.push.base=value;
            c.previous.previous.previous.previous.push.pushloc=(agc_word)(value+8u); rejected(&c);
        }
    }
    puts("PASS: 156 complete original-assembly velocity-group states; all unsupported MODE/base/pointer ordinary words preserve all 117 state/input words");
    return 0;
}
