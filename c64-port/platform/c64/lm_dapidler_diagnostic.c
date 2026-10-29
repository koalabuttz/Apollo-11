/* SPDX-License-Identifier: MIT */
/* lm-dapidler on the C64: the Luminary099 DAPIDLER_PROGRAM cases chosen
 * by tools/lane_f_c64_rows.py (every entry, outcome and branch the host
 * cases take) through the trampoline,
 * against digests frozen from the original program
 * (tools/lm_dapidler_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_dapidler_cases.h"
#include "lm_dapidler_c64_vectors.h"
/* The image sits in the RAM under the KERNAL ROM (banked out below), so
 * the program and its data stay under the $c000 mailbox. */
#define image (*(apollo_overlay_state *)0xe000u)
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 __asm__ volatile("sei");*(volatile uint8_t *)1u=0x35u;
 for(n=0;n<LM_DI_C64_COUNT;++n){++checks;if(lm_di_digest(&image,lm_di_c64_case[n])!=lm_di_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
