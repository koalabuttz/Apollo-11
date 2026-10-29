/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "executive_service_check.h"
#define MAIL ((volatile uint8_t *)0xc040)
int main(void) {
    unsigned n,failures=0;
    MAIL[0]=0;
    puts("APOLLO SPVAC / CHANG2");
    /* No IRQ producer touches these owned contexts. ROM IRQs remain enabled. */
    for(n=0;n<EXEC_SERVICE_CASE_COUNT;++n) {
        MAIL[6]=(uint8_t)n;
        if(!exec_service_check(n)) ++failures;
    }
    MAIL[1]=(uint8_t)failures; MAIL[2]=(uint8_t)(failures>>8);
    MAIL[3]=(uint8_t)EXEC_SERVICE_CASE_COUNT;
    MAIL[4]=(uint8_t)(EXEC_SERVICE_CASE_COUNT>>8);
    puts(failures ? "SERVICE CHECKS FAILED" : "768 SERVICE STATE CHECKS PASS");
    MAIL[0]=0xa5;
    return failures;
}
