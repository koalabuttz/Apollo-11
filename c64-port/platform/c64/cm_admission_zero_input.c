/* SPDX-License-Identifier: MIT */
#include "cm_admission_zero_binding.h"
#include "cm_admission_zero_cases.h"
#define CM_ADMISSION_ZERO_INDEX_ONLY 1
#include "cm_admission_zero_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_admission_zero_fixture *f=CM_ADMISSION_ZERO_FIXTURE;if(selector>=CM_ADMISSION_ZERO_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_admission_zero_indices[selector];f->status=f->step=f->failures=0;f->job=cm_admission_zero_input(s,CM_ADMISSION_ZERO_IO,f->source_case);return 0;}
