/* SPDX-License-Identifier: MIT */
#include "cm_waitlist_internal_binding.h"
#include "cm_waitlist_internal_cases.h"
#define CM_WL_INTERNAL_INDEX_ONLY 1
#include "cm_waitlist_internal_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_wl_internal_fixture *f=CM_WL_INTERNAL_FIXTURE;
 if(selector>=CM_WL_INTERNAL_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_wl_internal_indices[selector];f->status=f->step=f->failures=0;f->literal=cm_waitlist_internal_input(s,CM_WL_INTERNAL_IO,f->source_case);return 0;
}
