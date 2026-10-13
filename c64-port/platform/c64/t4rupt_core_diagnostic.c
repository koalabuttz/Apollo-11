/* SPDX-License-Identifier: MIT */
/* t4rupt-core on the C64: every sixteenth Luminary099 and Comanche055
 * T4RUPT core case (one routine), against digests frozen from the original
 * programs (tools/t4rupt_core_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "t4rupt_core_cases.h"
#include "t4rupt_core_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<T4_LM_C64_COUNT;++n){++checks;if(t4_digest(0,&image,(uint16_t)(n*T4_C64_STRIDE))!=t4_lm_c64[n])++failures;}
 for(n=0;n<T4_CM_C64_COUNT;++n){++checks;if(t4_digest(1,&image,(uint16_t)(n*T4_C64_STRIDE))!=t4_cm_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
