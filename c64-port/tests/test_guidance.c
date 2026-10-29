/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "guidance_cases.h"
int main(void) {
    guidance_case c;
    uint32_t n;
    agc_register_pair r;
    agc_dp d;
    assert(gn_check(&c) == 0);
    /* Every SP word, multiplied by either memory zero, gives two +0s. */
    for (n = 0; n <= AGC_MASK; ++n) {
        r = agc_multiply(agc_extend((agc_word)n),0); assert(r.a == 0 && r.l == 0);
        r = agc_multiply(agc_extend((agc_word)n),AGC_NEG_ZERO); assert(r.a == 0 && r.l == 0);
    }
    r = agc_multiply(0,077776); assert(r.a == 0177777 && r.l == 0177777);
    r = agc_multiply(0177777,077776); assert(r.a == 0 && r.l == 0);
    r = agc_multiply(1,020000); assert(r.a == 0 && r.l == 020000);
    r = agc_multiply(037777,037777); assert(r.a == 037776 && r.l == 1);
    r.a = 0; r.l = 1; d.high = 0; d.low = 037777;
    agc_double_add(&r,&d); assert(d.high == 1 && d.low == 0 && r.a == 0 && r.l == 0);
    r.a = 1; r.l = 0; d.high = 037777; d.low = 0;
    agc_double_add(&r,&d); assert(d.high == 0 && d.low == 0 && r.a == 1);
    r.a = 0; r.l = 0177776; d.high = 0; d.low = 040000;
    agc_double_add(&r,&d); assert(d.high == 077776 && d.low == 077777 && r.a == 0);
    r.a = 0177776; r.l = 0; d.high = 040000; d.low = 0;
    agc_double_add(&r,&d); assert(d.high == 077777 && d.low == 0 && r.a == 0177776);
    r.a = 1; r.l = 0; d.high = 077776; d.low = 0;
    agc_double_add(&r,&d); assert(d.high == 077777 && d.low == 0); /* independent signs */
    puts("PASS: 568 complete original-assembly checkpoints; all 65536 SP/memory-zero products; carry and overflow contracts");
    return 0;
}
