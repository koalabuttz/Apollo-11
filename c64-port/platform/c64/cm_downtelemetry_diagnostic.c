/* SPDX-License-Identifier: MIT */
/* cm-downtelemetry on the C64: every second Comanche055 DOWN-TELEMETRY_PROGRAM
 * case (all its downrupts) through the trampoline, against digests frozen from the original
 * program (tools/cm_downtelemetry_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_downtelemetry_cases.h"
#include "cm_downtelemetry_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<CM_DNTM_C64_COUNT;++n){++checks;if(cm_dntm_digest(&image,(uint16_t)(n*CM_DNTM_C64_STRIDE),&st)!=cm_dntm_c64[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
