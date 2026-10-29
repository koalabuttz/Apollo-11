/* SPDX-License-Identifier: MIT */
#include "cm_admission_complete_binding.h"
#include "cm_admission_complete_cases.h"
#define CM_ADMIT_INDEX_ONLY 1
#include "cm_admission_complete_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_admit_fixture *f=CM_ADMIT_FIXTURE;
 if(selector>=CM_ADMIT_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_admit_indices[selector];f->status=f->step=f->failures=0;f->job=cm_admission_complete_input(s,CM_ADMIT_IO,f->source_case);return 0;
}
