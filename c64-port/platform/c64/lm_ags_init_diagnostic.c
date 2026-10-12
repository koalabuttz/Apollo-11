/* SPDX-License-Identifier: MIT */
/* lm-ags-init on the C64: the Luminary099 AGS_INITIALIZATION cases chosen
 * by tools/lane_f_c64_rows.py (every routine, outcome and path the host
 * cases take) through the routines, against digests frozen from the
 * original program (tools/lm_ags_init_freeze.py). The undo image lives at
 * AGS_UNDO_ADDRESS, outside the program.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_ags_init_cases.h"
#include "lm_ags_init_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<AGS_C64_COUNT;++n){++checks;if(ags_digest(&image,ags_c64_case[n])!=ags_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
