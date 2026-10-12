/* SPDX-License-Identifier: MIT */
/* lm-trim-gimbal on the C64: every thirty-second Luminary099
 * TRIM_GIMBAL_CONTROL_SYSTEM case through the trampoline,
 * against digests frozen from the original program
 * (tools/lm_trim_gimbal_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_trim_gimbal_cases.h"
#include "lm_trim_gimbal_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<LM_TG_C64_COUNT;++n){++checks;if(lm_tg_digest(&image,(uint16_t)(n*LM_TG_C64_STRIDE))!=lm_tg_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
