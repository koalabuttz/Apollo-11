/* SPDX-License-Identifier: MIT */
#include "cm_wake_complete_binding.h"
#include "cm_wake_complete_cases.h"
#define CM_WAKE_INDEX_ONLY 1
#include "cm_wake_complete_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_wake_fixture *f=CM_WAKE_FIXTURE;
 if(selector>=CM_WAKE_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_wake_indices[selector];f->status=f->step=f->failures=0;cm_wake_complete_input(s,CM_WAKE_IO,f->source_case);return 0;
}
