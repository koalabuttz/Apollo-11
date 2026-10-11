/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_WORD_H
#define CM_ABORT_WORD_H
#include <stdbool.h>
#include "cm_native_image.h"
/* Genuine post-VAC5/SWRETURN BAILOUT7762/POODOO5726/CCSHOLE5754.
 * ALMCADR holds ordinary original caller0100..7775. Resolve fixed source data
 * only after snapshot has changed EB; erasable reads occur directly at step.
 * No word is decoded or used for dispatch. Caller owns producer exclusion. */
bool cm_abort_word_fixed_address(const apollo_overlay_state *,const cm_native_io *,uint16_t *physical_word);
enum exec_status cm_abort_word(apollo_overlay_state *,cm_native_io *,agc_word original_fixed_word);
#endif
