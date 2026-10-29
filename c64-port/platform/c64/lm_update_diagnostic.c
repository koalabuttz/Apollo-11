/* SPDX-License-Identifier: MIT */
/* lm-update on the C64: the outcome-chosen rows of the Luminary099 UPDATE_PROGRAM (every sixteenth case plus a greedy cover of every word the host cases execute)
 * through the trampoline, against digests frozen from the original
 * program (tools/lm_update_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_update_cases.h"
#include "lm_update_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<LM_UP_C64_COUNT;++n){++checks;if(lm_up_digest(&image,lm_up_c64_case[n],&st)!=lm_up_c64[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
