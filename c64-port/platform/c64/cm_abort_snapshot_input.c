/* SPDX-License-Identifier: MIT */
#include "cm_abort_snapshot_binding.h"
#include "cm_abort_snapshot_cases.h"
#include "cm_abort_snapshot_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_abort_fixture *f=CM_ABORT_FIXTURE;if(selector>=CM_ABORT_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_abort_indices[selector];f->phase=0;f->failures=0;f->literal=cm_abort_snapshot_input(s,CM_ABORT_IO,f->source_case);return 0;}
