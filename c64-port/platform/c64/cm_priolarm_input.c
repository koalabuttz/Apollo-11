/* SPDX-License-Identifier: MIT */
#include "cm_priolarm_binding.h"
#include "cm_priolarm_cases.h"
#define CM_PRIOLARM_INDEX_ONLY 1
#include "cm_priolarm_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_priolarm_fixture *f=CM_PRIOLARM_FIXTURE;
 if(selector>=CM_PRIOLARM_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_priolarm_indices[selector];f->status=f->step=f->failures=0;cm_priolarm_input(s,CM_PRIOLARM_IO,f->source_case);return 0;
}
