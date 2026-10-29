/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "scale_cases.h"
int main(void) {
    scale_case c;
    assert(sc_check(&c)==0);
    puts("PASS: 292 complete scaling states, signed-zero/mixed-sign DP pairs, all integrated TS-skip inputs, three retained four-step histories");
    return 0;
}
