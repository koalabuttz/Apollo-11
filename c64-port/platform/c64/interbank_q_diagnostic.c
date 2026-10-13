/* SPDX-License-Identifier: MIT */
/* interbank-q on the C64: the first IBQ_C64_CASES cases of
 * tests/interbank_q_cases.c (every entry kind, both BANKJUMP Q ranges)
 * against their frozen yaAGC digests, and the Q-edge refusals.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "interbank_q_check.h"
#define IBQ_C64_CASES 128u
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<IBQ_C64_CASES;++n)failures=(uint16_t)(failures+ibq_check(n,&checks));
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
