/* SPDX-License-Identifier: MIT */
/* keyrupt-uprupt on the C64: every sixteenth Luminary099 and Comanche055
 * KEYRUPT and UPRUPT case (one routine), against digests frozen from the original
 * programs (tools/keyrupt_uprupt_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "keyrupt_uprupt_cases.h"
#include "keyrupt_uprupt_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<KU_LM_C64_COUNT;++n){++checks;if(ku_digest(0,&image,(uint16_t)(n*KU_C64_STRIDE))!=ku_lm_c64[n])++failures;}
 for(n=0;n<KU_CM_C64_COUNT;++n){++checks;if(ku_digest(1,&image,(uint16_t)(n*KU_C64_STRIDE))!=ku_cm_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
