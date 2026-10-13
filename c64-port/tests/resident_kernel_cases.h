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
/* One case's inputs besides the image: the device record and the
 * module's case record. The C64 flight check (platform/c64/
 * resident_flight_check.c) gets it from a case module in the job slot and
 * runs the kernel half in the resident. */
typedef struct {
 uint16_t n;
#if RK_LM
 p66_bias_io io;
 union { lm_exs_case services; lm_exj_case jobs; agc_hardware idle; } k;
 /* The services programs' second table (RESTARTS' routines), set by the
  * caller: on the C64 the flight check's restart module provides it. */
 const agc_routine_table *restart;
#else
 cm_native_io io;
 cm_exs_case services;
#endif
} rk_case;
/* The digest of a refused run (the kind word alone). */
#define RK_REFUSED_DIGEST 0x00020002ul
/* The entry Z of the last case run (for diagnostics). */
extern agc_word rk_entry;
/* Run case n of each module on image s; the result is the digest. */
uint32_t rk_services_digest(apollo_overlay_state *s,uint16_t n);
uint32_t rk_jobs_digest(apollo_overlay_state *s,uint16_t n);
uint32_t rk_idle_digest(apollo_overlay_state *s,uint16_t n);
/* The two halves of each: the input writes image s and *c for case c->n
 * (the module's generator); the run is the kernel's part and the digest. */
void rk_services_input(apollo_overlay_state *s,rk_case *c);
void rk_jobs_input(apollo_overlay_state *s,rk_case *c);
void rk_idle_input(apollo_overlay_state *s,rk_case *c);
uint32_t rk_services_run(apollo_overlay_state *s,rk_case *c);
uint32_t rk_jobs_run(apollo_overlay_state *s,rk_case *c);
uint32_t rk_idle_run(apollo_overlay_state *s,rk_case *c);
#endif
