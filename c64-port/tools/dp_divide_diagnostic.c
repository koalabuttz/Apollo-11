/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned dp_divide_check(void);
int main(void) {
    volatile uint8_t *mail=(volatile uint8_t *)0xc080;
    unsigned failures;
    mail[0]=0; failures=dp_divide_check();
    mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);
    mail[3]=573u&255u;mail[4]=573u>>8;mail[0]=0xa5;
    return failures?1:0;
}
