/* SPDX-License-Identifier: MIT */
#ifndef LM_ADMISSION_RAW_H
#define LM_ADMISSION_RAW_H
#include "lm_admission_image.h"
/* Additive nonnegative priorities 0..037000, including low bits.
 * Erasable inline descriptors are gathered from canonical Q before stores.
 * Literal source admission through actual caller return, or BAILOUT1 entry.
 * NOVAC/FINDVAC's inline pair is explicit compiled metadata; SPVAC/SPVACIN
 * use the pair arriving in canonical A/L. Canonical resource markers and
 * admitted/sleeping priorities are required. No source IRQ or lifecycle poll. */
enum exec_status lm_admission_raw(apollo_overlay_state *,p66_bias_io *,enum lm_admission_entry,exec_address);
#endif
