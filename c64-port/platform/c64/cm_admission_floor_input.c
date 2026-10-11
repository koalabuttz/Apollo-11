/* SPDX-License-Identifier: MIT */
#include "cm_admission_floor_binding.h"
#include "cm_admission_floor_cases.h"
#define CM_FLOOR_INDEX_ONLY 1
#include "cm_admission_floor_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_floor_fixture *f=CM_FLOOR_FIXTURE;
 if(selector>=CM_FLOOR_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_floor_indices[selector];f->status=f->step=f->failures=0;(void)cm_admission_floor_input(s,CM_FLOOR_IO,f->source_case);return 0;
}
