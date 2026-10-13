/* SPDX-License-Identifier: MIT */
/* A resident-kernel case module (ABI 2 job slot) for the flight check
 * (resident_flight_check.c): one part's case generators, as the
 * resident-kernel C64 programs split them (RK_PART, LM_EXJ_PART). Selector
 * 0: frame->value is the address of the check's rk_case; the module writes
 * case c->n into the image and *c and returns value 1, or 0 when the case
 * is not in this part. Selector 1 (services module only): the dispatcher
 * probe; Z goes to RK_PROBE_Z and value is RK_PROBE_VALUE. */
#include "resident_kernel_cases.h"
#include "resident_flight_check.h"
#if RK_LM&&defined(LM_EXJ_PART)
static int in_part(uint16_t n){enum lm_exj_cohort k=lm_exj_cohort(n);
 return LM_EXJ_PART==1?k<LM_EXJ_JOBSLEEP:LM_EXJ_PART==2?k>=LM_EXJ_JOBSLEEP&&k<LM_EXJ_DELAY:k>=LM_EXJ_DELAY;}
#endif
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){rk_case *c=(rk_case *)(uintptr_t)f->value;
 if(selector==1u){f->state->words[5]=RK_PROBE_Z;f->value=RK_PROBE_VALUE;return APOLLO_OV2_DONE;}
#if RK_PART==1
 rk_services_input(f->state,c);
#elif RK_PART==2
#if RK_LM&&defined(LM_EXJ_PART)
 if(!in_part(c->n)){f->value=0;return APOLLO_OV2_DONE;}
#endif
 rk_jobs_input(f->state,c);
#else
 rk_idle_input(f->state,c);
#endif
 f->value=1;return APOLLO_OV2_DONE;
}
