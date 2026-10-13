/* SPDX-License-Identifier: MIT */
/* cm-exec-jobs on the C64: every sixteenth Comanche055 Executive job
 * service case (one routine), against digests frozen from the
 * original program (tools/cm_exec_jobs_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_exec_jobs_cases.h"
#include "cm_exec_jobs_c64_vectors.h"
#include "cm_exec_jobs_c64_rows.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<CM_EXJ_C64_COUNT;++n){++checks;if(cm_exj_digest(&image,(uint16_t)(n*CM_EXJ_C64_STRIDE))!=cm_exj_c64[n])++failures;}
 /* and the cases that take the branch arcs the stride misses (tools/lane_a_c64_cover.py) */
 for(n=0;n<CM_EXJ_C64_ROWS;++n){++checks;if(cm_exj_digest(&image,cm_exj_c64_row[n])!=cm_exj_c64_row_digest[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
