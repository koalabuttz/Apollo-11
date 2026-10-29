/* SPDX-License-Identifier: MIT */
#ifndef RESIDENT_KERNEL_H
#define RESIDENT_KERNEL_H
#include "agc_transfer.h"
/* The thin resident Executive and Waitlist (src/resident_kernel_body.inc):
 * EXECUTIVE.agc and WAITLIST.agc of each mission translated directly onto
 * the canonical image, with the same entries, operands and continuations
 * as the certified service routines, which are its differential oracle
 * (docs/RESIDENT-KERNEL.md). Each table is a drop-in replacement for the
 * certified table named beside it.
 *
 * Luminary099 (io is p66_bias_io; channel 11 and InIsr in program->hardware):
 *   lm_kernel_routines       lm_exec_routines       NOVAC FINDVAC TWIDDLE
 *                                                   WAITLIST LONGCALL chain
 *   lm_kernel_job_routines   lm_exec_job_routines   SPVAC CHANG1/2 JOBSLEEP
 *                                                   JOBWAKE PRIOCHNG ENDOFJOB
 *   lm_kernel_task_routines  lm_exec_task_routines  T3RUPT DLY2 -1 FIXDELAY
 *                                                   VARDELAY TASKOVER
 *   lm_kernel_idle_routines  lm_exec_idle_routines  DUMMYJOB ADVAN NUCHANG2
 *                                                   SUPDXCHZ SVCT3
 * Comanche055 (io is cm_native_io, with channel 11; InIsr in
 * program->hardware):
 *   cm_kernel_routines       cm_exec_routines       the Waitlist and admission
 *                                                   entries and T3RUPT
 *   cm_kernel_job_routines   cm_exec_job_routines
 *   cm_kernel_idle_routines  cm_exec_idle_routines
 * Inline operands come from the program's site table (lm_exec_site_table,
 * cm_exec_site_table). A routine refuses (EXEC_OUT_OF_SCOPE, nothing
 * written) only for a call site with no compiled operand or a missing
 * program->hardware. */
/* Built with RK_SEAMS, the kernel calls these (the resident defines them,
 * platform/c64/resident_flight.c) so overlay ABI 2 seams follow their job:
 * at CHANJOB4 with NEWJOB (12 times the core set exchanged with set 0),
 * and at ENDOFJOB, whose job in set 0 ends. */
void rk_seams_exchange(agc_word newjob);
void rk_seams_release(void);
extern const agc_routine_table lm_kernel_routines,lm_kernel_job_routines,lm_kernel_task_routines,lm_kernel_idle_routines;
extern const agc_routine_table cm_kernel_routines,cm_kernel_job_routines,cm_kernel_idle_routines;
#endif
