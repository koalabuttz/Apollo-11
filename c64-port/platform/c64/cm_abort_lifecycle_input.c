/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle_binding.h"
#include "cm_abort_lifecycle_cases.h"
#define CM_LIFECYCLE_INDEX_ONLY 1
#include "cm_abort_lifecycle_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_lifecycle_fixture *f=CM_LIFECYCLE_FIXTURE;if(selector>=CM_LIFECYCLE_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_lifecycle_indices[selector];f->phase=f->status=f->failures=0;f->literal=cm_abort_lifecycle_input(s,CM_LIFECYCLE_IO,f->source_case,&f->in_isr);return 0;}
