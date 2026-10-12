/* SPDX-License-Identifier: MIT */
/* lm-restart-select on the C64: every sixteenth Luminary099 RESTARTS selection
 * and RESTARTS-to-SWRETURN composition through the trampoline, against digests
 * frozen from the original program (tools/lm_restart_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_restart_routines_cases.h"
#include "lm_restart_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<LM_RS_SELECT_C64_COUNT;++n){++checks;if(lm_rs_select_digest(&image,(uint16_t)(n*LM_RS_C64_STRIDE))!=lm_rs_select_c64[n])++failures;}
 for(n=0;n<LM_RS_COMPOSE_C64_COUNT;++n){++checks;if(lm_rs_compose_digest(&image,(uint16_t)(n*LM_RS_C64_STRIDE),&st)!=lm_rs_compose_c64[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
