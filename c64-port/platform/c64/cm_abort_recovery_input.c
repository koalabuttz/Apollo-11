/* SPDX-License-Identifier: MIT */
#include "cm_abort_recovery_binding.h"
#include "cm_abort_recovery_cases.h"
#include "cm_abort_recovery_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_recovery_fixture *f=CM_RECOVERY_FIXTURE;if(selector>=CM_RECOVERY_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_recovery_indices[selector];f->phase=0;f->failures=0;cm_abort_recovery_input(s,CM_RECOVERY_IO,f->source_case);return 0;}
