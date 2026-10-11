/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "product_cases.h"
int main(void) {
    product_case c; uint16_t mode,before[PR_WORD_COUNT],after[PR_WORD_COUNT]; uint8_t sub;
    assert(pr_check(&c)==0);
    for(mode=0;mode<=AGC_MASK;++mode) for(sub=0;sub<2u;++sub) if(mode!=(sub ? 077776u : 0u)) {
        pr_prepare(&c,3072,sub); c.preparation.mpac[7]=mode; ve_words(&c,before);
        assert(pr_apply(&c,sub)==0); ve_words(&c,after);
        assert(memcmp(before,after,sizeof before)==0);
    }
    puts("PASS: 128 complete original-assembly DMP1/VXSC states; all 32767 unsupported MODE words preserve complete state for both operations");
    return 0;
}
