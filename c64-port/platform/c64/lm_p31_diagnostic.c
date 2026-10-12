/* SPDX-License-Identifier: MIT */
/* lm-p31 on the C64 (LM_P31_C64_GROUP=1, every entry). Case k (variant
 * k*512/LM_P31_C64_CASES of entry k%17) runs through the trampoline against digests frozen from the
 * original program (tools/lm_p31_differential.py).
 * The case generator is integer-only, so the C64 builds the same entry images
 * as the host. Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4
 * checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_p31_cases.h"
#include "lm_p31_vectors.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,k;
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * 4 KB image lives in the RAM under KERNAL at $e000. */
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(k=0;k<LM_P31_C64_CASES;++k){
  uint32_t n=(uint32_t)k*512u/LM_P31_C64_CASES*LM_P31_ROUTINES+lm_p31_c64_routine[k%sizeof lm_p31_c64_routine];
  ++checks;if(lm_p31_digest(image,n)!=lm_p31_expected[k])++failures;
 }
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
