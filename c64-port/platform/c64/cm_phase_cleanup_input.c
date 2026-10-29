/* SPDX-License-Identifier: MIT */
#include "cm_phase_cleanup_binding.h"
#include "cm_phase_cleanup_cases.h"
#define CM_CLEANUP_INDEX_ONLY 1
#include "cm_phase_cleanup_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_cleanup_fixture *f=CM_CLEANUP_FIXTURE;
 if(selector>=CM_CLEANUP_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_cleanup_indices[selector];f->status=f->step=f->failures=0;cm_phase_cleanup_input(s,CM_CLEANUP_IO,f->source_case);return 0;
}
