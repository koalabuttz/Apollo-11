/* SPDX-License-Identifier: MIT */
#include "cm_init_display_mixed_binding.h"
#include "cm_init_display_mixed_cases.h"
#include "cm_init_display_mixed_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_init_mixed_fixture *f=CM_INIT_MIXED_FIXTURE;if(selector>=CM_INIT_MIXED_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_init_mixed_indices[selector];f->phase=0;f->failures=0;cm_init_display_mixed_input(s,CM_INIT_MIXED_IO,f->source_case);return 0;}
