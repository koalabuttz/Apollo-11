/* SPDX-License-Identifier: MIT */
/* danzig-pending on the C64: every RTB-left pair's pending DANZIG state from
 * the original programs (tools/danzig_pending_freeze.py) through
 * agc_transfer_point, agc_transfer_resolve and agc_transfer_run.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "danzig_pending_check.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<dzp_row_count();++n)failures=(uint16_t)(failures+dzp_check(n,&checks));
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
