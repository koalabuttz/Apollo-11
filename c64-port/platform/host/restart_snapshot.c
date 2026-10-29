/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "restart_cases.h"
static void print_word(void *user,agc_word word) { (void)user; printf(" %u",word); }
int main(void) {
    uint16_t n; restart_case c;
    for(n=0;n<RS_CASE_COUNT;++n) {
        uint8_t step; rs_prepare(&c,n);
        for(step=0;step<rs_steps(n);++step) {
            int status=rs_apply(&c,n,step);
            printf("%u %u %d",n,step,status); rs_words(&c,print_word,0); putchar('\n');
        }
    }
    return 0;
}
