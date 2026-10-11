/* SPDX-License-Identifier: MIT */
#include "cm_abort_whimper_binding.h"
#include "cm_abort_whimper_cases.h"
#define CM_WHIMPER_INDEX_ONLY 1
#include "cm_abort_whimper_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_whimper_fixture *f=CM_WHIMPER_FIXTURE;
 if(selector>=CM_WHIMPER_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_whimper_indices[selector];f->status=f->step=f->failures=0;cm_abort_whimper_input(s,CM_WHIMPER_IO,&f->in_isr,f->source_case);return 0;
}
