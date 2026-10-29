/* SPDX-License-Identifier: MIT */
#ifndef LM_SPECTEST_HANDOFF_H
#define LM_SPECTEST_HANDOFF_H
#include "lm_admission_image.h"
enum lm_spectest_entry {LM_SPECTEST_NOVAC,LM_SPECTEST_FINDVAC,
 LM_SPECTEST_SPVAC,LM_SPECTEST_SPVACIN,LM_SPECTEST_WAKE,LM_SPECTEST_DIRECT};
/* Literal exceptional callers stop before CCSHOLE5705; EXEC_BAILOUT_PENDING
 * denotes that actual source handoff, not a synthesized recovery or alarm. */
enum exec_status lm_spectest_handoff(apollo_overlay_state *,p66_bias_io *,
                                   enum lm_spectest_entry,exec_address);
#endif
