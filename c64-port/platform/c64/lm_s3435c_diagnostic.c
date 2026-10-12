/* SPDX-License-Identifier: MIT */
/* lm-s3435c on the C64, built three times, each registering only the
 * routines its cases reach (src/lm_s3435c_routines.c, LM_S3435C_C64_GROUP):
 * =1 S34/35.2 with lm-s3435b's INTINT (LM_S3435B_C64_GROUP=2); =2 its LOMAT
 * return; =3 FLAGON, FLAGOFF's S34/35.4 return, VNPOOH's INTPRET return and
 * the basic routines, with lm-p34's and the INTER-BANK basic routines. Case
 * k (variant k*1024/LM_S3435C_C64_CASES of the
 * group's entry k%len) runs through the trampoline against digests frozen
 * from the original program (tools/lm_s3435c_differential.py).
 * The case generator is integer-only, so the C64 builds the same entry images
 * as the host. Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4
 * checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_s3435c_cases.h"
#include "lm_s3435c_vectors.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,k;
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * 4 KB image lives in the RAM under KERNAL at $e000. */
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(k=0;k<LM_S3435C_C64_CASES;++k){
  uint32_t n=(uint32_t)k*1024u/LM_S3435C_C64_CASES*LM_S3435C_ROUTINES+lm_s3435c_c64_routine[k%sizeof lm_s3435c_c64_routine];
  ++checks;if(lm_s3435c_digest(image,n)!=lm_s3435c_expected[k])++failures;
 }
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
