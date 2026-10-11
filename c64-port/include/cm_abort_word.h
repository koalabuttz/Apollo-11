/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_WORD_H
#define CM_ABORT_WORD_H
#include <stdbool.h>
#include "cm_native_image.h"
/* Genuine post-VAC5/SWRETURN BAILOUT7762/POODOO5726/CCSHOLE5754.
 * ALMCADR holds ordinary original caller0100..7775. A fixed inline code is the
 * caller's compiled operand (cm_abort_site_call reads cm_alarm_sites after the
 * snapshot has changed EB); erasable reads occur directly at step.
 * No word is decoded or used for dispatch. Caller owns producer exclusion. */
enum exec_status cm_abort_word(apollo_overlay_state *,cm_native_io *,agc_word original_fixed_word);
#endif
