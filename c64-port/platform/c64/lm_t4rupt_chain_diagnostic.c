/* SPDX-License-Identifier: MIT */
/* lm-t4rupt-chain on the C64: every sixteenth Luminary099 T4RUPT interrupt
 * (through the trampoline) and the cases that complete the branch outcomes, against
 * digests frozen from the original program
 * (tools/lm_t4rupt_chain_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_t4rupt_chain_cases.h"
#include "lm_t4rupt_chain_c64_vectors.h"
#include "lm_t4rupt_chain_c64_rows.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<LCH_C64_COUNT;++n){++checks;if(lch_digest(&image,lch_c64_case[n])!=lch_c64[n])++failures;}
 /* and the cases that take the branch arcs those rows miss (tools/lane_a_c64_cover.py) */
 for(n=0;n<LCH_C64_ROWS;++n){++checks;if(lch_digest(&image,lch_c64_row[n])!=lch_c64_row_digest[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
