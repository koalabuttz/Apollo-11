/* SPDX-License-Identifier: MIT */
#include "cm_priority_full_binding.h"
#include "cm_priority_full_cases.h"
#define CM_PRIO_FULL_INDEX_ONLY 1
#include "cm_priority_full_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_prio_full_fixture *f=CM_PRIO_FULL_FIXTURE;
 if(selector>=CM_PRIO_FULL_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_prio_full_indices[selector];f->status=f->step=f->failures=0;cm_priority_full_input(s,CM_PRIO_FULL_IO,f->source_case);return 0;
}
