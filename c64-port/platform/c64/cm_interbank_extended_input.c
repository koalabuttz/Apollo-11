/* SPDX-License-Identifier: MIT */
#include "cm_interbank_extended_binding.h"
#include "cm_interbank_extended_cases.h"
#define CM_IB_EXT_INDEX_ONLY 1
#include "cm_interbank_extended_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_ib_ext_fixture *f=CM_IB_EXT_FIXTURE;
 if(selector>=CM_IB_EXT_TARGET_CASES)return 1;
 f->run=selector;f->source_case=cm_ib_ext_indices[selector];f->status=f->step=f->failures=f->fetched=0;
 f->literal=cm_interbank_extended_input(s,CM_IB_EXT_IO,f->source_case);return 0;
}
