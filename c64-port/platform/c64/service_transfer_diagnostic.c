/* SPDX-License-Identifier: MIT */
/* service-transfer on the C64: every fifth case of
 * tests/service_transfer_cases.c (sites and entries of both missions) against
 * its frozen yaAGC digest, and the refusals.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "service_transfer_check.h"
#include "service_transfer_cases.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<4u;++n)failures=(uint16_t)(failures+svt_check(n,&checks));
 for(n=5;n<svt_cases();n=(uint16_t)(n+5u))failures=(uint16_t)(failures+svt_check(n,&checks));
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
