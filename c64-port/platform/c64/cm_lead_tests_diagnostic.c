/* SPDX-License-Identifier: MIT */
/* cm-lead-tests on the C64: every case of tests/cm_lead_tests_cases.c against
 * its frozen yaAGC digest, and the refusals.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_lead_tests_check.h"
#include "cm_lead_tests_cases.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<CLT_CASES;++n)failures=(uint16_t)(failures+clt_check(n,&checks));
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
