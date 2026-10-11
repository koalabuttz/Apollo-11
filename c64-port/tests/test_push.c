/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "push_cases.h"
static void rejected(push_case *c) {
    push_case before=*c;
    assert(ps_apply(c)==P66_PUSH_OUT_OF_SCOPE);
    assert(memcmp(c,&before,sizeof *c)==0);
}
int main(void) {
    push_case c;
    assert(ps_check(&c)==0);
    ps_prepare(&c,0); c.previous.preparation.mpac[7]=0; rejected(&c);
    ps_prepare(&c,0); c.push.base=0402; rejected(&c);
    ps_prepare(&c,0); c.push.base=0347; rejected(&c);
    ps_prepare(&c,0); c.push.base=0735; rejected(&c);
    ps_prepare(&c,0); c.push.pushloc=(agc_word)(c.push.base-1u); rejected(&c);
    ps_prepare(&c,0); c.push.pushloc=(agc_word)(c.push.base+38u); rejected(&c);
    ps_prepare(&c,0); c.push.pushloc=077777; rejected(&c);
    ps_prepare(&c,37); assert(ps_apply(&c)==P66_PUSH_OK);
    assert(c.push.pushloc==c.push.base+43u);
    c.previous.preparation.mpac[7]=077776; rejected(&c);
    puts("PASS: 200 complete PDDL states, five VAC mappings, retained exchange debris, composed four-push histories; invalid scope leaves both objects unchanged");
    return 0;
}
