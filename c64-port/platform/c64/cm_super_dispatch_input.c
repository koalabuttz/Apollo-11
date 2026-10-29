/* SPDX-License-Identifier: MIT */
#include "cm_super_dispatch_binding.h"
#include "cm_super_dispatch_cases.h"
#define CM_SUPER_INDEX_ONLY 1
#include "cm_super_dispatch_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_super_fixture *f=CM_SUPER_FIXTURE;
 if(selector>=CM_SUPER_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_super_indices[selector];f->status=f->step=f->failures=0;cm_super_dispatch_input(s,CM_SUPER_IO,f->source_case);return 0;
}
