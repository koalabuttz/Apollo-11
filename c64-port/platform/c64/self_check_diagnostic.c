/* SPDX-License-Identifier: MIT */
/* self-check on the C64: the outcome-chosen rows of the Luminary099 and
 * Comanche055 AGC_BLOCK_TWO_SELF-CHECK cases (every 64th case plus a greedy
 * cover of every word the host cases execute, per mission) through the
 * trampoline, against digests frozen from the original programs
 * (tools/self_check_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "self_check_cases.h"
#include "lm_self_check_c64_vectors.h"
#include "cm_self_check_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<LM_SELF_CHECK_C64_COUNT;++n){++checks;if(self_check_digest(0u,&image,lm_self_check_c64_case[n],&st)!=lm_self_check_c64[n]||st!=EXEC_OK)++failures;}
 for(n=0;n<CM_SELF_CHECK_C64_COUNT;++n){++checks;if(self_check_digest(1u,&image,cm_self_check_c64_case[n],&st)!=cm_self_check_c64[n]||st!=EXEC_OK)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
