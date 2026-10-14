/* SPDX-License-Identifier: MIT */
/* cm-t4rupt-chain on the C64: every sixteenth Comanche055 T4RUPT interrupt
 * (through the trampoline) and the cover rows that take the branch arcs the
 * stride misses (tools/lane_a_c64_cover.py), against digests frozen from the
 * original program (tools/cm_t4rupt_chain_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_t4rupt_chain_cases.h"
#include "cm_t4rupt_chain_c64_vectors.h"
#include "cm_t4rupt_chain_c64_rows.h"
/* The 4 KB image lives in the RAM under the KERNAL ROM ($e000), so the
 * three tables and the optics code's interpreter orders have the room below
 * $c000; main banks the KERNAL out with interrupts off before touching it. */
#define image (*(apollo_overlay_state *)0xe000)
_Static_assert(sizeof(apollo_overlay_state) <= 0x1ff0, "image fits under the KERNAL");
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 __asm__ volatile("sei");*(volatile uint8_t *)1=0x35;
 for(n=0;n<CCH_C64_COUNT;++n){++checks;if(cch_digest(&image,(uint16_t)(n*CCH_C64_STRIDE))!=cch_c64[n])++failures;}
 for(n=0;n<CCH_C64_ROWS;++n){++checks;if(cch_digest(&image,cch_c64_row[n])!=cch_c64_row_digest[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
