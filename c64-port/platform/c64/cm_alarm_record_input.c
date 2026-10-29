/* SPDX-License-Identifier: MIT */
#include "cm_alarm_record_binding.h"
#include "cm_alarm_record_cases.h"
#define CM_ALARM_INDEX_ONLY 1
#include "cm_alarm_record_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_alarm_fixture *f=CM_ALARM_FIXTURE;
 if(selector>=CM_ALARM_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_alarm_indices[selector];f->status=f->step=f->failures=0;f->literal=cm_alarm_record_input(s,CM_ALARM_IO,f->source_case);return 0;
}
