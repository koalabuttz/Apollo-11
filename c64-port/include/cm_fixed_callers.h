/* SPDX-License-Identifier: MIT */
#ifndef CM_FIXED_CALLERS_H
#define CM_FIXED_CALLERS_H
#include "cm_native_image.h"
/* Actual compiled DSPFMEM41,3353 and COMADRS43,3535 lead-ins to SUPDACAL.
 * Does not execute the later octal display or ROPECHK checksum algorithms. */
enum exec_status cm_fixed_caller_enter(apollo_overlay_state *,cm_native_io *,uint8_t selfcheck);
/* Derive physical data address at actual SUPDACAL entry without mutation.
 * Prefetch it through a passive data overlay after the current C frame returns,
 * then call finish with that independently authenticated original data word. */
enum exec_status cm_supdacal_address(const apollo_overlay_state *,const cm_native_io *,uint16_t *);
enum exec_status cm_supdacal_data(apollo_overlay_state *,cm_native_io *,agc_word original_data_word);
#endif
