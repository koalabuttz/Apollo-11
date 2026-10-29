/* SPDX-License-Identifier: MIT */
#ifndef LM_CORE_FAULT_H
#define LM_CORE_FAULT_H
#include "lm_admission_image.h"
enum lm_core_fault_entry {LM_CORE_FAULT_NOVAC,LM_CORE_FAULT_FINDVAC,
 LM_CORE_FAULT_SPVACIN,LM_CORE_FAULT_SPVAC,LM_CORE_FAULT_WAKE};
/* Literal +0 core-priority paths through TC7/TC014. Stops at reserved-code
 * source PC before executing that external word. No recovery or decoder. */
enum exec_status lm_core_fault(apollo_overlay_state *,p66_bias_io *,
 enum lm_core_fault_entry,exec_address);
#endif
