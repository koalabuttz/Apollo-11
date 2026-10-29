/* SPDX-License-Identifier: MIT */
#ifndef P66_APPLICATION_PENDING_H
#define P66_APPLICATION_PENDING_H
#include "p66_application_recurrence.h"
#include "p66_application_split.h"
#include "p66_elapsed_return_control.h"
#include "lm_danzig_change.h"
enum p66_pending_service {P66_PENDING_EXCHANGE=88,P66_PENDING_ELAPSED_RETURN=89,
 P66_PENDING_STEP_IMPORT=98,P66_PENDING_STEP_EXPORT=99};
typedef struct p66_application_pending {
 p66_application_recurrence recurrence;
 p66_guidance_modules original_math,wrapped_math;
 exec_continuation translated;
 agc_word elapsed_q;
 agc_dp elapsed_before,elapsed_piptime;
 enum exec_context_kind previous_kind;
 enum exec_status step_status;
 uint16_t (*invoke_pending)(void *,struct p66_application_pending *,exec_address *,uint16_t);
 void *pending_modules;
} p66_application_pending;
/* Additive source-selected exchange and return-debris services. The caller
 * retains producer exclusion and every slot frame returns before replacement.
 * The explicit operand metadata describes only the still-owned elapsed call;
 * Executive/VAC state remains in the sole canonical image. */
uint16_t p66_application_pending_entry(p66_application_pending *,exec_address *,uint16_t);
exec_continuation p66_application_pending_resolve(void *,exec_address,enum exec_context_kind);
enum exec_status p66_application_pending_init(exec_native *,executive *,p66_application_pending *);
enum exec_status p66_application_pending_step(p66_application_pending *);
#endif
