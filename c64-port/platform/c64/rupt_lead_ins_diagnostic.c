/* SPDX-License-Identifier: MIT */
/* rupt-lead-ins on the C64: every sixteenth Luminary099 and Comanche055
 * lead-in case (one routine), against digests frozen from the original
 * programs (tools/rupt_lead_ins_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "rupt_lead_ins_cases.h"
#include "rupt_lead_ins_c64_vectors.h"
#include "rupt_lead_ins_c64_rows.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<RLI_LM_C64_COUNT;++n){++checks;if(rli_digest(0,&image,(uint16_t)(n*RLI_C64_STRIDE))!=rli_lm_c64[n])++failures;}
 for(n=0;n<RLI_CM_C64_COUNT;++n){++checks;if(rli_digest(1,&image,(uint16_t)(n*RLI_C64_STRIDE))!=rli_cm_c64[n])++failures;}
 /* and the cases that take the branch arcs the stride misses (tools/lane_a_c64_cover.py;
  * a row below RLI_LM_CASES is Luminary's, the rest Comanche's) */
 for(n=0;n<RLI_C64_ROWS;++n){uint16_t r=rli_c64_row[n];++checks;if((r<RLI_LM_CASES?rli_digest(0,&image,r):rli_digest(1,&image,(uint16_t)(r-RLI_LM_CASES)))!=rli_c64_row_digest[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
