/* SPDX-License-Identifier: MIT */
#ifndef LM_ADMISSION_IMAGE_H
#define LM_ADMISSION_IMAGE_H
#include "p66_bias.h"
#include "p66_native.h"
enum lm_admission_entry {LM_ADMIT_NOVAC,LM_ADMIT_FINDVAC,LM_ADMIT_SPVAC,LM_ADMIT_SPVACIN};
/* Literal source admission through actual caller return, or BAILOUT1 entry.
 * NOVAC/FINDVAC's inline pair is explicit compiled metadata; SPVAC/SPVACIN
 * use the pair arriving in canonical A/L. Canonical resource markers and
 * admitted/sleeping priorities are required. No source IRQ or lifecycle poll. */
enum exec_status lm_admission_image(apollo_overlay_state *,p66_bias_io *,enum lm_admission_entry,exec_address);
#endif
