/* SPDX-License-Identifier: MIT */
/* lm-interbank-transfer on the C64: every genuine Luminary099 BANKCALL,
 * IBNKCALL, POSTJUMP and USPRCADR site in four contexts, every fourth case (the host runs all), through the native
 * routine registry and trampoline (round trips for BANKCALL/IBNKCALL). No rope
 * reader is linked; inline CADRs come from the compiled site table. Digests
 * must equal the host run, which test-lm-interbank-transfer checks against
 * lm_interbank fed the original rope words.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_interbank_routines_cases.h"
#include "lm-interbank-transfer-vectors.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0;uint32_t n;
 for(n=0;n<LM_IBR_TARGET_CASES;++n){++checks;if(lm_ibr_run(n*LM_IBR_TARGET_STRIDE)!=lm_ibr_expected[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
