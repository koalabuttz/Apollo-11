/* SPDX-License-Identifier: MIT */
#include "cm_scan_zero_binding.h"
#include "cm_scan_zero_cases.h"
#define CM_SCAN_INDEX_ONLY 1
#include "cm_scan_zero_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_scan_fixture *f=CM_SCAN_FIXTURE;
 if(selector>=CM_SCAN_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_scan_indices[selector];f->status=f->step=f->failures=0;cm_scan_zero_input(s,CM_SCAN_IO,f->source_case);return 0;
}
