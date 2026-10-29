/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "elapsed_cases.h"
int main(void) {
    elapsed_case c;
    agc_word old_ovfind,third;
    assert(el_check(&c)==0);
    el_prepare(&c,18); old_ovfind=c.previous.previous.scratch.ovfind;
    third=c.previous.previous.preparation.mpac[2];
    assert(el_apply(&c)==1);
    assert(c.previous.previous.scratch.ovfind==old_ovfind);
    assert(c.elapsed.buf==037777 && c.previous.previous.preparation.rate.registers.a==037777);
    assert(c.previous.previous.preparation.mpac[2]==third);
    el_prepare(&c,19); old_ovfind=c.previous.previous.scratch.ovfind;
    assert(el_apply(&c)==1 && c.previous.previous.scratch.ovfind==old_ovfind);
    assert(c.elapsed.buf==040000 && c.previous.previous.preparation.rate.registers.a==0140000);
    el_prepare(&c,16); assert(el_apply(&c)==7 && c.previous.previous.scratch.ovfind==1);
    el_prepare(&c,17); assert(el_apply(&c)==7 && c.previous.previous.scratch.ovfind==077776);
    puts("PASS: 172 complete elapsed-time states and skip paths, signed zeros, overflow realignment and retained OVFIND, preserved VAC/vector histories");
    return 0;
}
