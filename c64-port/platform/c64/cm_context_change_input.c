/* SPDX-License-Identifier: MIT */
#include "cm_context_change_binding.h"
#include "cm_context_change_cases.h"
#define CM_CTX_INDEX_ONLY 1
#include "cm_context_change_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_ctx_fixture *f=CM_CTX_FIXTURE;
 if(selector>=CM_CTX_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_ctx_indices[selector];f->status=f->step=f->failures=0;cm_context_change_input(s,CM_CTX_IO,f->source_case);return 0;
}
