/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "vector_cases.h"
int main(void) {
    vector_case c; uint16_t mode,before[VE_WORD_COUNT],after[VE_WORD_COUNT]; uint8_t sub;
    assert(ve_check(&c)==0);
    for(mode=0;mode<=AGC_MASK;++mode) if(mode!=077776) for(sub=0;sub<2u;++sub) {
        ve_prepare(&c,4496,sub); c.preparation.mpac[7]=mode; ve_words(&c,before);
        assert(ve_apply(&c,sub)==P66_VECTOR_OUT_OF_SCOPE); ve_words(&c,after);
        assert(memcmp(before,after,sizeof before)==0);
    }
    puts("PASS: 128 complete original-assembly VAD/VSU states; all 32767 unsupported MODE words preserve complete state for both operations");
    return 0;
}
