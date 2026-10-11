/* SPDX-License-Identifier: MIT */
#include "cm_idle_recheck_binding.h"
#include "cm_idle_recheck_cases.h"
#define CM_IDLE_RECHECK_INDEX_ONLY 1
#include "cm_idle_recheck_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_idle_recheck_fixture *f=CM_IDLE_RECHECK_FIXTURE;
 if(selector>=CM_IDLE_RECHECK_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_idle_recheck_indices[selector];f->status=f->step=f->failures=0;cm_idle_recheck_input(s,CM_IDLE_RECHECK_IO,f->source_case);return 0;
}
