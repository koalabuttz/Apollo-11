/* SPDX-License-Identifier: MIT */
#ifndef CM_EXEC_SERVICES_CASES_H
#define CM_EXEC_SERVICES_CASES_H
#include "cm_exec_routines.h"
/* cm-exec-services cases, each run through the trampoline from its entry to
 * a stop the case names; tools/cm_exec_services_reference.c runs the same
 * entry in the original Comanche055 program.
 *   site      every genuine NOVAC/FINDVAC/WAITLIST/LONGCALL/TWIDDLE call
 *             site (129), 32 states each, to the caller's return
 *   longtask  the LONGCYCL and GETCADR Waitlist tasks, to TASKOVER or the
 *             task 2CADR
 *   fixdelay  every genuine FIXDELAY site (14), 32 states each, to TASKOVER
 *   vardelay  VARDELAY from a task, to TASKOVER
 *   resume    TASKOVER with RUPTAGN -0, to the end of the interrupt
 *   dispatch  TASKOVER with RUPTAGN +1, or the T3RUPT vector, to the next task
 * Executive and Waitlist erasable is canonical, with occupancy chosen so the
 * no-core (1202), no-VAC (1201), full-Waitlist (1203) and bad-delay (1204)
 * alarms are reached too. */
enum cm_exs_cohort {CM_EXS_SITE,CM_EXS_LONGTASK,CM_EXS_FIXDELAY,CM_EXS_VARDELAY,CM_EXS_RESUME,CM_EXS_DISPATCH,CM_EXS_COHORTS};
#define CM_EXS_SITES 129u
#define CM_EXS_SITE_STATES 32u
#define CM_EXS_FIXDELAY_SITES 14u
#define CM_EXS_CASES 7648u
/* Outcome kinds, shared with the oracle. */
#define CM_EXS_RETURNED 0u   /* reached one of the case's stops, or the interrupt ended */
#define CM_EXS_ALARM 1u      /* at BAILOUT or POODOO */
#define CM_EXS_REFUSED 2u    /* native EXEC_OUT_OF_SCOPE; the original executes DPBIT14 or TC 1203 */
#define CM_EXS_MAX_STOPS 10u
typedef struct {
 agc_word stop_z[CM_EXS_MAX_STOPS],stop_fb[CM_EXS_MAX_STOPS]; /* stop_fb 0177777: any FB */
 uint8_t stops;
 uint8_t resume;      /* also ends when the interrupt ends (RESUME) */
 agc_hardware hw;
} cm_exs_case;
#define CM_EXS_ANY_FB 0177777u
enum cm_exs_cohort cm_exs_cohort(uint16_t n);
uint16_t cm_exs_first(enum cm_exs_cohort);
/* Fills the image, io and case for case n. */
void cm_exs_input(apollo_overlay_state *,cm_native_io *,cm_exs_case *,uint16_t n);
/* Runs case n natively; returns the outcome kind (3: anything else). */
unsigned cm_exs_run(apollo_overlay_state *,cm_native_io *,cm_exs_case *,uint16_t n,uint16_t *steps);
uint32_t cm_exs_digest(apollo_overlay_state *,uint16_t n);
#endif
