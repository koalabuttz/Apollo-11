/* SPDX-License-Identifier: MIT */
/* lm-phase-table on the C64: every sixteenth Luminary099 PHASE_TABLE_MAINTENANCE
 * case through the trampoline, against digests frozen from the original
 * program (tools/lm_phase_table_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_phase_table_cases.h"
#include "lm_phase_table_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<LM_PT_C64_COUNT;++n){++checks;if(lm_pt_digest(&image,(uint16_t)(n*LM_PT_C64_STRIDE),&st)!=lm_pt_c64[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
