/* SPDX-License-Identifier: MIT */
#include "cm_r52_priority_alarm_binding.h"
#include "cm_r52_priority_alarm_cases.h"
#define CM_R52_PRIORITY_ALARM_INDEX_ONLY 1
#include "cm_r52_priority_alarm_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_r52_priority_alarm_fixture *f=CM_R52_PRIORITY_ALARM_FIXTURE;
 if(selector>=CM_R52_PRIORITY_ALARM_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_r52_priority_alarm_indices[selector];f->status=f->step=f->failures=0;cm_r52_priority_alarm_input(s,CM_R52_PRIORITY_ALARM_IO,f->source_case);return 0;
}
