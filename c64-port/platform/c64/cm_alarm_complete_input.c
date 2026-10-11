/* SPDX-License-Identifier: MIT */
#include "cm_alarm_complete_binding.h"
#include "cm_alarm_complete_cases.h"
#define CM_ALARM_COMPLETE_INDEX_ONLY 1
#include "cm_alarm_complete_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_alarm_complete_fixture *f=CM_ALARM_COMPLETE_FIXTURE;
 if(selector>=CM_ALARM_COMPLETE_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_alarm_complete_indices[selector];f->status=f->step=f->failures=0;cm_alarm_complete_input(s,CM_ALARM_COMPLETE_IO,f->source_case);return 0;
}
