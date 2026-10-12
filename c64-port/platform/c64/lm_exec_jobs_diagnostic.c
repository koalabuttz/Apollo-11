/* SPDX-License-Identifier: MIT */
/* lm-exec-jobs on the C64: every sixteenth Luminary099 case of SPVAC, CHANG1
 * and CHANG2 (LM_EXJ_PART 1), of JOBSLEEP, JOBWAKE, PRIOCHNG and ENDOFJOB
 * (2) or of the Waitlist task services (3) through the trampoline, against
 * digests frozen from the original program (tools/lm_exec_jobs_freeze.py).
 * The parts are separate programs so each fits below the $c000 mailbox.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_exec_jobs_cases.h"
#include "lm_exec_jobs_c64_vectors.h"
#if LM_EXJ_PART==1
#define LO LM_EXJ_SPVAC
#define HI LM_EXJ_JOBSLEEP
#elif LM_EXJ_PART==2
#define LO LM_EXJ_JOBSLEEP
#define HI LM_EXJ_DELAY
#else
#define LO LM_EXJ_DELAY
#define HI LM_EXJ_COHORTS
#endif
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n,end;
 end=HI==LM_EXJ_COHORTS?LM_EXJ_CASES:lm_exj_first(HI);
 for(n=lm_exj_first(LO);n<end;n=(uint16_t)(n+LM_EXJ_C64_STRIDE)){++checks;if(lm_exj_digest(&image,n)!=lm_exj_c64[n/LM_EXJ_C64_STRIDE])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
