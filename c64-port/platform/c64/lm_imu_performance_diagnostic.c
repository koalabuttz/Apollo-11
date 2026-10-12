/* SPDX-License-Identifier: MIT */
/* lm-imu-performance on the C64, one part at a time (-DIPT_C64_PART=1..5,
 * tests/lm_imu_performance_cases.h): the sampled cases this part runs,
 * against the digests frozen from the original program
 * (tools/lm_imu_performance_freeze.py, which also lists the sampled cases
 * no part runs). Interrupts off and $01=$35: the image and the undo image
 * live in the RAM under KERNAL at $e000 and $f000.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_imu_performance_cases.h"
#include "lm_imu_performance_c64_vectors.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t n,failures=0,checks=0;
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");
 *(volatile uint8_t *)1u=0x35u;
 for(n=0;n<IPT_C64_COUNT;++n){++checks;if(ipt_digest(image,ipt_c64_case[n])!=ipt_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
