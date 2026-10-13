/* SPDX-License-Identifier: MIT */
/* resident-kernel cases: every case of the certified service milestones
 * (lm-/cm-exec-services, -exec-jobs, -exec-idle) run through the thin
 * resident kernel's tables (include/resident_kernel.h) in the same
 * trampoline programs, digested exactly as each milestone digests the
 * original program's run (its frozen yaAGC vectors). Build with -DRK_LM=1
 * or -DRK_CM=1. */
#ifndef RESIDENT_KERNEL_CASES_H
#define RESIDENT_KERNEL_CASES_H
#include "resident_kernel.h"
#if RK_LM
#include "lm_exec_services_cases.h"
#include "lm_exec_jobs_cases.h"
#include "lm_exec_idle_cases.h"
#define RK_SERVICES_CASES LM_EXS_CASES
#define RK_JOBS_CASES LM_EXJ_CASES
#define RK_IDLE_CASES LM_EXI_CASES
#else
#include "cm_exec_services_cases.h"
#include "cm_exec_jobs_cases.h"
#include "cm_exec_idle_cases.h"
#define RK_SERVICES_CASES CM_EXS_CASES
#define RK_JOBS_CASES CM_EXJ_CASES
#define RK_IDLE_CASES CM_EXI_CASES
#endif
/* The digest of a refused run (the kind word alone). */
#define RK_REFUSED_DIGEST 0x00020002ul
/* The entry Z of the last case run (for diagnostics). */
extern agc_word rk_entry;
/* Run case n of each module on image s; the result is the digest. */
uint32_t rk_services_digest(apollo_overlay_state *s,uint16_t n);
uint32_t rk_jobs_digest(apollo_overlay_state *s,uint16_t n);
uint32_t rk_idle_digest(apollo_overlay_state *s,uint16_t n);
#endif
