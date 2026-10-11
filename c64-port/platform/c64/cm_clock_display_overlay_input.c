/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_overlay_binding.h"
#include "cm_clock_display_mission_cases.h"
#include "cm_clock_display_overlay_vectors.h"
#include "cm_clock_display_busy_cases.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_mission_fixture *f=CM_CLOCK_DISPLAY_FIXTURE;
 if(selector>=3u)return 1;f->run=selector;f->source_case=cm_clock_display_overlay_indices[selector];f->step=0;f->failures=0;
 if(f->source_case<64u)cm_clock_display_input(s,CM_CLOCK_DISPLAY_IO,f->source_case);else cm_clock_display_busy_input(s,CM_CLOCK_DISPLAY_IO,(uint16_t)(f->source_case-64u));return 0;
}
