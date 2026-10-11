/* SPDX-License-Identifier: MIT */
#include "cm_abort_word_binding.h"
#include "cm_abort_word_cases.h"
#define CM_ABORT_WORD_INDEX_ONLY 1
#include "cm_abort_word_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_abort_word_fixture *f=CM_ABORT_WORD_FIXTURE;
 if(selector>=CM_ABORT_WORD_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_abort_word_indices[selector];f->status=f->step=f->failures=0;cm_abort_word_input(s,CM_ABORT_WORD_IO,f->source_case);return 0;
}
