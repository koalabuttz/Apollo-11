/* SPDX-License-Identifier: MIT */
/* lm-servicer on the C64: the Luminary099 SERVICER cases chosen
 * by tools/lane_f_c64_rows.py (every routine, outcome and path the host
 * cases take) through the routines, against digests frozen from the
 * original program (tools/lm_servicer_freeze.py). Two PRGs
 * (LMSV_C64_GROUP), each with its own rows: group 2 runs the bank-17 and
 * bank-35 routines (TIGTASK1, P63ZOOM, P40ZOOM, P40AUTO), group 1 those of
 * bank 36, KILLTSK2 and KILLTASK. The undo image lives at
 * LMSV_UNDO_ADDRESS, outside the program.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_servicer_cases.h"
#include "lm_servicer_c64_vectors.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * image lives in the RAM under KERNAL at $e000 (the undo image at $f000). */
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(n=0;n<LMSV_C64_COUNT;++n){++checks;if(lmsv_digest(image,lmsv_c64_case[n])!=lmsv_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
