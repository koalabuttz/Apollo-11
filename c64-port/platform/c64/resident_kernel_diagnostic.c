/* SPDX-License-Identifier: MIT */
/* resident-kernel on the C64: the thin Executive and Waitlist, built for the
 * canonical image at $7000 (RK_FIXED_IMAGE), on the C64 rows of
 * tests/<m>_kernel_c64_rows.h (every kernel branch arc the host cases take,
 * plus every 64th case with an oracle) against the original program's
 * frozen yaAGC digests. Build with -DRK_LM=1 or -DRK_CM=1 and RK_PART 1
 * (services), 2 (jobs; LM also LM_EXJ_PART 1-3, as lm-exec-jobs splits its
 * cohorts) or 3 (idle): each part is its own program so its case
 * generators fit below the image.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "resident_kernel_cases.h"
#if RK_LM
#include "lm_kernel_c64_rows.h"
#else
#include "cm_kernel_c64_rows.h"
#endif
#if RK_LM&&defined(LM_EXJ_PART)
/* The cohorts tests/lm_exec_jobs_cases.c builds for LM_EXJ_PART. */
static int in_part(uint16_t n){enum lm_exj_cohort k=lm_exj_cohort(n);
 return LM_EXJ_PART==1?k<LM_EXJ_JOBSLEEP:LM_EXJ_PART==2?k>=LM_EXJ_JOBSLEEP&&k<LM_EXJ_DELAY:k>=LM_EXJ_DELAY;}
#endif
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,i,n;uint32_t d;
 apollo_overlay_state *image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
 for(i=0;i<RK_C64_ROWS;++i){n=rk_c64_case[i];
#if RK_PART==1
  if(n>=RK_SERVICES_CASES)continue;
  d=rk_services_digest(image,n);
#elif RK_PART==2
  if(n<RK_SERVICES_CASES||(n=(uint16_t)(n-RK_SERVICES_CASES))>=RK_JOBS_CASES)continue;
#if RK_LM&&defined(LM_EXJ_PART)
  if(!in_part(n))continue;
#endif
  d=rk_jobs_digest(image,n);
#else
  if(n<RK_SERVICES_CASES+RK_JOBS_CASES)continue;
  d=rk_idle_digest(image,(uint16_t)(n-RK_SERVICES_CASES-RK_JOBS_CASES));
#endif
  ++checks;if(d!=rk_c64_digest[i])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
