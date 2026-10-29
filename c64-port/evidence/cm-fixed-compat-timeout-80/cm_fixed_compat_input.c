/* SPDX-License-Identifier: MIT */
#include "cm_fixed_compat_binding.h"
#include "cm_fixed_compat_cases.h"
#define CM_FIXED_COMPAT_INDEX_ONLY 1
#include "cm_fixed_compat_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_fixed_fixture *f=CM_FIXED_FIXTURE;
 if(selector>=80u)return 1;f->run=selector;f->source_case=cm_fixed_compat_indices[selector];f->phase=0;f->status=0;f->step=0;f->failures=0;cm_fixed_compat_input(s,CM_FIXED_IO,f->source_case);return 0;
}
