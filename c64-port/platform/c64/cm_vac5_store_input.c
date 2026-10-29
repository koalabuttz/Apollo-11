/* SPDX-License-Identifier: MIT */
#include "cm_vac5_store_binding.h"
#include "cm_vac5_store_cases.h"
#define CM_VAC5_INDEX_ONLY 1
#include "cm_vac5_store_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_vac5_fixture *f=CM_VAC5_FIXTURE;
 if(selector>=CM_VAC5_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_vac5_indices[selector];f->status=f->step=f->failures=0;cm_vac5_store_input(s,CM_VAC5_IO,f->source_case);return 0;
}
