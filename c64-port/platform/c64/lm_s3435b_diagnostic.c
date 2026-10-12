/* SPDX-License-Identifier: MIT */
/* lm-s3435b on the C64, built three times, each registering only the
 * routines its cases reach: LM_S3435B_C64_GROUP=1 S34/35.3's LOMAT return
 * and INTINT; =2 GOINT, INTINT and INTINT's INTSTALL return; =3 INTINT's
 * INTEGRVS return and S34/35.3's INTINT return with lm-s3435's NOVRWRT
 * (LM_S3435_C64_GROUP=3). Case k (variant k*2048/LM_S3435B_C64_CASES of the
 * group's routine k%len) runs through the trampoline against digests frozen
 * from the original program (tools/lm_s3435b_differential.py).
 * The case generator is integer-only, so the C64 builds the same entry images
 * as the host. Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4
 * checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_s3435b_cases.h"
#include "lm_s3435b_vectors.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,k;
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * 4 KB image lives in the RAM under KERNAL at $e000. */
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(k=0;k<LM_S3435B_C64_CASES;++k){
  uint32_t n=(uint32_t)k*2048u/LM_S3435B_C64_CASES*LM_S3435B_ROUTINES+lm_s3435b_c64_routine[k%sizeof lm_s3435b_c64_routine];
  ++checks;if(lm_s3435b_digest(image,n)!=lm_s3435b_expected[k])++failures;
 }
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
