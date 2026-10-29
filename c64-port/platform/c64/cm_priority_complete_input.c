/* SPDX-License-Identifier: MIT */
#include "cm_priority_complete_binding.h"
#include "cm_priority_complete_cases.h"
#define CM_PRIO_INDEX_ONLY 1
#include "cm_priority_complete_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_prio_fixture *f=CM_PRIO_FIXTURE;
 if(selector>=CM_PRIO_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_prio_indices[selector];f->status=f->step=f->failures=0;cm_priority_complete_input(s,CM_PRIO_IO,f->source_case);return 0;
}
