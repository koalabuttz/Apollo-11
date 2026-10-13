/* SPDX-License-Identifier: MIT */
/* cm-v37 on the C64: the outcome-chosen rows of the Comanche055
 * FRESH_START_AND_RESTART bank 04 / VAC5STOR cases (every 32nd case
 * plus a greedy cover of every word the host cases execute) through the
 * trampoline, against digests frozen from the original program
 * (tools/cm_v37_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_v37_cases.h"
#include "cm_v37_c64_vectors.h"
/* The image lives in RAM under the KERNAL ($e000-$efff, interrupts off and
 * $01=$35) so the program's own data and BSS stay below the $c000 mailbox. */
#define image (*(apollo_overlay_state *)0xe000u)
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(n=0;n<CM_V37_C64_COUNT;++n){++checks;if(cm_v37_digest(&image,cm_v37_c64_case[n],&st)!=cm_v37_c64[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
